#include "rvgMain.h"

Object::Object()
{
  color.set( 0, 0, 0, 1 );
  flat = false;
}

Object::~Object()
{
  clear();
}

void Object::clearFlat()
{
  lines.clear();
  cubics.clear();
  quads.clear();
  contours.clear();
  contourPoints.clear();
  flat = false;
}

void Object::clear()
{
  clearFlat();
  segments.clear();
  points.clear();
}

void Object::setId( const std::string &i )
{
  id = i;
}

void Object::setColor( float r, float g, float b, float a )
{
  color.set( r,g,b,a );
}

void Object::setTransform( const Matrix4x4 &t )
{
  transform = t;
}

void Object::moveTo( Float x, Float y, int space )
{
  segments.push_back( SegType::MoveTo | space );
  points.push_back( Vec2(x,y) );
  flat = false;
}

void Object::lineTo( Float x, Float y, int space )
{
  segments.push_back( SegType::LineTo | space );
  points.push_back( Vec2(x,y) );
  flat = false;
}

void Object::quadTo( Float x1, Float y1, Float x2, Float y2, int space )
{
  segments.push_back( SegType::QuadTo | space );
  points.push_back( Vec2(x1,y1) );
  points.push_back( Vec2(x2,y2) );
  flat = false;
}

void Object::cubicTo( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3, int space )
{
  segments.push_back( SegType::CubicTo | space );
  points.push_back( Vec2(x1,y1) );
  points.push_back( Vec2(x2,y2) );
  points.push_back( Vec2(x3,y3) );
  flat = false;
}

void Object::horizTo( Float x1, int space)
{
  segments.push_back( SegType::HorizTo | space );
  points.push_back( Vec2(x1,0) );
  flat = false;
}

void Object::vertTo (Float y1, int space)
{
  segments.push_back( SegType::VertTo | space );
  points.push_back( Vec2(0,y1) );
  flat = false;
}

void Object::close()
{
  segments.push_back( SegType::Close );
  flat = false;
}

void Object::process (ObjectProcessor &proc, int flags)
{
  int p = 0;

  proc.object = this;
  proc.pen.set( 0, 0 );
  proc.start.set( 0, 0 );

  proc.begin();

  for (Uint32 s=0; s<segments.size(); ++s)
  {
    //Get segment type and space
    int seg = segments[s];
    int type = seg & SegType::Mask;
    int space = seg & SegSpace::Mask;

    //If space relative,
    //current pen is base for next pen
    Vec2 penBase( 0,0 );
    if (space == SegSpace::Relative) {
      penBase = proc.pen;
    }

    //If space relative and absolute space requested,
    //current pen is base for output coordinates
    int coordSpace = space;
    Vec2 coordBase( 0,0 );
    if (space == SegSpace::Relative && (flags & ProcessFlags::Absolute)) {
      coordSpace = SegSpace::Absolute;
      coordBase = proc.pen;
    }

    //Invoke respective callback for segment type
    switch (type)
    {
    case SegType::MoveTo:{
      
      //Get coordinates
      vec2 &p1 = points[ p ];
      p += 1;

      //Invoke callback and update state
      proc.moveTo( coordBase + p1, coordSpace );
      proc.pen = penBase + p1;
      proc.start = proc.pen;
      break;}

    case SegType::LineTo:{

      //Get coordinates
      vec2 &p1 = points[ p ];
      p += 1;

      //Invoke callback and update state
      proc.lineTo( coordBase + p1, coordSpace );
      proc.pen = penBase + p1;
      break;}

    case SegType::QuadTo:{

      //Get coordinates
      vec2 &p1 = points[ p+0 ];
      vec2 &p2 = points[ p+1 ];
      p += 2;

      //Invoke callback and update state
      proc.quadTo( coordBase + p1, coordBase + p2, coordSpace );
      proc.pen = penBase + p2;
      break;}

    case SegType::CubicTo:{

      //Get coordinates
      vec2 &p1 = points[ p+0 ];
      vec2 &p2 = points[ p+1 ];
      vec2 &p3 = points[ p+2 ];
      p += 3;

      //Invoke callback and update state
      proc.cubicTo( coordBase + p1, coordBase + p2, coordBase + p3, coordSpace );
      proc.pen = penBase + p3;
      break;}

    case SegType::HorizTo:{

      //Get coordinates
      vec2 p1 = points[ p+0 ];
      p += 1;

      //Resolve implicit value
      p1.y = (space == SegSpace::Relative) ? 0 : proc.pen.y;

      //Invoke callback
      if (flags | ProcessFlags::Simplify)
        proc.lineTo( coordBase + p1, coordSpace );
      else proc.horizTo( coordBase.x + p1.x, coordSpace );

      //Update state
      proc.pen = penBase + p1;
      break;}

    case SegType::VertTo:{

      //Get coordinates
      vec2 p1 = points[ p+0 ];
      p += 1;

      //Resolve implicit values
      p1.x = (space == SegSpace::Relative) ? 0 : proc.pen.x;

      //Invoke callback
      if (flags | ProcessFlags::Simplify)
        proc.lineTo( coordBase + p1, coordSpace );
      else proc.horizTo( coordBase.x + p1.x, coordSpace );

      //Update state
      proc.pen = penBase + p1;
      break;}

    case SegType::Close:{

      //Invoke callback
      proc.close();
      proc.pen = proc.start;
      break;}
    }
  }

  proc.end();
}

