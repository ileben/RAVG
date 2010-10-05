#include "rvgMain.h"

Object::Object()
{
  contour = NULL;
  penDown = false;
  pen.set( 0,0 );
  buffersInit = false;
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

void Object::updateBuffers()
{
  checkGlError( "updateBuffersGrid start" );

  if (!buffersInit)
  {
    glGenBuffers( 1, &bufLines );
    glGenBuffers( 1, &bufQuads );
    glGenBuffers( 1, &bufLinesQuads );

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
}

Image::Image()
{
  buffersInit = false;
  ptrCpuInfo = NULL;
  ptrCpuGrid = NULL;
  ptrCpuStream = NULL;
}

void Image::updateBounds (int gridResX, int gridResY)
{
  /////////////////////////////////////////////////////
  // Find bounds

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
  
  //////////////////////////////////////////////////////
  // Format grid

  gridSize.x = gridResX;
  gridSize.y = gridResY;
  gridOrigin.x = min.x;
  gridOrigin.y = min.y;
  cellSize.x = (max.x - min.x) / gridSize.x;
  cellSize.y = (max.y - min.y) / gridSize.y;

  /////////////////////////////////////////////////////
  // Init object info

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

    //Store object coords
    Obj obj;
    obj.min = vec3( object->min, (float) o );
    obj.max = vec3( object->max, (float) o );
    objs.push_back( obj );

    //Store object info
    ObjInfo objInfo;
    objInfo.gridOrigin = objGridOrigin;
    objInfo.gridSize = objGridSize;
    objInfo.gridOffset = objGridOffset;
    objInfo.color = object->color;
    objInfos.push_back( objInfo );
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
    glGenBuffers( 1, &bufObjs );
    glGenBuffers( 1, &bufObjInfos );

    glGenBuffers( 1, &bufGpuInfo );
    glGenBuffers( 1, &bufGpuGrid );
    glGenBuffers( 1, &bufGpuStream );

    buffersInit = true;
  }

  ///////////////////////////////////////
  // Object buffers

  glBindBuffer( GL_ARRAY_BUFFER, bufObjs );
  glBufferData( GL_ARRAY_BUFFER, objs.size() * sizeof(Obj), &objs[0], GL_STATIC_DRAW );
  
  checkGlError( "Image::updateBuffers objs" );

  glBindBuffer( GL_ARRAY_BUFFER, bufObjInfos );
  glBufferData( GL_ARRAY_BUFFER, objInfos.size() * sizeof(ObjInfo), &objInfos[0], GL_STATIC_DRAW );
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
  glBufferData( GL_ARRAY_BUFFER, MAX_STREAM_SIZE * sizeof(int), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuGrid );

  checkGlError( "Image::updateBuffers gpuobjGrid" );

  glBindBuffer( GL_ARRAY_BUFFER, bufGpuStream );
  glBufferData( GL_ARRAY_BUFFER, MAX_STREAM_SIZE * sizeof(float), 0, GL_STATIC_DRAW );
  glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
  glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &ptrGpuStream );

  checkGlError( "Image::updateBuffers gpuobjStream" );
}


