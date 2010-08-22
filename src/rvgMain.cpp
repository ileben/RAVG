#include "rvgMain.h"

////////////////////////////////////////////////////////////////////
// Global vars

int resX = 500;
int resY = 600;
int gridResX = 60;
int gridResY = 60;

bool cpu = false;
bool encode = true;
bool draw = true;
bool drawGrid = false;
bool drawCylinder = false;

MatrixStack matModelView;
MatrixStack matProjection;

Shader *shader1;
Shader *shaderQuad;
Shader *shaderStream;
Shader *shaderGrid;
Shader *shaderEncodeInit;
Shader *shaderEncodeObject;
Shader *shaderEncodeObjectAux;
Shader *shaderEncodeLines;
Shader *shaderEncodeLinesAux;
Shader *shaderEncodeQuads;
Shader *shaderEncodeQuadsAux;
Shader *shaderEncodeAux;
Shader *shaderCell;
Shader *shaderCellAux;

Shader *shaderEncodeObjInit;
Shader *shaderEncodeObjInitObject;
Shader *shaderEncodeObjLines;
Shader *shaderEncodeObjQuads;
Shader *shaderEncodeObjObject;
Shader *shaderEncodeObjSort;
Shader *shaderCellObj;

Object *object1;
Object *object2;
Image *image;

int mouseButton = 0;
Vec2 mouseDown;
Float angleX = 0.0f;
Float angleY = 0.0f;
Float zoomZ = 6.0f;


///////////////////////////////////////////////////////////////////
// Functions

void checkGlError (const std::string &text)
{
  static GLenum err = GL_NO_ERROR;
  GLenum newErr = glGetError();
  if (newErr != GL_NO_ERROR && newErr != err) {
    const GLubyte *errString = gluErrorString( newErr );
    std::cout << "GLERROR at '" << text << "': 0x" << std::hex  << int(newErr) << std::dec << " " << errString << std::endl;
  }
  err = newErr;
  glGetError();
}

VertexBuffer::VertexBuffer()
{
  onGpu = false;
  gpuId = 0;
}

void VertexBuffer::toGpu()
{
  if (onGpu) return;
  onGpu = true;

  glGenBuffers( 1, &gpuId );
  glBindBuffer( GL_ARRAY_BUFFER, gpuId );
  glBufferData( GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0], GL_STATIC_DRAW );
  
  checkGlError( "VertexBuffer::toGpu" );
}

void renderVertexBuffer ( Shader *shader, VertexBuffer *buf, GLenum mode )
{
  Int32 pos = shader->program->getAttribute( "in_pos" );
  Int32 tex = shader->program->getAttribute( "in_tex" );

  if (buf->onGpu)
  {
    glBindBuffer( GL_ARRAY_BUFFER, buf->gpuId );
    glVertexAttribPointer( pos, 3, GL_FLOAT, false, sizeof(Vertex), 0 );
    glVertexAttribPointer( tex, 2, GL_FLOAT, false, sizeof(Vertex), (void*) sizeof(Vec3) );
  }
  else
  {
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glVertexAttribPointer( pos, 3, GL_FLOAT, false, sizeof(Vertex), &buf->verts[0] );
    glVertexAttribPointer( tex, 2, GL_FLOAT, false, sizeof(Vertex), ((Uint8*)&buf->verts[0]) + sizeof(Vec3) );
  }

  glEnableVertexAttribArray( pos );
  glEnableVertexAttribArray( tex );
  glDrawArrays( mode, 0, buf->verts.size() );
  glDisableVertexAttribArray( pos );
  glDisableVertexAttribArray( tex );

  checkGlError( "renderVertexBuffer" );
}

Object::Object()
{
  contour = NULL;
  penDown = false;
  pen.set( 0,0 );
  buffersInit = false;

  gridWinding = NULL;
}

void Object::moveTo( Float x, Float y,
  SegSpace::Enum space )
{
  contour = new Contour();
  contours.push_back( contour );

  contour->segments.push_back( SegType::MoveTo | space );
  contour->points.push_back( Vec2(x,y) );
  contour->flatPoints.push_back( Vec2(x,y) );

  pen.set( x,y );
  start.set( x,y );
  penDown = true;
}

void Object::lineTo( Float x, Float y,
  SegSpace::Enum space )
{
  if (!penDown) return;

  contour->segments.push_back( SegType::LineTo | space );
  contour->points.push_back( Vec2(x,y) );
  contour->flatPoints.push_back( Vec2(x,y) );

  lines.push_back( Line( pen.x,pen.y, x, y ));
  pen.set( x,y );
}

void Object::quadTo( Float x1, Float y1, Float x2, Float y2,
  SegSpace::Enum space )
{
  if (!penDown) return;

  contour->segments.push_back( SegType::QuadTo | space );
  contour->points.push_back( Vec2(x1,y1) );
  contour->points.push_back( Vec2(x2,y2) );
  contour->flatPoints.push_back( Vec2(x2,y2) );

  quads.push_back( Quad( pen.x,pen.y, x1,y1, x2,y2 ));
  pen.set( x2,y2 );
}

void Object::cubicTo( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3,
  SegSpace::Enum space )
{
  if (!penDown) return;

  contour->segments.push_back( SegType::CubicTo | space );
  contour->points.push_back( Vec2(x1,y1) );
  contour->points.push_back( Vec2(x2,y2) );
  contour->points.push_back( Vec2(x3,y3) );
  contour->flatPoints.push_back( Vec2(x3,y3) );

  cubics.push_back( Cubic( pen.x,pen.y, x1,y1, x2,y2, x3,y3 ));
  pen.set( x3,y3 );
}

void Object::close()
{
  if (!penDown) return;

  contour->segments.push_back( SegType::Close );

  lines.push_back( Line( pen.x,pen.y, start.x,start.y ));
  pen.set( start.x, start.y );
  penDown = false;
}

void Object::updateBounds()
{
  for (Uint32 c=0; c<contours.size(); ++c)
  {
    for (Uint32 p=0; p<contours[c]->points.size(); ++p)
    {
      Vec2 &point = contours[c]->points[p];
      if (c==0 && p==0)
      {
        min = max = point;
        continue;
      }

      if (point.x < min.x) min.x = point.x;
      if (point.y < min.y) min.y = point.y;

      if (point.x > max.x) max.x = point.x;
      if (point.y > max.y) max.y = point.y;
    }
  }
}

bool intersectLines (const Vec2 &o1, const Vec2 &p1, const Vec2 &o2, const Vec2 &p2, Vec2 &pout)
{
  Vec2 v1 = p1 - o1;
  Vec2 v2 = p2 - o2;

  Float rightU = o2.x - o1.x;
  Float rightD = o2.y - o1.y;
  
  Float D  = v1.x   * (-v2.y) - v1.y   * (-v2.x);
  Float DX = rightU * (-v2.y) - rightD * (-v2.x);
//Float DY = v1.x   * rightD  - v1.y   * rightU;
  
  if (D == 0.0f)
    return false;

  Float t1 = DX / D;
  pout.set(
    o1.x + t1*v1.x,
    o1.y + t1*v1.y);
  return true;
}
/*
Vec2 intersectLines (Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3)
{
  float a0 = (p0.y - p1.y) / (p0.x - p1.x);
  float b0 = p0.y - a0 * p0.x;

  float a1 = (p2.y - p3.y) / (p2.x - p3.x);
  float b1 = (p2.y - a1 * p3.x);

  Vec2 p;
  p.x = (a0 - a1) / (b1 - b0);
  p.y = a0 * p.x + b0;
  return p;
}*/

void subdivCubic (const Cubic &c, Cubic &c1, Cubic &c2)
{
  Vec2 p01 = (c.p0 + c.p1) * 0.5f;
  Vec2 p12 = (c.p1 + c.p2) * 0.5f;
  Vec2 p23 = (c.p2 + c.p3) * 0.5f;
  Vec2 pa = (p01 + p12) * 0.5f;
  Vec2 pb = (p12 + p23) * 0.5f;
  Vec2 pc = (pa + pb) * 0.5f;

  c1.p0 = c.p0;
  c1.p1 = p01;
  c1.p2 = pa;
  c1.p3 = pc;

  c2.p0 = pc;
  c2.p1 = pb;
  c2.p2 = p23;
  c2.p3 = c.p3;
}