//////////////////////////////////////////////////////////////////////
// Flatten object processor

Vec2 ObjectFlatten::transform (const Vec2 &p)
{
  return (getObject()->getTransform() * Vec4( p, 0, 1 )).xy();
}

void ObjectFlatten::begin ()
{
  penDown = false;
}

void ObjectFlatten::moveTo (const Vec2 &p1, int space)
{
  Contour contour;
  contour.length = 0;
  contour.start = getObject()->contourPoints.size();
  getObject()->contours.push_back( contour );

  Vec2 t0 = transform( p1 );

  getObject()->contourPoints.push_back( t0 );
  getObject()->contours.back().length++;

  penDown = true;
}

void ObjectFlatten::lineTo (const Vec2 &p1, int space)
{
  if (!penDown) return;

  Vec2 t0 = transform( getPen() );
  Vec2 t1 = transform( p1 );

  getObject()->lines.push_back( Line( t0.x,t0.y, t1.x,t1.y ));
  getObject()->contourPoints.push_back( t1 );
  getObject()->contours.back().length++;
}

void ObjectFlatten::quadTo (const Vec2 &p1, const Vec2 &p2, int space)
{
  if (!penDown) return;

  Vec2 t0 = transform( getPen() );
  Vec2 t1 = transform( p1 );
  Vec2 t2 = transform( p2 );

  getObject()->quads.push_back( Quad( t0.x,t0.y, t1.x,t1.y, t2.x,t2.y ));
  getObject()->contourPoints.push_back( t2 );
  getObject()->contours.back().length++;
}

void ObjectFlatten::cubicTo (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int space)
{
  if (!penDown) return;

  Vec2 t0 = transform( getPen() );
  Vec2 t1 = transform( p1 );
  Vec2 t2 = transform( p2 );
  Vec2 t3 = transform( p3 );

  getObject()->cubics.push_back( Cubic( t0.x,t0.y, t1.x,t1.y, t2.x,t2.y, t3.x,t3.y ));
  getObject()->contourPoints.push_back( t2 );
  getObject()->contours.back().length++;
}

void ObjectFlatten::close ()
{
  if (!penDown) return;

  Vec2 t0 = transform( getPen() );
  Vec2 t1 = transform( getStart() );

  getObject()->lines.push_back( Line( t0.x,t0.y, t1.x,t1.y ));
  penDown = false;
}

