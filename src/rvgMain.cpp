#include "rvgMain.h"

////////////////////////////////////////////////////////////////////
// Global vars

int resX = 500;
int resY = 600;
int gridResX = 60;
int gridResY = 60;

namespace Proc {
  enum Enum {
    Cpu   = 0,
    Gpu   = 1,
    Count = 2
  };
};

namespace Rep {
  enum Enum {
    Aux   = 0,
    Pivot = 1,
    Count = 2
  };
};

namespace View {
  enum Enum {
    Classic         = 0,
    RandomDirect    = 1,
    RandomCylinder  = 2,
    Count           = 3
  };
};

int proc = Proc::Gpu;
int rep = Rep::Pivot;
int view = View::RandomDirect;

bool encode = false;
bool draw = true;
bool drawGrid = false;

MatrixStack matModelView;
MatrixStack matProjection;

Shader *shaderGrid;

Shader *shaderEncodeAuxInit;
Shader *shaderEncodeAuxInitObject;
Shader *shaderEncodeAuxLines;
Shader *shaderEncodeAuxQuads;
Shader *shaderEncodeAuxObject;
Shader *shaderEncodeAuxSort;
Shader *shaderRenderAux;

Shader *shaderEncodePivotInit;
Shader *shaderEncodePivotInitObject;
Shader *shaderEncodePivotLines;
Shader *shaderEncodePivotQuads;
Shader *shaderEncodePivotObject;
Shader *shaderEncodePivotSort;
Shader *shaderRenderPivot;

Shader *shaderClassicQuads;
Shader *shaderClassicContour;
Shader *shaderClassic;

Object *object1;
Object *object2;
Image *image;
ImageEncoder *imageEncoderAux;
ImageEncoder *imageEncoderPivot;

int mouseButton = 0;
Vec2 mouseDown;

Float angleX = 0.0f;
Float angleY = 0.0f;
Float zoomZ = 6.0f;

Float panX = 0.0f;
Float panY = 0.0f;
Float zoomS = 1.0f;

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