void Object::cubicsToQuads()
{/*
  //Generic recursive subdivision
  for (Uint32 i=0; i<cubics.size(); ++i)
  {
    Cubic c = cubics[i];
    
    Cubic c1,c2;
    subdivCubic( c, c1,c2 );
    
    Cubic cubics[4];
    subdivCubic( c1, cubics[0], cubics[1] );
    subdivCubic( c2, cubics[2], cubics[3] );

    for (int j=0; j<4; ++j)
    {
      Cubic cj = cubics[j];
      
      Quad quad;
      quad.p0 = cj.p0;
      quad.p2 = cj.p3;

      if (! intersectLines( cj.p0, cj.p1, cj.p2, cj.p3, quad.p1 ))
        quad.p1 = (cj.p1 + cj.p2) * 0.5f;

      quads.push_back( quad );
    }
  }*/

  //Fixed subdivision into 4 quads
  for (Uint32 i=0; i<cubics.size(); ++i)
  {
    Cubic cu = cubics[i];

    Vec2 p01 = (cu.p0 + cu.p1) * 0.5f;
    Vec2 p12 = (cu.p1 + cu.p2) * 0.5f;
    Vec2 p23 = (cu.p2 + cu.p3) * 0.5f;

    Vec2 p001 = (cu.p0 + p01) * 0.5f;
    Vec2 a = (p001 + p01) * 0.5f;

    Vec2 p233 = (p23 + cu.p3) * 0.5f;
    Vec2 d = (p23 + p233) * 0.5f;

    Vec2 p0112 = (p01 + p12) * 0.5f;
    Vec2 p1223 = (p12 + p23) * 0.5f;
    Vec2 B = (p0112 + p1223) * 0.5f;

    Vec2 p0112B = (p0112 + B) * 0.5f;
    Vec2 b = (p0112 + p0112B) * 0.5f;

    Vec2 pB1223 = (B + p1223) * 0.5f;
    Vec2 c = (pB1223 + p1223) * 0.5f;

    Vec2 A = (a + b) * 0.5f;
    Vec2 C = (c + d) * 0.5f;

    quads.push_back( Quad( cu.p0, a, A ) );
    quads.push_back( Quad( A, b, B ) );
    quads.push_back( Quad( B, c, C ) );
    quads.push_back( Quad( C, d, cu.p3 ) );
  }

  cubics.clear();
}

void Object::updateGrid()
{
  if (gridWinding) delete[] gridWinding;
  gridWinding = new int[ image->gridSize.x * image->gridSize.y ];

  Vec2 testPivot( image->min.x - 1.0f, image->min.y - 1.0f );

  for (int x=0; x < image->gridSize.x; ++x) {
    for (int y=0; y < image->gridSize.y; ++y) {
      
      Vec2 pivot(
        image->min.x + (x + 0.5f) * image->cellSize.x,
        image->min.y + (y + 0.5f) * image->cellSize.y);

      int gridIndex = y * image->gridSize.x + x;
      int w = 0;

      //Walk the list of line segments
      for (Uint32 l=0; l<lines.size(); ++l) {

        //Update pivot winding
        w += lineWinding( lines[l].p0, lines[l].p1, testPivot, pivot );
      }

      //Walk the list of quad segments
      for (Uint32 q=0; q<quads.size(); ++q) {

        //Update pivot winding
        w += quadWinding( quads[q].p0, quads[q].p1, quads[q].p2, testPivot, pivot );
      }

      //Store pivot winding
      gridWinding[ gridIndex ] = w;
    }
  }
}

void Object::updateBuffers()
{
  checkGlError( "updateBuffersGrid start" );

  if (!buffersInit)
  {
    glGenBuffers( 1, &bufLines );
    glGenBuffers( 1, &bufQuads );
    glGenBuffers( 1, &bufABC );
    glGenBuffers( 1, &bufPivotWind );
    glGenBuffers( 1, &bufLinesQuads );
    glGenTextures( 1, &texGrid );
    buffersInit = true;
  }

  checkGlError( "Object::updateBuffers init" );

  //////////////////////////////////////
  //Line control points
  
  if (lines.size() > 0)
  {
    glBindBuffer( GL_ARRAY_BUFFER, bufLines );
    glBufferData( GL_ARRAY_BUFFER, lines.size() * sizeof( Line ), &lines[0], GL_STATIC_DRAW );

    checkGlError( "Object::updateBuffersGrid lines" );
  }

  //////////////////////////////////////
  //Quadratic control points

  if (quads.size() > 0)
  {
    glBindBuffer( GL_ARRAY_BUFFER, bufQuads );
    glBufferData( GL_ARRAY_BUFFER, quads.size() * sizeof( Quad ), &quads[0], GL_STATIC_DRAW );

    checkGlError( "Object::updateBuffers quads" );
  }

  //////////////////////////////////////
  //Line + quad control points

  int countSize = 2 * sizeof( Float );
  int lineSize = lines.size() * sizeof( Line );
  int quadSize = quads.size() * sizeof( Quad );
  Float counts[2] = { (Float) lines.size(), (Float) quads.size() };

  glBindBuffer( GL_ARRAY_BUFFER, bufLinesQuads );
  glBufferData( GL_ARRAY_BUFFER, countSize + lineSize + quadSize, 0, GL_STATIC_DRAW );
  
                    glBufferSubData( GL_ARRAY_BUFFER, 0,                    countSize, counts );
  if (lineSize > 0) glBufferSubData( GL_ARRAY_BUFFER, countSize,            lineSize,  &lines[0] );
  if (quadSize > 0) glBufferSubData( GL_ARRAY_BUFFER, countSize + lineSize, quadSize,  &quads[0] );

  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_ONLY );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrLinesQuads );

  checkGlError( "Object::updateBuffers lines+quads" );

  //////////////////////////////////////
  //Pivot winding

  if (gridWinding)
  {
    std::vector< Float > grid;

    for (int y=0; y < image->gridSize.y; ++y) {
      for (int x=0; x < image->gridSize.x; ++x) {

        int gridIndex = y * image->gridSize.x + x;

        grid.push_back( (Float) gridWinding[ gridIndex ] );
        grid.push_back( 0.0f ); //padding
        grid.push_back( 0.0f ); //padding
        grid.push_back( 0.0f ); //padding
      }
    }

    glBindTexture( GL_TEXTURE_1D, texGrid );
    glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA32F, grid.size() / 4, 0, GL_RGBA, GL_FLOAT, &grid[0] );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    checkGlError( "Object::updateBuffers grid" );
  }

  //////////////////////////////////////
  //Setup pivot buffers

  if (gridWinding)
  {
    glBindBuffer( GL_ARRAY_BUFFER, bufPivotWind );
    glBufferData( GL_ARRAY_BUFFER, image->gridSize.x * image->gridSize.y * sizeof(int), gridWinding, GL_STATIC_DRAW );
  }

  checkGlError( "Object::updateBuffers pivot" );
}

Image::Image()
{
  gridPivots = NULL;
  gridWinding = NULL;
  buffersInit = false;

  cpuCounters = NULL;
  cpuStream = NULL;
  cpuStreamLen = 0;

  ptrInfo = NULL;
  ptrObjects = NULL;
  ptrObjectGrids = NULL;
  ptrGrid = NULL;
  ptrStream = NULL;
}

void Image::updateBounds()
{
  for (Uint32 o=0; o<objects.size(); ++o)
  {
    Object* obj = objects[o];
    obj->updateBounds();

    if (o == 0)
    {
      min = obj->min;
      max = obj->max;
      continue;
    }

    if (obj->min.x < min.x) min.x = obj->min.x;
    if (obj->min.y < min.y) min.y = obj->min.y;

    if (obj->max.x > max.x) max.x = obj->max.x;
    if (obj->max.y > max.y) max.y = obj->max.y;
  }
  
  gridSize.x = gridResX;
  gridSize.y = gridResY;
  gridOrigin.x = min.x;
  gridOrigin.y = min.y;
  cellSize.x = (max.x - min.x) / gridSize.x;
  cellSize.y = (max.y - min.y) / gridSize.y;
}

void Image::updateGrid ()
{
  for (Uint32 o=0; o<objects.size(); ++o)
  {
    Object* obj = objects[o];
    obj->updateGrid();
  }

  if (gridPivots) delete[] gridPivots;
  if (gridWinding) delete[] gridWinding;

  gridPivots = new Vec2 [gridSize.x * gridSize.y];
  gridWinding = new int [gridSize.x * gridSize.y];

  for (int x=0; x<gridSize.x; ++x) {
    for (int y=0; y<gridSize.y; ++y) {
      
      Vec2 pivot(
        min.x + (x + 0.5f) * cellSize.x,
        min.y + (y + 0.5f) * cellSize.y );

      int gridIndex = y * gridSize.x + x;

      gridPivots[ gridIndex ] = pivot;
      gridWinding[ gridIndex ] = 0;
      
      for (Uint32 o=0; o < objects.size(); ++o)
        gridWinding[ gridIndex ] += objects[o]->gridWinding[ gridIndex ];
    }
  }
}