//////////////////////////////////////////////////////////////////////
// Clone object processor

void ObjectClone::begin ()
{
  clone = new Object();
  Vec4 c = getObject()->getColor();

  clone->setId( getObject()->getId() );
  clone->setColor( c.x, c.y, c.z, c.w );
  clone->setTransform( getObject()->getTransform() );
}

void ObjectClone::moveTo (const Vec2 &p1, int space)
{ clone->moveTo( p1.x, p1.y, space ); }

void ObjectClone::lineTo (const Vec2 &p1, int space)
{ clone->lineTo( p1.x, p1.y, space ); }

void ObjectClone::quadTo (const Vec2 &p1, const Vec2 &p2, int space)
{ clone->quadTo( p1.x, p1.y, p2.x, p2.y, space ); }

void ObjectClone::cubicTo (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int space)
{ clone->cubicTo( p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, space ); }

void ObjectClone::close ()
{ clone->close(); }

Object* ObjectClone::getClone()
{ return clone; }

//////////////////////////////////////////////////////////////////////
// Cubic-to-quad objet processor

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

void CubicsToQuads::cubicTo (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int space)
{
  Cubic cubic;
  cubic.p0 = getPen();
  cubic.p1 = p1;
  cubic.p2 = p2;
  cubic.p3 = p3;

  quads.clear();
  cubicToQuads( cubic, quads );
  for (Uint32 q=0; q<quads.size(); ++q)
  {
    Quad quad = quads[q];
    ObjectClone::quadTo( quad.p1, quad.p2, space );
  }
}

Object* Object::cubicsToQuads()
{
  CubicsToQuads proc;
  process( proc, ProcessFlags::Absolute | ProcessFlags::Simplify );
  return proc.getClone();
}

//////////////////////////////////////////////////////////////////////
// Update object functions

void Object::updateFlat()
{
  //if (flat) return;
  clearFlat();
  ObjectFlatten proc;
  process( proc, ProcessFlags::Absolute | ProcessFlags::Simplify);
  flat = true;
}