Object::~Object()
{
  for (Uint32 c=0; c<contours.size(); ++c)
    delete contours[c];
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

void cubicToQuads (const Cubic &cu, std::vector< Quad > &out)
{
  /*
  //Generic recursive subdivision
  Cubic c1,c2;
  subdivCubic( cu, c1,c2 );
  
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

    out.push_back( quad );
  }
  */

  //Fixed subdivision into 4 quads
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

  out.push_back( Quad( cu.p0, a, A ) );
  out.push_back( Quad( A, b, B ) );
  out.push_back( Quad( B, c, C ) );
  out.push_back( Quad( C, d, cu.p3 ) );
}

Object* Object::cubicsToQuads()
{
  Object *obj = new Object();
  obj->color = color;

  std::vector< Quad > quadsFromCubic;

  for (Uint32 c=0; c<contours.size(); ++c)
  {
    Contour *cnt = contours[c];
    int p=0;

    for (Uint32 s=0; s<cnt->segments.size(); ++s)
    {
      int seg = cnt->segments[s];

      if (seg & SegType::MoveTo) {
        vec2 point0 = contour->points[ p ];
        obj->moveTo( point0.x, point0.y );
        p += 1;

      }else if (seg & SegType::LineTo) {
        vec2 point0 = contour->points[ p ];
        obj->lineTo( point0.x, point0.y );
        p += 1;

      }else if (seg & SegType::QuadTo) {
        vec2 point0 = contour->points[ p+0 ];
        vec2 point1 = contour->points[ p+1 ];
        obj->quadTo( point0.x, point0.y, point1.x, point1.y );
        p += 2;

      }else if (seg & SegType::CubicTo) {

        Cubic cubic;
        cubic.p0 = contour->points[ p-1 ];
        cubic.p1 = contour->points[ p+0 ];
        cubic.p2 = contour->points[ p+1 ];
        cubic.p3 = contour->points[ p+2 ];
        p += 3;

        quadsFromCubic.clear();
        cubicToQuads( cubic, quadsFromCubic );
        for (Uint32 q=0; q<quadsFromCubic.size(); ++q) {

          Quad quad = quadsFromCubic[q];
          obj->quadTo( quad.p1.x, quad.p1.y, quad.p2.x, quad.p2.y );
        }
      }
    }
  }

  return obj;
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

    for (Uint32 c=0; c<contours.size(); ++c)
      glGenBuffers( 1, &contours[c]->bufFlatPoints );

    buffersInit = true;
  }

  checkGlError( "Object::updateBuffers init" );

  //////////////////////////////////////
  //Contour flat points
  
  for (Uint32 c=0; c<contours.size(); ++c)
  {
    if (contours[c]->flatPoints.size() > 0)
    {
      Contour *cnt = contours[c];
      glBindBuffer( GL_ARRAY_BUFFER, cnt->bufFlatPoints );
      glBufferData( GL_ARRAY_BUFFER, cnt->flatPoints.size() * sizeof( vec2 ), &cnt->flatPoints[0], GL_STATIC_DRAW );
    }

    checkGlError( "Object::updateBuffers contours" );
  }

  //////////////////////////////////////
  //Line control points
  
  if (lines.size() > 0)
  {
    glBindBuffer( GL_ARRAY_BUFFER, bufLines );
    glBufferData( GL_ARRAY_BUFFER, lines.size() * sizeof( Line ), &lines[0], GL_STATIC_DRAW );

    checkGlError( "Object::updateBuffers lines" );
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

  ptrCpuInfo = NULL;
  ptrCpuGrid = NULL;
  ptrCpuStream = NULL;
  cpuStreamLen = 0;
}

void Image::updateBounds()
{
  /////////////////////////////////////////////////////
  // Find min/max

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


  /////////////////////////////////////////////////////
  // Update object data

  objs.clear();
  objInfos.clear();

  //First object grid comes after the main grid
  int gridOffset = gridSize.x * gridSize.y * NUM_CELL_COUNTERS;
  for (int o=0; o<(int)objects.size(); ++o)
  {
    Object *object = objects[o];

    //Transform object bounds into grid space
    ivec2 gridMin = (ivec2) Vec::Floor( (object->min - gridOrigin) / cellSize );
    ivec2 gridMax = (ivec2) Vec::Ceil( (object->max - gridOrigin) / cellSize );

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

    glGenBuffers( 1, &bufObjs );
    glGenBuffers( 1, &bufObjInfos );

    glGenBuffers( 1, &bufGpuInfo );
    glGenBuffers( 1, &bufGpuGrid );
    glGenBuffers( 1, &bufGpuStream );

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

  ///////////////////////////////////////
  // Object buffers

  glBindBuffer( GL_ARRAY_BUFFER, bufObjs );
  glBufferData( GL_ARRAY_BUFFER, image->objs.size() * sizeof(Obj), &image->objs[0], GL_STATIC_DRAW );
  
  checkGlError( "Image::updateBuffers objs" );

  glBindBuffer( GL_ARRAY_BUFFER, bufObjInfos );
  glBufferData( GL_ARRAY_BUFFER, image->objInfos.size() * sizeof(ObjInfo), &image->objInfos[0], GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrObjInfos );
  
  checkGlError( "Image::updateBuffers gpuobjObjects" );

  //////////////////////////////////////
  //Algorithm buffers

  glBindBuffer( GL_ARRAY_BUFFER, bufGpuInfo );
  glBufferData( GL_ARRAY_BUFFER, NUM_INFO_COUNTERS * sizeof(int), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuInfo );

  checkGlError( "Image::updateBuffers gpuobjInfo" );

  glBindBuffer( GL_ARRAY_BUFFER, bufGpuGrid );
  glBufferData( GL_ARRAY_BUFFER, 500000 * sizeof(int), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuGrid );

  checkGlError( "Image::updateBuffers gpuobjGrid" );

  glBindBuffer( GL_ARRAY_BUFFER, bufGpuStream );
  glBufferData( GL_ARRAY_BUFFER, 500000 * sizeof(float), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuStream );

  checkGlError( "Image::updateBuffers gpuobjStream" );
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

void renderGrid (Image *i)
{
  glDisable( GL_DEPTH_TEST );

  Shader *shader = shaderGrid;
  shader->use();

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
}

void encodeImageAux (Image *image)
{ 
  glViewport( 0, 0, image->gridSize.x, image->gridSize.y );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_MULTISAMPLE );
  
  /////////////////////////////////////////////////////
  // Init size counters and main grid
  {
    Shader *shader = shaderEncodeAuxInit;
    shader->use();

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    renderFullScreenQuad( shader );
  }

  /////////////////////////////////////////////////////
  // Init object grids
  {
    Shader *shader = shaderEncodeAuxInitObject;
    shader->use();
    
    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrObjInfos );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

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
    Shader *shader = shaderEncodeAuxLines;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuStream );

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
    Shader *shader = shaderEncodeAuxQuads;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuStream );

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
    Shader *shader = shaderEncodeAuxObject;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuStream );

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
      glUniformui64( uPtrObjGrid, image->ptrGpuGrid + obj.gridOffset * sizeof(int) );

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
      min = (min / vec2( image->gridSize )) * 2.0f - Vec2(1.0f,1.0f);
      max = (max / vec2( image->gridSize )) * 2.0f - Vec2(1.0f,1.0f);

      renderQuad( shader, min, max );
    }
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage object" );

  /////////////////////////////////////////////////////
  // Sort objects in every cell back to front
  {
    Shader *shader = shaderEncodeAuxSort;
    shader->use();

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuStream );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    renderFullScreenQuad( shader );
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage sort" );

  glViewport( 0,0, resX, resY );
}