void Image::updateBuffers ()
{
  for (Uint32 o=0; o<objects.size(); ++o)
  {
    Object* obj = objects[o];
    obj->updateBuffers();
  }

  if (!buffersInit)
  {
    glGenBuffers( 1, &bufPivotPos );
    glGenBuffers( 1, &bufPivotWind );
    glGenTextures( 1, &texGrid );
    
    glGenTextures( 1, &texCellCounters );
    glGenTextures( 1, &texCellStreams );
    glGenBuffers( 1, &bufCellStreams );

    glGenTextures( 1, &texCpuCounters );
    glGenBuffers( 1, &bufCpuStream );

    glGenBuffers( 1, &bufObjGrid );
    glGenBuffers( 1, &bufObjStream );

    glGenBuffers( 1, &bufObjs );
    glGenBuffers( 1, &bufGpuObjInfo );
    glGenBuffers( 1, &bufGpuObjObjects );
    glGenBuffers( 1, &bufGpuObjGrid );
    glGenBuffers( 1, &bufGpuObjStream );

    buffersInit = true;
  }

  //////////////////////////////////////
  //Setup pivot buffers
  
  glBindBuffer( GL_ARRAY_BUFFER, bufPivotPos );
  glBufferData( GL_ARRAY_BUFFER, gridSize.x * gridSize.y * sizeof(Vec2), gridPivots, GL_STATIC_DRAW );

  glBindBuffer( GL_ARRAY_BUFFER, bufPivotWind );
  glBufferData( GL_ARRAY_BUFFER, gridSize.x * gridSize.y * sizeof(int), gridWinding, GL_STATIC_DRAW );

  checkGlError( "Image::updateBuffers pivot" );

  //////////////////////////////////////
  //Pivot winding texture
  
  if (gridWinding)
  {
    std::vector< Float > grid;

    for (int y=0; y < image->gridSize.y; ++y) {
      for (int x=0; x < image->gridSize.x; ++x) {

        int gridIndex = y * image->gridSize.x + x;

        grid.push_back( (Float) gridWinding[ gridIndex ] );
        grid.push_back( 0.0f ); //padding;
        grid.push_back( 0.0f ); //padding;
        grid.push_back( 0.0f ); //padding;
      }
    }

    glBindTexture( GL_TEXTURE_1D, texGrid );
    glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA32F, grid.size() / 4, 0, GL_RGBA, GL_FLOAT, &grid[0] );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    checkGlError( "Image::updateBuffers grid" );
  }

  //////////////////////////////////////
  //Cell stream texture

  glBindBuffer( GL_ARRAY_BUFFER, bufCellStreams );
  glBufferData( GL_ARRAY_BUFFER, MAX_COMBINED_STREAM_LEN * sizeof(GLfloat), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrCellStreams );

  checkGlError( "Image::updateBuffers stream" );

  //GL_R32UI with GL_UNSIGNED_INT doesn't work atm (error 0x502 invalid operation)
  glBindTexture( GL_TEXTURE_2D_ARRAY, texCellCounters );
  glTexImage3D( GL_TEXTURE_2D_ARRAY, 0, GL_R32F, gridSize.x, gridSize.y, COUNTER_LEN, 0, GL_RED, GL_FLOAT, 0 );

  checkGlError( "Image::updateBuffers counter" );

  //////////////////////////////////////
  //Cpu stream texture

  glBindBuffer( GL_ARRAY_BUFFER, bufCpuStream );
  glBufferData( GL_ARRAY_BUFFER, MAX_COMBINED_STREAM_LEN * sizeof(GLfloat), cpuStream, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_ONLY );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrCpuStream );

  checkGlError( "Image::updateBuffers cpuStream" );

  glBindTexture( GL_TEXTURE_2D_ARRAY, texCpuCounters );
  glTexImage3D( GL_TEXTURE_2D_ARRAY, 0, GL_R32F, gridSize.x, gridSize.y, COUNTER_LEN, 0, GL_RED, GL_FLOAT, cpuCounters );

  checkGlError( "Image::updateBuffers cpuCounters" );

  //////////////////////////////////////
  //Cpu object buffers

  glBindBuffer( GL_ARRAY_BUFFER, bufObjGrid );
  glBufferData( GL_ARRAY_BUFFER, gridSize.x * gridSize.y * NUM_CELL_COUNTERS * sizeof(int), ptrGrid, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrObjGrid );

  checkGlError( "Image::updateBuffers objGrid" );

  glBindBuffer( GL_ARRAY_BUFFER, bufObjStream );
  glBufferData( GL_ARRAY_BUFFER, 500000 * sizeof(float), ptrStream, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrObjStream );

  checkGlError( "Image::updateBuffers objStream" );

  //////////////////////////////////////
  //Gpu object buffers

  glBindBuffer( GL_ARRAY_BUFFER, bufObjs );
  glBufferData( GL_ARRAY_BUFFER, 1000 * sizeof(Obj), 0, GL_STATIC_DRAW );
  
  checkGlError( "Image::updateBuffers objs" );

  glBindBuffer( GL_ARRAY_BUFFER, bufGpuObjInfo );
  glBufferData( GL_ARRAY_BUFFER, NUM_INFO_COUNTERS * sizeof(int), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuObjInfo );

  checkGlError( "Image::updateBuffers gpuobjInfo" );

  glBindBuffer( GL_ARRAY_BUFFER, bufGpuObjObjects );
  glBufferData( GL_ARRAY_BUFFER, 1000 * sizeof(ObjInfo), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuObjObjects );
  
  checkGlError( "Image::updateBuffers gpuobjObjects" );

  glBindBuffer( GL_ARRAY_BUFFER, bufGpuObjGrid );
  glBufferData( GL_ARRAY_BUFFER, 500000 * sizeof(int), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuObjGrid );

  checkGlError( "Image::updateBuffers gpuobjGrid" );

  glBindBuffer( GL_ARRAY_BUFFER, bufGpuObjStream );
  glBufferData( GL_ARRAY_BUFFER, 500000 * sizeof(float), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuObjStream );

  checkGlError( "Image::updateBuffers gpuobjStream" );
}

std::vector< Shader::Def > Shader::defs;

void Shader::Define (const std::string &key, const std::string &value)
{
  Def def;
  def.key = key;
  def.value = value;
  defs.push_back( def );
}

std::string Shader::ApplyDefs (const std::string &source)
{
  std::string str = source;
  
  for (Uint32 d=0; d<defs.size(); ++d)
  {
    std::size_t x = 0;
    while ((x = str.find( defs[d].key )) != std::string::npos)
      str = str.replace( x, defs[d].key.length(), defs[d].value );
  }

  return str;
}

Shader::Shader (const std::string &vertFile, const std::string &fragFile)
{
  this->vertFile = vertFile;
  this->fragFile = fragFile;

  vertex = NULL;
  geometry = NULL;
  fragment = NULL;
  program = NULL;
}

Shader::Shader (const std::string &vertFile, const std::string &geomFile, const std::string &fragFile)
{
  this->vertFile = vertFile;
  this->geomFile = geomFile;
  this->fragFile = fragFile;

  vertex = NULL;
  geometry = NULL;
  fragment = NULL;
  program = NULL;
}

Shader::~Shader ()
{
  if (vertex) delete vertex;
  if (geometry) delete geometry;
  if (fragment) delete fragment;
  if (program) delete program;
}

std::string readFile (const std::string &filename)
{
  std::ifstream f;
  f.open( filename.c_str(), std::ios::binary );
  if (!f.is_open()) {
    std::cout << "Failed opening file!" << std::endl;
    return std::string( "" );
  }

  long begin = f.tellg();
  f.seekg( 0, std::ios::end );
  long end = f.tellg();
  f.seekg( 0, std::ios::beg );
  long size = end - begin;

  if (size == 0) {
    std::cout << "Zero file size!"  << std::endl;
    return std::string( "" );
  }

  char *cstr = new char[ size+1 ];
  f.read( cstr, size );
  f.close();
  cstr[ size ] = '\0';

  std::string str( cstr );
  delete[] cstr;

  return str;
}