void Object::updateBounds()
{
  for (Uint32 p=0; p<contourPoints.size(); ++p)
  {
    Vec2 &point = contourPoints[p];
    if (p==0)
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

void Object::updateBuffers()
{
  //////////////////////////////////////
  //Contour points

  if (contourPoints.size() > 0)
  {
    bufContourPoints.upload( contourPoints.size() * sizeof( vec2 ), &contourPoints[0] );
    checkGlError( "Object::updateBuffers contours" );
  }

  //////////////////////////////////////
  //Line control points

  if (lines.size() > 0)
  {
    bufLines.upload( lines.size() * sizeof( Line ), &lines[0] );
    checkGlError( "Object::updateBuffers lines" );
  }

  //////////////////////////////////////
  //Quadratic control points
  if (quads.size() > 0)
  {
    bufQuads.upload( quads.size() * sizeof( Quad ), &quads[0] );
    checkGlError( "Object::updateBuffers quads" );
  }

  //////////////////////////////////////
  //Line + quad control points
/*
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
  */
}

Image::Image()
{
  ptrCpuInfo = NULL;
  ptrCpuGrid = NULL;
  ptrCpuStream = NULL;

  gridResX = 1;
  gridResY = 1;
}

void Image::setGridResolution (int x, int y)
{
  gridResX = x;
  gridResY = y;
}

void Image::addObject (Object *obj)
{
  objects.push_back( obj );
}

void Image::removeAllObjects ()
{
  objects.clear();
}

int Image::getNumObjects () {
  return objects.size();
}

Object* Image::getObject (int index) {
  return objects[ index ];
}

Object* Image::getObjectById (const std::string &id)
{
  for (Uint o=0; o<objects.size(); ++o)
    if (objects[o]->getId() == id)
      return objects[o];

  return NULL;
}

void Image::getObjectsBySubId (const std::string &id, std::vector<Object*> &list)
{
  for (Uint o=0; o<objects.size(); ++o)
    if (objects[o]->getId().find( id ) != std::string::npos)
      list.push_back( objects[o] );
}

void Image::updateFlat ()
{
  for (Uint32 o=0; o<objects.size(); ++o)
  {
    Object* obj = objects[o];
    obj->updateFlat();
  }
}

void Image::updateBounds ()
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
}

void Image::updateBuffers ()
{
  for (Uint32 o=0; o<objects.size(); ++o)
  {
    Object* obj = objects[o];
    obj->updateBuffers();
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
  // Object buffers

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

  if (objs.size() > 0)
  {
    bufObjs.upload( objs.size() * sizeof(Obj), &objs[0] );
    checkGlError( "Image::updateBuffers objs" );
  }

  if (objInfos.size() > 0)
  {
    bufObjInfos.upload( objInfos.size() * sizeof(ObjInfo), &objInfos[0] );
    checkGlError( "Image::updateBuffers gpuobjObjects" );
  }

  //////////////////////////////////////
  //Algorithm buffers

  bufGpuInfo.reserve( NUM_INFO_COUNTERS * sizeof(int) );
  checkGlError( "Image::updateBuffers gpuobjInfo" );

  bufGpuGrid.reserve( MAX_STREAM_SIZE * sizeof(int) );
  checkGlError( "Image::updateBuffers gpuobjGrid" );

  bufGpuStream.reserve( MAX_STREAM_SIZE * sizeof(float) );
  checkGlError( "Image::updateBuffers gpuobjStream" );
}

void Image::update ()
{
  //Prepare image data for rendering
  updateFlat();
  updateBounds();
  updateBuffers();
}

void Image::encodeCpu (EncoderCpu *encoder)
{
  //Prepare image data for encoding
  updateFlat();
  updateBounds();
  updateBuffers();

  //Reset cpu buffers
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
  
  /////////////////////////////////////////////////////
  // Upload data

  Uint32 streamLen = 0;
  encoder->getTotalStreamInfo( streamLen );
  bufGpuGrid.upload( gridSize.x * gridSize.y * NUM_CELL_COUNTERS * sizeof(int), ptrCpuGrid );
  bufGpuStream.upload( streamLen * sizeof(float), ptrCpuStream );
  checkGlError( "Image::encodeCpu upload" );
}


void Image::encodeGpu (EncoderGpu *encoder)
{
  //Prepare image data for encoding
  updateFlat();
  updateBounds();
  updateBuffers();

  glViewport( 0, 0, gridSize.x, gridSize.y );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_MULTISAMPLE );
  
  /////////////////////////////////////////////////////
  // Init size counters and main grid
  {
    Shader *shader = encoder->shaderInit;
    shader->use();

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, bufGpuInfo.getAddress() );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, bufGpuGrid.getAddress() );

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
    glUniformui64( uPtrObjects, bufObjInfos.getAddress() );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, bufGpuGrid.getAddress() );

    Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
    glUniform2f( uGridOrigin, gridOrigin.x, gridOrigin.y );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, gridSize.x, gridSize.y );

    Int32 uCellSize = shader->program->getUniform( "cellSize" );
    glUniform2f( uCellSize, cellSize.x, cellSize.y );

    Int32 aPos = shader->program->getAttribute( "in_pos" );
    glBindBuffer( GL_ARRAY_BUFFER, bufObjs.getId() );
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
    glUniformui64( uPtrObjects, bufObjInfos.getAddress() );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, bufGpuInfo.getAddress() );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, bufGpuGrid.getAddress() );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, bufGpuStream.getAddress() );

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
      glBindBuffer( GL_ARRAY_BUFFER, object->bufLines.getId() );
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
    glUniformui64( uPtrObjects, bufObjInfos.getAddress() );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, bufGpuInfo.getAddress() );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, bufGpuGrid.getAddress() );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, bufGpuStream.getAddress() );

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
      glBindBuffer( GL_ARRAY_BUFFER, object->bufQuads.getId() );
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
    glUniformui64( uPtrObjects, bufObjInfos.getAddress() );

    Int32 uPtrInfo = shader->program->getUniform( "ptrInfo" );
    glUniformui64( uPtrInfo, bufGpuInfo.getAddress() );

    Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
    glUniformui64( uPtrGrid, bufGpuGrid.getAddress() );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, bufGpuStream.getAddress() );

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
      glUniformui64( uPtrObjGrid, bufGpuGrid.getAddress() + obj.gridOffset * sizeof(int) );

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
    glUniformui64( uPtrGrid, bufGpuGrid.getAddress() );

    Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
    glUniformui64( uPtrStream, bufGpuStream.getAddress() );

    Int32 uGridSize = shader->program->getUniform( "gridSize" );
    glUniform2i( uGridSize, gridSize.x, gridSize.y );

    renderFullScreenQuad( shader );
  }

  glMemoryBarrier( GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV );
  checkGlError( "encodeImage sort" );
}