void Image::encodeCpu (ImageEncoder *encoder)
{
  //Reset buffers
  if (ptrCpuInfo)        delete[] ptrCpuInfo;
  if (ptrCpuGrid)        delete[] ptrCpuGrid;
  if (ptrCpuStream)      delete[] ptrCpuStream;

  //Init new buffers
  ptrCpuInfo         = new int[ NUM_INFO_COUNTERS ];
  ptrCpuGrid         = new int[ MAX_STREAM_SIZE ];
  ptrCpuStream       = new float[ MAX_STREAM_SIZE ];

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

  for (int o=0; o<(int)objects.size(); ++o)
  {
    Object *obj = objects[o];
    encoder->objectId = o;
    encoder->objMin = obj->min;
    encoder->objMax = obj->max;
    encoder->encodeInitObject();
  }

  /////////////////////////////////////////////////////
  // Encode object lines

  for (int o=0; o<(int)objects.size(); ++o)
  {
    Object *obj = objects[o];

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

  for (int o=0; o<(int)objects.size(); ++o)
  {
    Object *obj = objects[o];

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

  for (int o=0; o<(int)objects.size(); ++o)
  {
    Object *obj = objects[o];
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
  
  encoder->getTotalStreamInfo( cpuTotalStreamLen );
  
  cpuMaxCellLen = 0;
  cpuMaxCellObjects = 0;
  cpuMaxCellSegments = 0;

  for (int x=0; x < gridSize.x; ++x) {
    for (int y=0; y < gridSize.y; ++y) {

      Uint32 cellLen = 0, cellObjects = 0, cellSegments = 0;
      encoder->getCellStreamInfo( x, y, cellLen, cellObjects, cellSegments );

      if (cellLen > cpuMaxCellLen) cpuMaxCellLen = cellLen;
      if (cellObjects > cpuMaxCellObjects) cpuMaxCellObjects = cellObjects;
      if (cellSegments > cpuMaxCellSegments) cpuMaxCellSegments = cellSegments;
    }}
}


void Image::encodeGpu (ImageEncoderGpu *encoder)
{ 
  glViewport( 0, 0, gridSize.x, gridSize.y );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_MULTISAMPLE );
  
  /////////////////////////////////////////////////////
  // Init size counters and main grid
  {
    Shader *shader = encoder->shaderInit;
    shader->use();

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, ptrGpuGrid );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, gridSize.x, gridSize.y );

    renderFullScreenQuad( shader );
  }

  /////////////////////////////////////////////////////
  // Init object grids
  {
    Shader *shader = encoder->shaderInitObject;
    shader->use();
    
    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, ptrObjInfos );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, ptrGpuGrid );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, gridOrigin.x, gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, gridSize.x, gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, cellSize.x, cellSize.y );

    Int32 aPos = shader->program->getAttribute( "in_pos" );
    glBindBuffer( GL_ARRAY_BUFFER, bufObjs );
    glVertexAttribPointer( aPos, 3, GL_FLOAT, false, sizeof(vec3), 0 );

    glEnableVertexAttribArray( aPos );
    glDrawArrays( GL_LINES, 0, objs.size() * 2 );
    glDisableVertexAttribArray( aPos );
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage init" );
  
  /////////////////////////////////////////////////////
  // Encode object lines
  {
    Shader *shader = encoder->shaderLines;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, ptrGpuStream );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, gridOrigin.x, gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, gridSize.x, gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, cellSize.x, cellSize.y );

    for (int o=0; o<(int)objects.size(); ++o)
    {
      Object *object = objects[o];

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
    Shader *shader = encoder->shaderQuads;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, ptrGpuStream );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, gridOrigin.x, gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, gridSize.x, gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, cellSize.x, cellSize.y );

    for (int o=0; o<(int)objects.size(); ++o)
    {
      Object *object = objects[o];

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
  // Encode object properties
  {
    Shader *shader = encoder->shaderObject;
    shader->use();

    Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
    glUniformui64( uPtrObjects, ptrObjInfos );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, ptrGpuInfo );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, ptrGpuStream );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, gridOrigin.x, gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, gridSize.x, gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, cellSize.x, cellSize.y );

    for (int o=0; o<(int)objects.size(); ++o)
    {
      Object *object = objects[o];
      ObjInfo &obj = objInfos[o];

      //
      Int32 uPtrObjGrid = shader->program->getUniform( "ptrObjGrid" );
      glUniformui64( uPtrObjGrid, ptrGpuGrid + obj.gridOffset * sizeof(int) );

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
      Vec2 min = Vec::Floor( (object->min - gridOrigin) / cellSize );
      Vec2 max = Vec::Ceil( (object->max - gridOrigin) / cellSize );

      //Transform to [-1,1] normalized coordinates (glViewport will transform back)
      min = (min / vec2( gridSize )) * 2.0f - Vec2(1.0f,1.0f);
      max = (max / vec2( gridSize )) * 2.0f - Vec2(1.0f,1.0f);

      renderQuad( shader, min, max );
    }
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage object" );

  /////////////////////////////////////////////////////
  // Sort objects in every cell back to front
  {
    Shader *shader = encoder->shaderSort;
    shader->use();

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, ptrGpuGrid );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, ptrGpuStream );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, gridSize.x, gridSize.y );

    renderFullScreenQuad( shader );
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage sort" );
}