bool Shader::load()
{
  if (vertex) delete vertex;
  if (geometry) delete geometry;
  if (fragment) delete fragment;
  if (program) delete program;

  program = new GLProgram();
  program->create();

  //Vertex
  if (vertFile != "")
  {
    vertex = new GLShader();
    std::cout << "Loading shader " << vertFile << "..." << std::endl;
    std::string vertString = readFile( vertFile );
    vertString = Shader::ApplyDefs( vertString );

    vertex->create( ShaderType::Vertex );
    if (vertex->compile( vertString ))
    {
      std::cout << "Vertex compiled successfuly" << std::endl;
      program->attach( vertex );
    }
    else std::cout << "Vertex compilation FAILED" << std::endl;

    std::string vertLog = vertex->getInfoLog();
    if (vertLog.length() > 0)
      std::cout << vertLog << std::endl;
  }

  //Geometry
  if (geomFile != "")
  {
    geometry = new GLShader();
    std::cout << "Loading shader " << geomFile << "..." << std::endl;
    std::string geomString = readFile( geomFile );
    geomString = Shader::ApplyDefs( geomString );

    geometry->create( ShaderType::Geometry );
    if (geometry->compile( geomString ))
    {
      std::cout << "Geometry compiled successfuly" << std::endl;
      program->attach( geometry );
    }
    else std::cout << "Geometry compilation FAILED" << std::endl;

    std::string geomLog = geometry->getInfoLog();
    if (geomLog.length() > 0)
      std::cout << geomLog << std::endl;
  }

  //Fragment
  if (fragFile != "")
  {
    fragment = new GLShader();
    std::cout << "Loading shader " << fragFile << "..." << std::endl;
    std::string fragString = readFile( fragFile );
    fragString = Shader::ApplyDefs( fragString );

    fragment->create( ShaderType::Fragment );
    if (fragment->compile( fragString ))
    {
      std::cout << "Fragment compiled successfuly" << std::endl;
      program->attach( fragment );
    }
    else std::cout << "Fragment compilation FAILED" << std::endl;
    
    std::string fragLog = fragment->getInfoLog();
    if (fragLog.length() > 0)
      std::cout << fragLog << std::endl;
  }

  //Link
  if (program->link())
    std::cout << "Program linked successfully" << std::endl;
  else std::cout << "Program link FAILED" << std::endl;

  std::string linkLog = program->getInfoLog();
  if (linkLog.length() > 0)
    std::cout << linkLog << std::endl;

  return true;
}

void Shader::use()
{
  if (program) program->use();
}

void renderFullScreenQuad (Shader *shader)
{
  Float coords[] = {
    -1.0f, -1.0f,
    +1.0f, -1.0f,
    +1.0f, +1.0f,
    -1.0f, +1.0f
  };

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, 2 * sizeof( Float ), coords );

  glEnableVertexAttribArray( pos );
  glDrawArrays( GL_QUADS, 0, 4 );
  glDisableVertexAttribArray( pos );
}

void renderQuad (Shader *shader, Vec2 min, Vec2 max)
{
  Float coords[] = {
    min.x, min.y,
    max.x, min.y,
    max.x, max.y,
    min.x, max.y
  };

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, 2 * sizeof( Float ), coords );

  glEnableVertexAttribArray( pos );
  glDrawArrays( GL_QUADS, 0, 4 );
  glDisableVertexAttribArray( pos );
}

void renderQuads (Object *o, Shader *shader)
{
  Int32 modelview = shader->program->getUniform( "modelview" );
  glUniformMatrix4fv( modelview, 1, false, (GLfloat*) matModelView.top().m );

  Int32 projection = shader->program->getUniform( "projection" );
  glUniformMatrix4fv( projection, 1, false, (GLfloat*) matProjection.top().m );

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glEnableVertexAttribArray( pos );
  glBindBuffer( GL_ARRAY_BUFFER, o->bufQuads );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

  Int32 abcU = shader->program->getAttribute( "in_abcU" );
  Int32 abcV = shader->program->getAttribute( "in_abcV" );
  glEnableVertexAttribArray( abcU );
  glEnableVertexAttribArray( abcV );
  glBindBuffer( GL_ARRAY_BUFFER, o->bufABC );
  glVertexAttribPointer( abcU, 3, GL_FLOAT, false, 2 * sizeof( Vec3 ), 0 );
  glVertexAttribPointer( abcV, 3, GL_FLOAT, false, 2 * sizeof( Vec3 ), (GLvoid*) sizeof( Vec3 ) );
  
  glDrawArrays( GL_TRIANGLES, 0, o->quads.size() * 3 );
  glDisableVertexAttribArray( pos );
}

void renderObjectAlter (Object *o)
{
  //-----------------------------------------------

  /*
  //Vertex data needs to come from a buffer for gl_VertexID to be defined in shaders
  Shader *shader = shaderQuad;
  shader->use();

  //glEnable( GL_MULTISAMPLE );
  //glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
  
  renderQuads( o, shader );

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glEnableVertexAttribArray( pos );
  glBindBuffer( GL_ARRAY_BUFFER, o->bufQuads );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );
  glDrawArrays( GL_TRIANGLES, 0, o->quads.size() * 3 );
  glDisableVertexAttribArray( pos );
  */

  /*
  //-----------------------------------------------

  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );

  glEnable( GL_STENCIL_TEST );
  glStencilFunc( GL_ALWAYS, 0, 1 );
  glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
  glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

  renderQuads( o, shader );

  glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );

  //-----------------------------------------------

  shader = shader1;
  shader->use();

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glEnableVertexAttribArray( pos );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  for (Uint32 c=0; c<o->contours.size(); ++c)
  {
    Contour *contour = o->contours[c];
    glVertexAttribPointer( pos, 2, GL_FLOAT, false, sizeof( Vec2 ), &contour->flatPoints[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, contour->flatPoints.size() * 2 );
  }

  glDisableVertexAttribArray( pos );

  //-----------------------------------------------

  glStencilFunc( GL_EQUAL, 1, 1 );
  glStencilOp( GL_ZERO, GL_ZERO, GL_ZERO );
  glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

  glBegin( GL_QUADS );
  glVertex2f( o->min.x, o->min.y );
  glVertex2f( o->max.x, o->min.y );
  glVertex2f( o->max.x, o->max.y );
  glVertex2f( o->min.x, o->max.y );
  glEnd(); */
}

void encodeImage (Image *image)
{
  //Even though the texture is R32F it can be "viewed" as R32I by the image load / store
  //operations (according to the spec) as it shares the same equivalence format of 1x32
  glBindImageTexture( 1, image->texCellCounters, 0, true, 0, GL_READ_WRITE, GL_R32I );
  
  checkGlError( "encodeImage bind" );
 
  glViewport( 0, 0, image->gridSize.x, image->gridSize.y );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_MULTISAMPLE );

  /////////////////////////////////////////////////////
  // Init stream values and counters
  {
    Shader *shader = shaderEncodeInit;
    shader->use();

    Int32 uPtrCellStreams = shader->program->getUniform( "ptrCellStreams" );
    glUniformui64( uPtrCellStreams, image->ptrCellStreams );

    Int32 uImgCellCounters = shader->program->getUniform( "imgCellCounters" );
    glUniform1i( uImgCellCounters, 1 );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    renderFullScreenQuad( shader );
  }

  glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT | GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );

  checkGlError( "encodeImage init" );