/*
void checkGridBuffers (Image *image)
{
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
}
*/

void Image::renderRandom (RendererRandom *renderer, VertexBuffer *buf, GLenum mode)
{
  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );
  
  //glEnable( GL_BLEND );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  Shader *shader = renderer->shader;
  shader->use();

  Int32 uModelview = shader->program->getUniform( "modelview" );
  glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

  Int32 uProjection = shader->program->getUniform( "projection" );
  glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

  Int32 uMatTexture = shader->program->getUniform( "matTexture" );
  glUniformMatrix4fv( uMatTexture, 1, false, (GLfloat*) matTexture.top().m );

  Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
  glUniformui64( uPtrGrid, bufGpuGrid.getAddress() );

  Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
  glUniformui64( uPtrStream, bufGpuStream.getAddress() );

  Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
  glUniformui64( uPtrObjects, bufObjInfos.getAddress() );

  Int32 uCellSize = shader->program->getUniform( "cellSize" );
  glUniform2f( uCellSize, cellSize.x, cellSize.y );

  Int32 uGridSize = shader->program->getUniform( "gridSize" );
  glUniform2i( uGridSize, gridSize.x, gridSize.y );

  Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
  glUniform2f( uGridOrigin, min.x, min.y );
  
  buf->render( shader, mode );

  //glDisable( GL_BLEND );
}

void Image::renderClassic (RendererClassic *renderer)
{
  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );

  glDisable( GL_DEPTH_TEST );
  glEnable( GL_STENCIL_TEST );

  for (Uint32 o=0; o<objects.size(); ++o)
  {
    Object *obj = objects[o];

    ///////////////////////////////////////////////
    //Render into stencil

    glStencilFunc( GL_ALWAYS, 0, 1 );
    glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

    ///////////////////////////////////////////////
    //Render quads
    {
      Shader *shader = renderer->shaderQuads;
      shader->use();
      
      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );
      
      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glEnableVertexAttribArray( aPos );
      glBindBuffer( GL_ARRAY_BUFFER, obj->bufQuads.getId() );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );
      
      glDrawArrays( GL_TRIANGLES, 0, obj->quads.size() * 3 );
      glDisableVertexAttribArray( aPos );
    }
    
    ///////////////////////////////////////////////
    //Render contours
    {
      Shader *shader = renderer->shaderContour;
      shader->use();
      
      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

      for (Uint32 c=0; c<obj->contours.size(); ++c)
      {
        Contour &cnt = obj->contours[c];

        Int32 aPos = shader->program->getAttribute( "in_pos" );
        glEnableVertexAttribArray( aPos );
        glBindBuffer( GL_ARRAY_BUFFER, obj->bufContourPoints.getId() );
        glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

        glDrawArrays( GL_TRIANGLE_FAN, cnt.start, cnt.length );
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
      Shader *shader = renderer->shader;
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
}