void encodeImagePivot (Image *image)
{ 
  glViewport( 0, 0, image->gridSize.x, image->gridSize.y );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_MULTISAMPLE );
  
  /////////////////////////////////////////////////////
  // Init size counters and main grid
  {
    Shader *shader = shaderEncodePivotInit;
    shader->use();

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    renderFullScreenQuad( shader );
  }

  /////////////////////////////////////////////////////
  // Init object grids
  {
    Shader *shader = shaderEncodePivotInitObject;
    shader->use();
    
    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrObjInfos );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

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

  glBindBuffer( GL_ARRAY_BUFFER, image->bufGpuAuxGrid );
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
    Shader *shader = shaderEncodePivotLines;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuStream );

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
    Shader *shader = shaderEncodePivotQuads;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuStream );

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

/*
  glFinish();

  glBindBuffer( GL_ARRAY_BUFFER, image->bufGpuAuxGrid );
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
  // Encode object properties
  {
    Shader *shader = shaderEncodePivotObject;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, image->ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, image->ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuStream );

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
      glUniformui64( uPtrObjGrid, image->ptrGpuGrid + obj.gridOffset * sizeof(int) );

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
      min = (min / vec2( image->gridSize )) * 2.0f - Vec2(1.0f,1.0f);
      max = (max / vec2( image->gridSize )) * 2.0f - Vec2(1.0f,1.0f);

      renderQuad( shader, min, max );
    }
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage object" );

  /////////////////////////////////////////////////////
  // Sort objects in every cell back to front
  {
    Shader *shader = shaderEncodePivotSort;
    shader->use();

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, image->ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, image->ptrGpuStream );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

    renderFullScreenQuad( shader );
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage sort" );

  glViewport( 0,0, resX, resY );
}

void Image::encodeCpu (ImageEncoder *encoder)
{
  //Reset buffers
  if (ptrCpuInfo)        delete[] ptrCpuInfo;
  if (ptrCpuGrid)        delete[] ptrCpuGrid;
  if (ptrCpuStream)      delete[] ptrCpuStream;

  //Init new buffers
  ptrCpuInfo         = new int[ NUM_INFO_COUNTERS ];
  ptrCpuGrid         = new int[ 500000 ];
  ptrCpuStream       = new float[ 500000 ];

  /////////////////////////////////////////////////////
  // Init size counters and main grid

  encoder->ptrInfo = ptrCpuInfo;
  encoder->ptrGrid = ptrCpuGrid;
  encoder->ptrStream = ptrCpuStream;
  encoder->ptrObjects = (int*) &objInfos[0];

  encoder->gridOrigin = gridOrigin;
  encoder->gridSize = gridSize;
  encoder->cellSize = cellSize;

  encoder->encodeInit();

  /////////////////////////////////////////////////////
  // Init object grids

  for (int o=0; o<(int)image->objects.size(); ++o)
  {
    Object *obj = image->objects[o];
    encoder->objectId = o;
    encoder->objMin = obj->min;
    encoder->objMax = obj->max;
    encoder->encodeInitObject();
  }

  /////////////////////////////////////////////////////
  // Encode object lines

  for (int o=0; o<(int)image->objects.size(); ++o)
  {
    Object *obj = image->objects[o];

    for (Uint32 l=0; l<obj->lines.size(); ++l)
    {
      Line &line = obj->lines[l];
      encoder->objectId = o;
      encoder->line0 = line.p0;
      encoder->line1 = line.p1;
      encoder->encodeLine();
    }
  }

  /////////////////////////////////////////////////////
  // Encode object quads

  for (int o=0; o<(int)image->objects.size(); ++o)
  {
    Object *obj = image->objects[o];

    for (Uint32 q=0; q < obj->quads.size(); ++q)
    {
      Quad &quad= obj->quads[q];
      encoder->objectId = o;
      encoder->quad0 = quad.p0;
      encoder->quad1 = quad.p1;
      encoder->quad2 = quad.p2;
      encoder->encodeQuad();
    }
  }

  /////////////////////////////////////////////////////
  // Encode object properties

  for (int o=0; o<(int)image->objects.size(); ++o)
  {
    Object *obj = image->objects[o];
    encoder->objectId = o;
    encoder->objMin = obj->min;
    encoder->objMax = obj->max;
    encoder->color = obj->color;
    encoder->encodeObject();
  }

  /////////////////////////////////////////////////////
  // Sort objects in every cell back to front

  encoder->encodeSort();
  
  //Measure data
  ////////////////////////////////////////////////////
  
  cpuStreamLen = ptrCpuInfo[ INFO_COUNTER_STREAMLEN ];
}

void renderImage (Shader *shader, Image *image, VertexBuffer *buf, GLenum mode)
{
  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );
  
  //glEnable( GL_BLEND );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  shader->use();

  Int32 modelview = shader->program->getUniform( "modelview" );
  glUniformMatrix4fv( modelview, 1, false, (GLfloat*) matModelView.top().m );

  Int32 projection = shader->program->getUniform( "projection" );
  glUniformMatrix4fv( projection, 1, false, (GLfloat*) matProjection.top().m );

  Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
  glUniformui64( uPtrGrid, image->ptrGpuGrid );

  Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
  glUniformui64( uPtrStream, image->ptrGpuStream );

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

  //glDisable( GL_BLEND );
}

void renderImageClassic (Image *image)
{
  matModelView.push();
  matModelView.translate( panX, panY, 0 );
  matModelView.translate( 180, 400, 0 );
  matModelView.scale( 1.0f, -1.0f, 1.0f );
  matModelView.scale( zoomS, zoomS, zoomS );

  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );

  glDisable( GL_DEPTH_TEST );
  glEnable( GL_STENCIL_TEST );

  for (Uint32 o=0; o<image->objects.size(); ++o)
  {
    Object *obj = image->objects[o];

    ///////////////////////////////////////////////
    //Render into stencil

    glStencilFunc( GL_ALWAYS, 0, 1 );
    glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

    ///////////////////////////////////////////////
    //Render quads
    {
      Shader *shader = shaderClassicQuads;
      shader->use();
      
      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );
      
      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glEnableVertexAttribArray( aPos );
      glBindBuffer( GL_ARRAY_BUFFER, obj->bufQuads );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );
      
      glDrawArrays( GL_TRIANGLES, 0, obj->quads.size() * 3 );
      glDisableVertexAttribArray( aPos );
    }
    
    ///////////////////////////////////////////////
    //Render contours
    {
      Shader *shader = shaderClassicContour;
      shader->use();
      
      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

      for (Uint32 c=0; c<obj->contours.size(); ++c)
      {
        Contour *cnt = obj->contours[c];

        Int32 aPos = shader->program->getAttribute( "in_pos" );
        glEnableVertexAttribArray( aPos );
        glBindBuffer( GL_ARRAY_BUFFER, cnt->bufFlatPoints );
        glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

        glDrawArrays( GL_TRIANGLE_FAN, 0, cnt->flatPoints.size() );
        glDisableVertexAttribArray( aPos );
      }
    }

    ///////////////////////////////////////////////
    //Render through stencil

    glStencilFunc( GL_EQUAL, 1, 1 );
    glStencilOp( GL_ZERO, GL_ZERO, GL_ZERO );
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

    ///////////////////////////////////////////////
    //Render boundbox quad
    {
      Shader *shader = shaderClassic;
      shader->use();

      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

      Int32 uColor = shader->program->getUniform( "color" );
      glUniform4fv( uColor, 1, (GLfloat*) &obj->color );

      Float coords[] = {
        obj->min.x, obj->min.y,
        obj->max.x, obj->min.y,
        obj->max.x, obj->max.y,
        obj->min.x, obj->max.y
      };

      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, 2 * sizeof( Float ), coords );

      glEnableVertexAttribArray( aPos );
      glDrawArrays( GL_QUADS, 0, 4 );
      glDisableVertexAttribArray( aPos );
    }
  }

  glDisable( GL_STENCIL_TEST );

  matModelView.pop();
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
  matModelView.translate( panX, panY, 0 );
  matModelView.translate( 180, 400, 0 );
  matModelView.scale( 1.0f, -1.0f, 1.0f );
  matModelView.scale( zoomS, zoomS, zoomS );

  glDisable( GL_DEPTH_TEST );

  switch (rep) {
  case Rep::Aux:   renderImage( shaderRenderAux, image, &buf, GL_QUADS );  break;
  case Rep::Pivot: renderImage( shaderRenderPivot, image, &buf, GL_QUADS ); break;
  }

  if (drawGrid) renderGrid( image );
  
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

  glEnable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );

  switch (rep) {
  case Rep::Aux:   renderImage( shaderRenderAux, image, &buf, GL_TRIANGLE_STRIP );  break;
  case Rep::Pivot: renderImage( shaderRenderPivot, image, &buf, GL_TRIANGLE_STRIP ); break;
  }

  matProjection.pop();
  matModelView.pop();
}

void display ()
{
  glClearColor( 1,1,1,1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if (view == View::Classic)
  {
    //Render classic image
    if (draw) renderImageClassic( image );
  }
  else
  {
    //Encode image
    static bool first = true;
    static int prevRep = rep;
    static int prevProc = proc;
    if (first || encode || rep != prevRep || proc != prevProc)
    {
      first = false;
      prevRep = rep;
      prevProc = proc;
      switch (proc)
      {
      case Proc::Cpu:

        switch (rep) {
        case Rep::Aux:   image->encodeCpu( imageEncoderAux ); break;
        case Rep::Pivot: image->encodeCpu( imageEncoderPivot ); break;
        }

        glBindBuffer( GL_ARRAY_BUFFER, image->bufGpuGrid );
        glBufferSubData( GL_ARRAY_BUFFER, 0, image->gridSize.x * image->gridSize.y * NUM_CELL_COUNTERS * sizeof(int), image->ptrCpuGrid );
        glBindBuffer( GL_ARRAY_BUFFER, image->bufGpuStream );
        glBufferSubData( GL_ARRAY_BUFFER, 0, image->cpuStreamLen * sizeof(float), image->ptrCpuStream );
        break;

      case Proc::Gpu:

        switch (rep) {
        case Rep::Aux:   encodeImageAux( image ); break;
        case Rep::Pivot: encodeImagePivot( image ); break;
        } break;
      }
    }

    //Render image
    if (draw)
    {
      switch (view) {
      case View::RandomDirect:   renderImage1to1(); break;
      case View::RandomCylinder: renderImageCylinder(); break;
      }
    }
  }

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

void reportState ()
{
  std::cout << std::endl;

  switch (proc) {
  case Proc::Cpu: std::cout << "(F1) Using CPU encoding" << std::endl; break;
  case Proc::Gpu: std::cout << "(F1) Using GPU encoding" << std::endl; break;
  }

  switch (rep) {
  case Rep::Aux:   std::cout << "(F2) Using AUX representation" << std::endl; break;
  case Rep::Pivot: std::cout << "(F2) Using PIVOT representation" << std::endl; break;
  }

  std::cout << (encode ? "(F3) Encoding ON" : "(F3) Encoding OFF" ) << std::endl;
  std::cout << (draw ? "(F4) Drawing ON" : "(F4) Drawing OFF" ) << std::endl;

  switch (view) {
  case View::Classic:        std::cout << "(F5) View Classic 1:1" << std::endl; break;
  case View::RandomDirect:   std::cout << "(F5) View Random 1:1" << std::endl; break;
  case View::RandomCylinder: std::cout << "(F5) View Random Cylinder" << std::endl; break;
  }

  std::cout << std::endl;
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
    proc = (proc + 1) % Proc::Count;
    reportState();
  }
  else if (key == GLUT_KEY_F2)
  {
    rep = (rep + 1) % Rep::Count;
    reportState();
  }
  else if (key == GLUT_KEY_F3)
  {
    encode = !encode;
    reportState();
  }
  else if (key == GLUT_KEY_F4)
  {
    draw = !draw;
    reportState();
  }/*
  else if (key == GLUT_KEY_F4)
  {
    drawGrid = !drawGrid;
    std::cout << (drawGrid ? "Grid ON" : "Grid OFF" ) << std::endl;
  }*/
  else if (key == GLUT_KEY_F5)
  {
    view = (view + 1) % View::Count;
    reportState();
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

  if (view == View::RandomCylinder)
  {
    if (mouseButton == GLUT_LEFT_BUTTON)
    {
      angleX += -mouseDiff.x * 0.5f * (PI / 180);
      angleY += -mouseDiff.y * 0.5f * (PI / 180);
    }
    else if (mouseButton == GLUT_RIGHT_BUTTON)
    {
      zoomZ *= 1.0f + -mouseDiff.y / 100.0f;
    }
  }
  else
  {
    if (mouseButton == GLUT_LEFT_BUTTON)
    {
      panX += mouseDiff.x;
      panY += -mouseDiff.y;
    }
    else if (mouseButton == GLUT_RIGHT_BUTTON)
    {
      zoomS *= 1.0f + mouseDiff.y / 100.0f;
    }
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
  Shader::Define( "OBJCELL_COUNTER_WIND",   "1" );
  Shader::Define( "NUM_OBJCELL_COUNTERS",   "2" );

  Shader::Define( "CELL_COUNTER_PREV",      "0" );
  Shader::Define( "NUM_CELL_COUNTERS",      "1" );

  Shader::Define( "NODE_TYPE_LINE",         "1" );
  Shader::Define( "NODE_TYPE_QUAD",         "2" );
  Shader::Define( "NODE_TYPE_OBJECT",       "3" );

  shaderGrid = new Shader(
    "shaders/grid.vert.c",
    "shaders/grid.frag.c" );

  shaderEncodeAuxInit = new Shader(
    "shaders_aux/encode_aux_init.vert.c",
    "shaders_aux/encode_aux_init.frag.c" );

  shaderEncodeAuxInitObject = new Shader(
    "shaders_aux/encode_aux_initobject.vert.c",
    "shaders_aux/encode_aux_initobject.geom.c",
    "shaders_aux/encode_aux_initobject.frag.c" );

  shaderEncodeAuxLines = new Shader(
    "shaders_aux/encode_aux_lines.vert.c",
    "shaders_aux/encode_aux_lines.geom.c",
    "shaders_aux/encode_aux_lines.frag.c" );

  shaderEncodeAuxQuads = new Shader(
    "shaders_aux/encode_aux_quads.vert.c",
    "shaders_aux/encode_aux_quads.geom.c",
    "shaders_aux/encode_aux_quads.frag.c" );

  shaderEncodeAuxObject = new Shader(
    "shaders_aux/encode_aux_object.vert.c",
    "shaders_aux/encode_aux_object.frag.c" );

  shaderEncodeAuxSort = new Shader(
    "shaders_aux/encode_aux_sort.vert.c",
    "shaders_aux/encode_aux_sort.frag.c" );

  shaderRenderAux = new Shader(
    "shaders_aux/render_aux.vert.c",
    "shaders_aux/render_aux.frag.c" );

  shaderGrid->load();
  shaderEncodeAuxInit->load();
  shaderEncodeAuxInitObject->load();
  shaderEncodeAuxLines->load();
  shaderEncodeAuxQuads->load();
  shaderEncodeAuxObject->load();
  shaderEncodeAuxSort->load();
  shaderRenderAux->load();

  shaderEncodePivotInit = new Shader(
    "shaders_pivot/encode_pivot_init.vert.c",
    "shaders_pivot/encode_pivot_init.frag.c" );

  shaderEncodePivotInitObject = new Shader(
    "shaders_pivot/encode_pivot_initobject.vert.c",
    "shaders_pivot/encode_pivot_initobject.geom.c",
    "shaders_pivot/encode_pivot_initobject.frag.c" );

  shaderEncodePivotLines = new Shader(
    "shaders_pivot/encode_pivot_lines.vert.c",
    "shaders_pivot/encode_pivot_lines.geom.c",
    "shaders_pivot/encode_pivot_lines.frag.c" );

  shaderEncodePivotQuads = new Shader(
    "shaders_pivot/encode_pivot_quads.vert.c",
    "shaders_pivot/encode_pivot_quads.geom.c",
    "shaders_pivot/encode_pivot_quads.frag.c" );

  shaderEncodePivotObject = new Shader(
    "shaders_pivot/encode_pivot_object.vert.c",
    "shaders_pivot/encode_pivot_object.frag.c" );

  shaderEncodePivotSort = new Shader(
    "shaders_pivot/encode_pivot_sort.vert.c",
    "shaders_pivot/encode_pivot_sort.frag.c" );

  shaderRenderPivot = new Shader(
    "shaders_pivot/render_pivot.vert.c",
    "shaders_pivot/render_pivot.frag.c" );

  shaderEncodePivotInit->load();
  shaderEncodePivotInitObject->load();
  shaderEncodePivotLines->load();
  shaderEncodePivotQuads->load();
  shaderEncodePivotObject->load();
  shaderEncodePivotSort->load();
  shaderRenderPivot->load();


  shaderClassicQuads = new Shader(
    "shaders_classic/classic_quads.vert.c",
    "shaders_classic/classic_quads.frag.c" );

  shaderClassicContour = new Shader(
    "shaders_classic/classic_contour.vert.c",
    "shaders_classic/classic_contour.frag.c" );

  shaderClassic = new Shader(
    "shaders_classic/classic.vert.c",
    "shaders_classic/classic.frag.c" );

  shaderClassicQuads->load();
  shaderClassicContour->load();
  shaderClassic->load();

  imageEncoderAux = new ImageEncoderAux;
  imageEncoderPivot = new ImageEncoderPivot;

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
  //for (int i=12; i<13; ++i)
  {
    const float *style = styleArrays[i];
    if (! (((int)style[9]) & VG_FILL_PATH)) continue;

    Object *objCubic = new Object();
    processCommands( objCubic, i );

    Object *obj = objCubic->cubicsToQuads();
    delete objCubic;
    
    obj->color.set( style[4], style[5], style[6], style[7] );
    //if (obj->color == Vec4( 1,1,1,1 ))
      //obj->color = Vec4( 0,0,0,1 );

    image->objects.push_back( obj );
  }

  image->updateBounds();
  //image->updateGrid();
  image->encodeCpu( imageEncoderAux );
  image->updateBuffers();

  std::cout << "Total stream len: " << image->cpuStreamLen << std::endl;
  //std::cout << "Maximum cell len: " << maxCellLen << std::endl;
  
  reportState();
  glutMainLoop();
}