/*
  Float *testBuf = new Float[ o->gridSize.x * o->gridSize.y * COUNTER_LEN ];
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, testBuf );
  Int32 *uTestBuf = (Int32*)testBuf;
  for (int y=0; y<o->gridSize.y; ++y) {
    for (int x=0; x<o->gridSize.x; ++x) {
      std::cout << uTestBuf[ y * o->gridSize.x + x ] << " ";
    }
    std::cout << std::endl;
  }
  delete[] testBuf;
*/
  for (int o=(int)image->objects.size()-1; o>=0; --o)
  {
    Object *obj = image->objects[o];
    
    /////////////////////////////////////////////////////
    // Encode object lines into stream
    {
#if (GPU_USE_AUX)
      Shader *shader = shaderEncodeLinesAux;
      shader->use();
#else
      Shader *shader = shaderEncodeLines;
      shader->use();
#endif

      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

      Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
      glUniform2f( uGridOrigin, image->min.x, image->min.y );

      Int32 uCellSize = shader->program->getUniform( "cellSize" );
      glUniform2f( uCellSize, image->cellSize.x, image->cellSize.y );

      Int32 uGridSize = shader->program->getUniform( "gridSize" );
      glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glEnableVertexAttribArray( aPos );
      glBindBuffer( GL_ARRAY_BUFFER, obj->bufLines );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

      Int32 uPtrCellStreams = shader->program->getUniform( "ptrCellStreams" );
      glUniformui64( uPtrCellStreams, image->ptrCellStreams );

      Int32 uImgCellCounters = shader->program->getUniform( "imgCellCounters" );
      glUniform1i( uImgCellCounters, 1 );
      
      glDrawArrays( GL_LINES, 0, obj->lines.size() * 2 );
      glDisableVertexAttribArray( aPos );
    }

    /////////////////////////////////////////////////////
    // Encode object quads into stream
    {
#if (GPU_USE_AUX)
      Shader *shader = shaderEncodeQuadsAux;
      shader->use();
#else
      Shader *shader = shaderEncodeQuads;
      shader->use();
#endif

      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

      Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
      glUniform2f( uGridOrigin, image->min.x, image->min.y );

      Int32 uCellSize = shader->program->getUniform( "cellSize" );
      glUniform2f( uCellSize, image->cellSize.x, image->cellSize.y );

      Int32 uGridSize = shader->program->getUniform( "gridSize" );
      glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glEnableVertexAttribArray( aPos );
      glBindBuffer( GL_ARRAY_BUFFER, obj->bufQuads );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

      Int32 uPtrCellStreams = shader->program->getUniform( "ptrCellStreams" );
      glUniformui64( uPtrCellStreams, image->ptrCellStreams );

      Int32 uImgCellCounters = shader->program->getUniform( "imgCellCounters" );
      glUniform1i( uImgCellCounters, 1 );
      
      glDrawArrays( GL_TRIANGLES, 0, obj->quads.size() * 3 );
      glDisableVertexAttribArray( aPos );
    }

    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT | GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );

    /////////////////////////////////////////////////////
    // Encode object properties into stream
    {
#if (GPU_USE_AUX)
      Shader *shader = shaderEncodeObjectAux;
      shader->use();
#else
      Shader *shader = shaderEncodeObject;
      shader->use();
#endif

      Int32 uPtrCellStreams = shader->program->getUniform( "ptrCellStreams" );
      glUniformui64( uPtrCellStreams, image->ptrCellStreams );

      Int32 uImgCellCounters = shader->program->getUniform( "imgCellCounters" );
      glUniform1i( uImgCellCounters, 1 );

      Int32 uPtrLinesQuads = shader->program->getUniform( "ptrLinesQuads" );
      glUniformui64( uPtrLinesQuads, obj->ptrLinesQuads );
      
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_1D, obj->texGrid );

      Int32 uSampGrid = shader->program->getUniform( "sampGrid" );
      glUniform1i( uSampGrid, 0 );
      
      Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
      glUniform2f( uGridOrigin, image->min.x, image->min.y );

      Int32 uCellSize = shader->program->getUniform( "cellSize" );
      glUniform2f( uCellSize, image->cellSize.x, image->cellSize.y );

      Int32 uGridSize = shader->program->getUniform( "gridSize" );
      glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

      Int32 uColor = shader->program->getUniform( "color" );
      glUniform4fv( uColor, 1, (GLfloat*) &obj->color );
      
      //Transform and round object bounds to grid space
      Vec2 min = Vec::Floor( (obj->min - image->min) / image->cellSize );
      Vec2 max = Vec::Ceil( (obj->max - image->min) / image->cellSize );

      //Transform to [-1,1] normalized coordinates (glViewport will transform back)
      min = (min / Vec2( (float)image->gridSize.x, (float)image->gridSize.y )) * 2.0f - Vec2(1.0f,1.0f);
      max = (max / Vec2( (float)image->gridSize.x, (float)image->gridSize.y )) * 2.0f - Vec2(1.0f,1.0f);

      renderQuad( shader, min, max );
    }

    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT | GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );

    /*
#if (GPU_USE_AUX)

    /////////////////////////////////////////////////////
    // Encode auxiliary vertical segments
    {
      Shader *shader = shaderEncodeAux;
      shader->use();

      Int32 uPtrCellStreams = shader->program->getUniform( "ptrCellStreams" );
      glUniformui64( uPtrCellStreams, image->ptrCellStreams );

      Int32 uImgCellCounters = shader->program->getUniform( "imgCellCounters" );
      glUniform1i( uImgCellCounters, 1 );

      Int32 uGridSize = shader->program->getUniform( "gridSize" );
      glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

      renderFullScreenQuad( shader );
    }

    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT | GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );

#endif//GPU_USE_AUX
    */
  }

  /*
  Float *testBuf = new Float[ o->gridSize.x * o->gridSize.y * COUNTER_LEN ];
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, testBuf );
  Int32* uTestBuf = (Int32*) testBuf;
  for (int y=0; y<o->gridSize.y; ++y) {
    for (int x=0; x<o->gridSize.x; ++x) {
      std::cout << uTestBuf[ y * o->gridSize.x + x ] << " ";
    }
    std::cout << std::endl;
  }
  delete[] testBuf;
  */

  checkGlError( "encodeImage encode" );

  glViewport( 0,0, resX, resY );
}

void encodeImageObj (Image *image)
{ 
  glViewport( 0, 0, image->gridSize.x, image->gridSize.y );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_MULTISAMPLE );

  /////////////////////////////////////////////////////
  // Init object data

  image->objs.clear();
  image->objInfos.clear();

  //First object grid comes after the main grid
  int gridOffset = image->gridSize.x * image->gridSize.y * NUM_CELL_COUNTERS;

  for (int o=0; o<(int)image->objects.size(); ++o)
  {
    Object *object = image->objects[o];

    //Transform object bounds into grid space
    ivec2 gridMin = Vec::Floor( (object->min - image->gridOrigin) / image->cellSize ).toInt();
    ivec2 gridMax = Vec::Ceil( (object->max - image->gridOrigin) / image->cellSize ).toInt();

    //Find object grid dimensions
    ivec2 objGridOrigin = gridMin;
    ivec2 objGridSize = gridMax - gridMin;

    //Find object grid offset
    int objGridOffset = gridOffset;
    gridOffset += objGridSize.x * objGridSize.y * NUM_OBJCELL_COUNTERS;

    //Store object data
    Obj obj;
    obj.min = vec3( object->min, (float) o );
    obj.max = vec3( object->max, (float) o );
    image->objs.push_back( obj );

    ObjInfo objInfo;
    objInfo.gridOrigin = objGridOrigin;
    objInfo.gridSize = objGridSize;
    objInfo.gridOffset = objGridOffset;
    image->objInfos.push_back( objInfo );
  }

  glBindBuffer( GL_ARRAY_BUFFER, image->bufObjs );
  glBufferSubData( GL_ARRAY_BUFFER, 0, image->objs.size() * sizeof(Obj), &image->objs[0] );

  glBindBuffer( GL_ARRAY_BUFFER, image->bufGpuObjObjects );
  glBufferSubData( GL_ARRAY_BUFFER, 0, image->objInfos.size() * sizeof(ObjInfo), &image->objInfos[0] );
  checkGlError( "encodeImageObj init objects" );
  
  /////////////////////////////////////////////////////
  // Init size counters and main grid
  {
    Shader *shader = shaderEncodeObjInit;
    shader->use();

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuObjInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuObjGrid );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    renderFullScreenQuad( shader );
  }

  /////////////////////////////////////////////////////
  // Init object grids
  {
    Shader *shader = shaderEncodeObjInitObject;
    shader->use();
    
    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrGpuObjObjects );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuObjGrid );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, image->gridOrigin.x, image->gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, image->cellSize.x, image->cellSize.y );

    Int32 aPos = shader->program->getAttribute( "in_pos" );
    glBindBuffer( GL_ARRAY_BUFFER, image->bufObjs );
    glVertexAttribPointer( aPos, 3, GL_FLOAT, false, sizeof(vec3), 0 );

    glEnableVertexAttribArray( aPos );
    glDrawArrays( GL_LINES, 0, image->objs.size() * 2 );
    glDisableVertexAttribArray( aPos );
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage init" );

  /*
  glFinish();

  glBindBuffer( GL_ARRAY_BUFFER, image->bufGpuObjGrid );
  glMakeBufferNonResident( GL_ARRAY_BUFFER );
  int *ptr = (int*) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_ONLY );
  checkGlError( "encodeImage map" );

  for (int o=0; o<(int)image->objInfos.size(); ++o)
  {
    ObjInfo &obj = image->objInfos[o];
    int *ptrObjGrid = ptr + obj.gridOffset;
    for (int x=0; x<obj.gridSize.x; ++x) {
      for (int y=0; y<obj.gridSize.y; ++y) {

        int *ptrObjCell = ptrObjGrid + (y * obj.gridSize.x + x) * NUM_OBJCELL_COUNTERS;
        std::cout << ptrObjCell[0] << "," << ptrObjCell[1] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
  glUnmapBuffer( GL_ARRAY_BUFFER );
  */
  
  /////////////////////////////////////////////////////
  // Encode object lines
  {
    Shader *shader = shaderEncodeObjLines;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrGpuObjObjects );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuObjInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuObjGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuObjStream );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, image->gridOrigin.x, image->gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, image->cellSize.x, image->cellSize.y );

    for (int o=0; o<(int)image->objects.size(); ++o)
    {
      Object *object = image->objects[o];
      ObjInfo &obj = image->objInfos[o];

      Int32 uObjectId = shader->program->getUniform( "objectId" );
      glUniform1i( uObjectId, o );

      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glBindBuffer( GL_ARRAY_BUFFER, object->bufLines );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

      glEnableVertexAttribArray( aPos );      
      glDrawArrays( GL_LINES, 0, object->lines.size() * 2 );
      glDisableVertexAttribArray( aPos );
    }
  }

  /////////////////////////////////////////////////////
  // Encode object quads
  {
    Shader *shader = shaderEncodeObjQuads;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrGpuObjObjects );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuObjInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuObjGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuObjStream );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, image->gridOrigin.x, image->gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, image->cellSize.x, image->cellSize.y );

    for (int o=0; o<(int)image->objects.size(); ++o)
    {
      Object *object = image->objects[o];
      ObjInfo &obj = image->objInfos[o];

      Int32 uObjectId = shader->program->getUniform( "objectId" );
      glUniform1i( uObjectId, o );

      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glBindBuffer( GL_ARRAY_BUFFER, object->bufQuads );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

      glEnableVertexAttribArray( aPos );
      glDrawArrays( GL_TRIANGLES, 0, object->quads.size() * 3 );
      glDisableVertexAttribArray( aPos );
    }
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage lines quads" );
  
  /////////////////////////////////////////////////////
  // Encode object properties into stream
  {
    Shader *shader = shaderEncodeObjObject;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrGpuObjObjects );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuObjInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuObjGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuObjStream );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, image->gridOrigin.x, image->gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, image->cellSize.x, image->cellSize.y );

    for (int o=0; o<(int)image->objects.size(); ++o)
    {
      Object *object = image->objects[o];
      ObjInfo &obj = image->objInfos[o];

      //
      Int32 uPtrObjGrid = shader->program->getUniform( "ptrObjGrid" );
      glUniformui64( uPtrObjGrid, image->ptrGpuObjGrid + obj.gridOffset * sizeof(int) );

      Int32 uObjGridOrigin = shader->program->getUniform( "objGridOrigin" );
      glUniform2i( uObjGridOrigin, obj.gridOrigin.x, obj.gridOrigin.y );

      Int32 uObjGridSize = shader->program->getUniform( "objGridSize" );
      glUniform2i( uObjGridSize, obj.gridSize.x, obj.gridSize.y );
      //

      Int32 uObjectId = shader->program->getUniform( "objectId" );
      glUniform1i( uObjectId, o );

      Int32 uColor = shader->program->getUniform( "color" );
      glUniform4fv( uColor, 1, (GLfloat*) &object->color );
      
      //Transform and round object bounds to grid space
      Vec2 min = Vec::Floor( (object->min - image->gridOrigin) / image->cellSize );
      Vec2 max = Vec::Ceil( (object->max - image->gridOrigin) / image->cellSize );

      //Transform to [-1,1] normalized coordinates (glViewport will transform back)
      min = (min / image->gridSize.toFloat()) * 2.0f - Vec2(1.0f,1.0f);
      max = (max / image->gridSize.toFloat()) * 2.0f - Vec2(1.0f,1.0f);

      renderQuad( shader, min, max );
    }
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage object" );

  /////////////////////////////////////////////////////
  // Sort objects in every cell back to front
  {
    Shader *shader = shaderEncodeObjSort;
    shader->use();

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuObjGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuObjStream );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    renderFullScreenQuad( shader );
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage sort" );

  glViewport( 0,0, resX, resY );
}

void renderImageGrid (Image *i)
{
  glDisable( GL_DEPTH_TEST );

  Shader *shader = shaderGrid;
  shader->use();

  //glMatrixMode( GL_MODELVIEW );
  //glPushMatrix();
  //glTranslatef( -o->min.x, -o->min.y, 0);

  Int32 modelview = shader->program->getUniform( "modelview" );
  glUniformMatrix4fv( modelview, 1, false, (GLfloat*) matModelView.top().m );

  Int32 projection = shader->program->getUniform( "projection" );
  glUniformMatrix4fv( projection, 1, false, (GLfloat*) matProjection.top().m );

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glBindBuffer( GL_ARRAY_BUFFER, i->bufPivotPos );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, 2 * sizeof( Float ), 0 );

  Int32 wind = shader->program->getAttribute( "in_wind" );
  glBindBuffer( GL_ARRAY_BUFFER, i->bufPivotWind );
  glVertexAttribIPointer( wind, 1, GL_INT, sizeof( int ), 0 );

  glPointSize( 5.0f );
  glEnableVertexAttribArray( pos );
  glEnableVertexAttribArray( wind );
  glDrawArrays( GL_POINTS, 0, i->gridSize.x * i->gridSize.y );
  glDisableVertexAttribArray( pos );
  glDisableVertexAttribArray( wind );

  //glPopMatrix();
}

void renderImage (Image *image, VertexBuffer *buf, GLenum mode)
{
  glColor3f( 0,0,0 );
  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );

#if (GPU_USE_AUX)
  Shader *shader = shaderCellAux;
  shader->use();
#else
  Shader *shader = shaderCell;
  shader->use();
#endif

  Int32 modelview = shader->program->getUniform( "modelview" );
  glUniformMatrix4fv( modelview, 1, false, (GLfloat*) matModelView.top().m );

  Int32 projection = shader->program->getUniform( "projection" );
  glUniformMatrix4fv( projection, 1, false, (GLfloat*) matProjection.top().m );

  glBindImageTexture( 1, image->texCellCounters, 0, true, 0, GL_READ_ONLY, GL_R32I );
  //glBindImageTexture( 1, image->texCpuCounters, 0, true, 0, GL_READ_ONLY, GL_R32I );

  Int32 uPtrCellStreams = shader->program->getUniform( "ptrCellStreams" );
  glUniformui64( uPtrCellStreams, image->ptrCellStreams );
  //glUniformui64( uPtrCellStreams, image->ptrCpuStream );
  
  Int32 uImgCellCounters = shader->program->getUniform( "imgCellCounters" );
  glUniform1i( uImgCellCounters, 1 );
  
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  Int32 cellSize = shader->program->getUniform( "cellSize" );
  glUniform2f( cellSize, image->cellSize.x, image->cellSize.y );

  Int32 gridSize = shader->program->getUniform( "gridSize" );
  glUniform2i( gridSize, image->gridSize.x, image->gridSize.y );

  Int32 gridOrigin = shader->program->getUniform( "gridOrigin" );
  glUniform2f( gridOrigin, image->min.x, image->min.y );

  Int32 viewOrigin = shader->program->getUniform( "viewOrigin" );
  glUniform2f( viewOrigin, 0.0f, 0.0f );

  Int32 viewSize = shader->program->getUniform( "viewSize" );
  glUniform2f( viewSize, (float) resX, (float) resY );
  
  renderVertexBuffer( shader, buf, mode );

  glDisable( GL_BLEND );
}

void renderImageObj (Image *image, VertexBuffer *buf, GLenum mode)
{
  glColor3f( 0,0,0 );
  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );

  Shader *shader = shaderCellObj;
  shader->use();

  Int32 modelview = shader->program->getUniform( "modelview" );
  glUniformMatrix4fv( modelview, 1, false, (GLfloat*) matModelView.top().m );

  Int32 projection = shader->program->getUniform( "projection" );
  glUniformMatrix4fv( projection, 1, false, (GLfloat*) matProjection.top().m );

  Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
  glUniformui64( uPtrGrid, image->ptrGpuObjGrid );

  Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
  glUniformui64( uPtrStream, image->ptrGpuObjStream );
  
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  Int32 cellSize = shader->program->getUniform( "cellSize" );
  glUniform2f( cellSize, image->cellSize.x, image->cellSize.y );

  Int32 gridSize = shader->program->getUniform( "gridSize" );
  glUniform2i( gridSize, image->gridSize.x, image->gridSize.y );

  Int32 gridOrigin = shader->program->getUniform( "gridOrigin" );
  glUniform2f( gridOrigin, image->min.x, image->min.y );

  Int32 viewOrigin = shader->program->getUniform( "viewOrigin" );
  glUniform2f( viewOrigin, 0.0f, 0.0f );

  Int32 viewSize = shader->program->getUniform( "viewSize" );
  glUniform2f( viewSize, (float) resX, (float) resY );
  
  renderVertexBuffer( shader, buf, mode );

  glDisable( GL_BLEND );
}

void renderImageCpuObj (Image *image, VertexBuffer *buf, GLenum mode)
{
  glColor3f( 0,0,0 );
  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );

  Shader *shader = shaderCellObj;
  shader->use();

  Int32 modelview = shader->program->getUniform( "modelview" );
  glUniformMatrix4fv( modelview, 1, false, (GLfloat*) matModelView.top().m );

  Int32 projection = shader->program->getUniform( "projection" );
  glUniformMatrix4fv( projection, 1, false, (GLfloat*) matProjection.top().m );

  Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
  glUniformui64( uPtrGrid, image->ptrObjGrid );

  Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
  glUniformui64( uPtrStream, image->ptrObjStream );
  
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  Int32 cellSize = shader->program->getUniform( "cellSize" );
  glUniform2f( cellSize, image->cellSize.x, image->cellSize.y );

  Int32 gridSize = shader->program->getUniform( "gridSize" );
  glUniform2i( gridSize, image->gridSize.x, image->gridSize.y );

  Int32 gridOrigin = shader->program->getUniform( "gridOrigin" );
  glUniform2f( gridOrigin, image->min.x, image->min.y );

  Int32 viewOrigin = shader->program->getUniform( "viewOrigin" );
  glUniform2f( viewOrigin, 0.0f, 0.0f );

  Int32 viewSize = shader->program->getUniform( "viewSize" );
  glUniform2f( viewSize, (float) resX, (float) resY );
  
  renderVertexBuffer( shader, buf, mode );

  glDisable( GL_BLEND );
}

void renderImage1to1 ()
{
  //Setup quad buffer

  Vec2 off( 0, 0 );
  Vec2 sz = image->max - image->min;

  Vec2 coords[] = {
    Vec2( off.x,        off.y ),
    Vec2( off.x + sz.x, off.y ),
    Vec2( off.x + sz.x, off.y + sz.y ),
    Vec2( off.x,        off.y + sz.y )
  };

  Vec2 texcoords[] = {
    Vec2( image->min.x, image->min.y ),
    Vec2( image->max.x, image->min.y ),
    Vec2( image->max.x, image->max.y ),
    Vec2( image->min.x, image->max.y )
  };

  static bool bufInit = false;
  static VertexBuffer buf;
  if (!bufInit)
  {
    bufInit = true;
    for (int v=0; v<4; ++v)
    {
      Vertex vert;
      vert.coord = texcoords[v].xy( 0.0f );
      vert.texcoord = texcoords[v];
      buf.verts.push_back( vert );
    }
    buf.toGpu();
  }

  //Render

  matModelView.push();
  //matModelView.translate( 100, 100, 0 );
  matModelView.translate( 180, 400, 0 );
  matModelView.scale( 1.0f, -1.0f, 1.0f );

  glDisable( GL_DEPTH_TEST );

  if (cpu)
  {
    if (encode) image->updateStream();
    if (draw) renderImageCpuObj( image, &buf, GL_QUADS );

    //glBindBuffer( GL_ARRAY_BUFFER, image->bufCpuStream );
    //glBufferSubData( GL_ARRAY_BUFFER, 0, MAX_COMBINED_STREAM_LEN * sizeof(GLfloat), image->cpuStream );

    //glBindTexture( GL_TEXTURE_2D_ARRAY, image->texCpuCounters );
    //glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, image->gridSize.x, image->gridSize.y, COUNTER_LEN,
      //GL_RED, GL_FLOAT, image->cpuCounters );

    //if (draw) renderImageCpu( image, &buf, GL_QUADS );
  }
  else 
  {
    //if (encode) encodeImage( image );
    //if (draw) renderImage( image, &buf, GL_QUADS );
    if (encode) encodeImageObj( image );
    if (draw) renderImageObj( image, &buf, GL_QUADS );
  }
  
  if (drawGrid) renderImageGrid( image );
  
  matModelView.pop();
}

void renderImageCylinder()
{
  //Setup cylinder buffer
  static bool bufInit = false;
  static VertexBuffer buf;

  if (!bufInit)
  {
    bufInit = true;
    int numSteps = 60;
    float aStep = 2 * PI / numSteps;
    float tStep = 1.0f / numSteps;

    for (int s=0; s<=numSteps; ++s)
    {
      float a = s * aStep;
      float t = s * tStep;
      float cosa = COS( a );
      float sina = SIN( a );
      
      Vertex vertTop;
      Vertex vertBtm;

      vertTop.coord.set( -sina, +1.0f, cosa );
      vertBtm.coord.set( -sina, -1.0f, cosa );

      vertTop.texcoord.set( t, 0.0f );
      vertBtm.texcoord.set( t, 1.0f );

      Vec2 sz = image->max - image->min;
      vertTop.texcoord = image->min + vertTop.texcoord * sz;
      vertBtm.texcoord = image->min + vertBtm.texcoord * sz;
      
      buf.verts.push_back( vertTop );
      buf.verts.push_back( vertBtm );
    }

    buf.toGpu();
  }

  matProjection.push();
  matModelView.push();

  Matrix4x4 mproj;
  mproj.setPerspectiveFovLH( PI/4, (float)resX/resY, 0.01f, 1000.0f );

  matProjection.load( mproj );
  matModelView.identity();
  matModelView.translate( 0.0f, 0.0f, zoomZ );
  matModelView.rotate( 1.0f, 0.0f, 0.0f, angleY );
  matModelView.rotate( 0.0f, 1.0f, 0.0f, angleX );
  matModelView.scale( 1.0f, 2.0f, 1.0f );

  if (encode) encodeImage( image );
  glEnable( GL_DEPTH_TEST );
  if (draw) renderImage( image, &buf, GL_TRIANGLE_STRIP );

  matProjection.pop();
  matModelView.pop();
}

void display ()
{
  glClearColor( 1,1,1,1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if (drawCylinder) renderImageCylinder();
  else renderImage1to1();

  //Check fps
  static int fps = 0;
  static int lastUpdate = glutGet( GLUT_ELAPSED_TIME );
  fps++;

  int now = glutGet( GLUT_ELAPSED_TIME );
  if (now - lastUpdate > 1000) {
    std::cout << "Fps: " << fps << std::endl;
    lastUpdate = now;
    fps = 0;
  }

  //Check for errors
  checkGlError( "display end" );
  glutSwapBuffers();
}

void reshape (int w, int h)
{
  std::cout << "reshape (" << w << "," << h << ")" << std::endl;

  resX = w;
  resY = h;

  //glMatrixMode( GL_PROJECTION );
  //glLoadIdentity();
  //gluOrtho2D( 0, w, 0, h );

  //glMatrixMode( GL_MODELVIEW );
  //glLoadIdentity();

  Matrix4x4 m;
  m.setOrthoLH( 0, (Float)w, 0, (Float)h, -1.0f, 1.0f );
  matProjection.clear();
  matProjection.load( m );
  matModelView.clear();
  
  glViewport( 0,0,w,h );
}

void animate ()
{
  glutPostRedisplay();
}

void specialKey (int key, int x, int y)
{
  if (key == GLUT_KEY_F7)
  {
    std::cout << "Compiling..." << std::endl;
    //shader1->load();
    //shaderQuad->load();
    //shaderStream->load();
  }
  else if (key == GLUT_KEY_F1)
  {
    cpu = !cpu;
    std::cout << (cpu ? "Using CPU stream" : "Using GPU stream" ) << std::endl;
  }
  else if (key == GLUT_KEY_F2)
  {
    encode = !encode;
    std::cout << (encode ? "Encoding ON" : "Encoding OFF" ) << std::endl;
  }
  else if (key == GLUT_KEY_F3)
  {
    draw = !draw;
    std::cout << (draw ? "Drawing ON" : "Drawing OFF" ) << std::endl;
  }
  else if (key == GLUT_KEY_F4)
  {
    drawGrid = !drawGrid;
    std::cout << (drawGrid ? "Grid ON" : "Grid OFF" ) << std::endl;
  }
  else if (key == GLUT_KEY_F5)
  {
    drawCylinder = !drawCylinder;
    std::cout << (drawCylinder ? "Drawing Cylinder" : "Drawing 1:1" ) << std::endl;
  }
}

void mouseClick (int button, int state, int x, int y)
{
  mouseButton = button;
  mouseDown.set( (Float)x, (Float)y );
}

void mouseMove (int x, int y)
{
  Vec2 mouse( (Float)x, (Float)y );
  Vec2 mouseDiff = mouse - mouseDown;
  mouseDown = mouse;

  if (mouseButton == GLUT_LEFT_BUTTON)
  {
    angleX += -mouseDiff.x * 0.5f * (PI / 180);
    angleY += -mouseDiff.y * 0.5f * (PI / 180);
  }
  else if (mouseButton == GLUT_RIGHT_BUTTON)
  {
    zoomZ *= 1.0f + mouseDiff.y / 100.0f;
  }
}

void rvgGlutInit (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitContextVersion( 3, 0 );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_STENCIL | GLUT_DEPTH );
  //glutInitDisplayString( "rgba alpha double stencil depth samples=32" );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  int ret = glutCreateWindow( "Random-access Vector Graphics" );

  //glutGameModeString( "1280x1024:32@60" );
  //glutEnterGameMode();
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  //glutKeyboardFunc( keyDown );
  //glutKeyboardUpFunc( keyUp );
  glutSpecialFunc( specialKey );
  glutMouseFunc( mouseClick );
  glutMotionFunc( mouseMove );
  //glutPassiveMotionFunc( mouseMove );
  glutIdleFunc( animate );

  int numSamples = 0;
  glGetIntegerv( GL_SAMPLES, &numSamples );
  std::cout << "Number of FSAA samples: " << numSamples << std::endl;
}

void calcSamples()
{
  float off[4] = { -0.3f, -0.1f, +0.1f, +0.3f };
  float a = PI / 6;

  std::ofstream file( "samples.txt", std::ios::out | std::ios::binary );
  if (!file.is_open())
    return;

  int s=0;
  for (int x=0; x<4; ++x) {
    for (int y=0; y<4; ++y) {

      float newx = COS(a) * off[x] - SIN(a) * off[y];
      float newy = SIN(a) * off[x] + COS(a) * off[y];
      file << "samples[" << s << "] = vec2( " << newx << ", " << newy << " );\r\n";
      s++;
    }
  }

  file.close();
}

void processCommands (Object *o, int i)
{
  int numCommands = commandCounts[ i ];
  const VGfloat *data = dataArrays[ i ];
  const VGubyte *commands = commandArrays[ i ];
 
  int d = 0;
  for (int c=0; c<numCommands; c++)
  {
    VGubyte cmd = commands[ c ];
    switch (cmd)
    {
    case VG_MOVE_TO_ABS:
      {
      float x = data[ d++ ];
      float y = data[ d++ ];
      o->moveTo( x, y );
      break;
      }
    case VG_LINE_TO_ABS:
      {
      float x0 = data[ d++ ];
      float y0 = data[ d++ ];
      o->lineTo( x0, y0 );
      break;
      }
    case VG_CUBIC_TO_ABS:
      {
      float x0 = data[ d++ ];
      float y0 = data[ d++ ];
      float x1 = data[ d++ ];
      float y1 = data[ d++ ];
      float x2 = data[ d++ ];
      float y2 = data[ d++ ];
      o->cubicTo( x0,y0, x1,y1, x2,y2 );
      break;
      }
    case VG_CLOSE_PATH:
      o->close();
      break;
    }
  }

  //if (commands[ numCommands-1 ] != VG_CLOSE_PATH)
    //o->close();
}

int main (int argc, char **argv)
{
  rvgGlutInit( argc, argv );
  rvgGLInit();
  wglSwapInterval( 0 );

  Shader::Define( "INFO_COUNTER_STREAMLEN", "0" );
  Shader::Define( "INFO_COUNTER_GRIDLEN",   "1" );
  Shader::Define( "NUM_INFO_COUNTERS",      "2" );

  Shader::Define( "OBJCELL_COUNTER_PREV",   "0" );
  Shader::Define( "OBJCELL_COUNTER_AUX",    "1" );
  Shader::Define( "NUM_OBJCELL_COUNTERS",   "2" );

  Shader::Define( "CELL_COUNTER_PREV",      "0" );
  Shader::Define( "NUM_CELL_COUNTERS",      "1" );

  Shader::Define( "NODE_TYPE_LINE",         "1" );
  Shader::Define( "NODE_TYPE_QUAD",         "2" );
  Shader::Define( "NODE_TYPE_OBJECT",       "3" );

  shader1 = new Shader( "shader.vert.c", "shader.frag.c" );
  shader1->load();

  shaderQuad = new Shader( "curvequad.vert.c", "curvequad.frag.c" );
  shaderQuad->load();

  shaderStream = new Shader( "stream.vert.c", "stream.frag.c" );
  shaderStream->load();

  shaderGrid = new Shader( "grid.vert.c", "grid.frag.c" );
  shaderGrid->load();

  shaderEncodeInit = new Shader( "encode_init.vert.c", "encode_init.frag.c" );
  shaderEncodeInit->load();

  shaderEncodeObject = new Shader( "encode_object.vert.c", "encode_object.frag.c" );
  shaderEncodeObject->load();

  shaderEncodeObjectAux = new Shader( "encode_object_aux.vert.c", "encode_object_aux.frag.c" );
  shaderEncodeObjectAux->load();

  shaderEncodeLines = new Shader( "encode_lines.vert.c", "encode_lines.geom.c", "encode_lines.frag.c" );
  shaderEncodeLines->load();

  shaderEncodeLinesAux = new Shader( "encode_lines_aux.vert.c", "encode_lines_aux.geom.c", "encode_lines_aux.frag.c" );
  shaderEncodeLinesAux->load();

  shaderEncodeQuads = new Shader( "encode_quads.vert.c", "encode_quads.geom.c", "encode_quads.frag.c" );
  shaderEncodeQuads->load();

  shaderEncodeQuadsAux = new Shader( "encode_quads_aux.vert.c", "encode_quads_aux.geom.c", "encode_quads_aux.frag.c" );
  shaderEncodeQuadsAux->load();

  shaderEncodeAux = new Shader( "encode_aux.vert.c", "encode_aux.frag.c" );
  shaderEncodeAux->load();

  shaderCell = new Shader( "cell.vert.c", "cell.frag.c" );
  shaderCell->load();

  shaderCellAux = new Shader( "cell_aux.vert.c", "cell_aux.frag.c" );
  shaderCellAux->load();

  //shaderCellAux = new Shader( "cell_aux.vert.c", "cell_aux.geom.c", "cell_aux.frag.c" );
  //shaderCellAux->load();


  shaderEncodeObjInit = new Shader(
    "shaders_obj/encode_obj_init.vert.c",
    "shaders_obj/encode_obj_init.frag.c" );
  shaderEncodeObjInit->load();

  shaderEncodeObjInitObject = new Shader(
    "shaders_obj/encode_obj_initobject.vert.c",
    "shaders_obj/encode_obj_initobject.geom.c",
    "shaders_obj/encode_obj_initobject.frag.c" );
  shaderEncodeObjInitObject->load();

  shaderEncodeObjLines = new Shader(
    "shaders_obj/encode_obj_lines.vert.c",
    "shaders_obj/encode_obj_lines.geom.c",
    "shaders_obj/encode_obj_lines.frag.c" );
  shaderEncodeObjLines->load();

  shaderEncodeObjQuads = new Shader(
    "shaders_obj/encode_obj_quads.vert.c",
    "shaders_obj/encode_obj_quads.geom.c",
    "shaders_obj/encode_obj_quads.frag.c" );
  shaderEncodeObjQuads->load();

  shaderEncodeObjObject = new Shader(
    "shaders_obj/encode_obj_object.vert.c",
    "shaders_obj/encode_obj_object.frag.c" );
  shaderEncodeObjObject->load();

  shaderEncodeObjSort = new Shader(
    "shaders_obj/encode_obj_sort.vert.c",
    "shaders_obj/encode_obj_sort.frag.c" );
  shaderEncodeObjSort->load();

  shaderCellObj = new Shader(
    "shaders_obj/cell_obj.vert.c",
    "shaders_obj/cell_obj.frag.c" );
  shaderCellObj->load();


  object1 = new Object();
  object2 = new Object();
  image = new Image();

  /*
  object1->moveTo( 100,100 );
  object1->quadTo( 150,140, 200,100 );
  object1->lineTo( 150,150 );
  object1->quadTo( 130,200, 110,150 );
  object1->close();
  */
  /*
  object1->moveTo( 140,100 );
  //object1->cubicTo( 100,200, 200,200, 200,100 );
  object1->cubicTo( 200,200, 100,200, 160,100 );
  //object1->cubicTo( 200,0, 100,0, 100,100 );
  object1->close();
  */

  //object1->cubicsToQuads();
  //image->objects.push_back( object1 );

  
  GLint param;
  glGetIntegerv( GL_MAX_TEXTURE_SIZE, &param);

  const int VG_FILL_PATH  (1 << 1);
  for (int i=0; i<pathCount; ++i)
  //for (int i=100; i<102; ++i)
  //for (int i=12; i<14; ++i)
  {
    const float *style = styleArrays[i];
    if (! (((int)style[9]) & VG_FILL_PATH)) continue;

    Object *obj = new Object();
    processCommands( obj, i );
    
    obj->color.set( style[4], style[5], style[6], style[7] );
    //if (obj->color == Vec4( 1,1,1,1 ))
      //obj->color = Vec4( 0,0,0,1 );

    obj->cubicsToQuads();
    image->objects.push_back( obj );
  }
  
  image->updateBounds();
  //image->updateGrid();
  image->updateStream();
  image->updateBuffers();

  std::cout << "Total stream len: " << image->cpuStreamLen << std::endl;
  //std::cout << "Maximum cell len: " << maxCellLen << std::endl;

  int start = glutGet( GLUT_ELAPSED_TIME );
  image->updateStream();
  int end = glutGet( GLUT_ELAPSED_TIME );
  int time1 = end - start;

  start = glutGet( GLUT_ELAPSED_TIME );
  image->updateStream();
  end = glutGet( GLUT_ELAPSED_TIME );
  int time2 = end - start;

  start = glutGet( GLUT_ELAPSED_TIME );
  image->updateStream();
  end = glutGet( GLUT_ELAPSED_TIME );
  int time3 = end - start;

  std::cout << time1 << "ms" << std::endl;
  std::cout << time2 << "ms" << std::endl;
  std::cout << time3 << "ms" << std::endl;

  glutMainLoop();
}
