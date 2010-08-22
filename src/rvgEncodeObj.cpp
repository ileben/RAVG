#if (1)
#include "rvgMain.h"

int Image::atomicAdd (int *ptr, int value)
{
  int oldValue = *ptr;
  *ptr += value;
  return oldValue;
}

int Image::atomicExchange (int *ptr, int value)
{
  int oldValue = *ptr;
  *ptr = value;
  return oldValue;
}

int Image::addLine (const vec2 &l0, const vec2 &l1, int *ptrObjCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 6 );
  int prevOffset = atomicExchange( ptrObjCell + OBJCELL_COUNTER_PREV, nodeOffset );
  float *ptrNode = ptrStream + nodeOffset;

  //Store line data
  ptrNode[ 0 ] = (float) NODE_TYPE_LINE;
  ptrNode[ 1 ] = (float) prevOffset;
  ptrNode[ 2 ] = l0.x;
  ptrNode[ 3 ] = l0.y;
  ptrNode[ 4 ] = l1.x;
  ptrNode[ 5 ] = l1.y;

  return nodeOffset;
}

int Image::addQuad (const Vec2 &q0, const Vec2 &q1, const Vec2 &q2, int *ptrObjCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 8 );
  int prevOffset = atomicExchange( ptrObjCell + OBJCELL_COUNTER_PREV, nodeOffset );
  float *ptrNode = ptrStream + nodeOffset;

  //Store quad data
  ptrNode[ 0 ] = (float) NODE_TYPE_QUAD;
  ptrNode[ 1 ] = (float) prevOffset;
  ptrNode[ 2 ] = q0.x;
  ptrNode[ 3 ] = q0.y;
  ptrNode[ 4 ] = q1.x;
  ptrNode[ 5 ] = q1.y;
  ptrNode[ 6 ] = q2.x;
  ptrNode[ 7 ] = q2.y;

  return nodeOffset;
}

int Image::addObject (const Vec4 &color, int lastSegmentOffset, int *ptrCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 8 );
  int prevOffset = atomicExchange( ptrCell + CELL_COUNTER_PREV, nodeOffset );
  float *ptrNode = ptrStream + nodeOffset;

  //Store object data
  ptrNode[ 0 ] = (float) NODE_TYPE_OBJECT;
  ptrNode[ 1 ] = (float) prevOffset;
  ptrNode[ 2 ] = (float) objectId;
  ptrNode[ 3 ] = color.x;
  ptrNode[ 4 ] = color.y;
  ptrNode[ 5 ] = color.z;
  ptrNode[ 6 ] = color.w;
  ptrNode[ 7 ] = (float) lastSegmentOffset;

  return nodeOffset;
}

void swapObjects (float *ptrObj1, float *ptrObj2)
{
  float tmp2 = ptrObj1 [2];
  float tmp3 = ptrObj1 [3];
  float tmp4 = ptrObj1 [4];
  float tmp5 = ptrObj1 [5];
  float tmp6 = ptrObj1 [6];
  float tmp7 = ptrObj1 [7];

  ptrObj1 [2] = ptrObj2 [2];
  ptrObj1 [3] = ptrObj2 [3];
  ptrObj1 [4] = ptrObj2 [4];
  ptrObj1 [5] = ptrObj2 [5];
  ptrObj1 [6] = ptrObj2 [6];
  ptrObj1 [7] = ptrObj2 [7];

  ptrObj2 [2] = tmp2;
  ptrObj2 [3] = tmp3;
  ptrObj2 [4] = tmp4;
  ptrObj2 [5] = tmp5;
  ptrObj2 [6] = tmp6;
  ptrObj2 [7] = tmp7;
}

void Image::frag_encodeInit
(
 const ivec2 &gridCoord
)
{
  ptrInfo[ INFO_COUNTER_STREAMLEN ] = 0; //This should be done only once instead
  ptrInfo[ INFO_COUNTER_GRIDLEN ] = 0; //This should be done only once instead

  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    //Get grid cell pointer
    int *ptrCell = ptrGrid
      + (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS;

    //Init grid cell data
    ptrCell[ CELL_COUNTER_PREV ] = -1;
  }
}

void Image::geom_encodeInitObject
(
 vec2 min,
 vec2 max,
 ivec2 &objGridOrigin,
 ivec2 &objGridSize,
 int &objGridOffset
)
{
  //Transform object bounds into grid space
  ivec2 gridMin = Vec::Floor( (min - gridOrigin) / cellSize ).toInt();
  ivec2 gridMax = Vec::Ceil( (max - gridOrigin) / cellSize ).toInt();

  objGridOrigin = gridMin;
  objGridSize = gridMax - gridMin;

  //Get object offset, store new grid stream size
  objGridOffset = atomicAdd( ptrInfo + INFO_COUNTER_GRIDLEN,
    objGridSize.x * objGridSize.y * NUM_OBJCELL_COUNTERS );

  //Store object data
  int *ptrObj = ptrObjects + objectId * 5;
  ptrObj[ 0 ] = objGridOrigin.x;
  ptrObj[ 1 ] = objGridOrigin.y;
  ptrObj[ 2 ] = objGridSize.x;
  ptrObj[ 3 ] = objGridSize.y;
  ptrObj[ 4 ] = objGridOffset;
}

void Image::frag_encodeInitObject
(
 const ivec2 &gridCoord,
 ivec2 objGridOrigin,
 ivec2 objGridSize,
 int objGridOffset
)
{
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    //Get object grid pointer
    int *ptrObjGrid = ptrObjectGrids + objGridOffset;
    ivec2 objGridCoord = gridCoord - objGridOrigin;

    //Get object grid cell pointer
    int* ptrObjCell = ptrObjGrid
      + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

    //Init object grid cell data
    ptrObjCell[ OBJCELL_COUNTER_PREV ] = -1;
    ptrObjCell[ OBJCELL_COUNTER_AUX ] = 0;
  }
}

void Image::frag_encodeLine
(
 const ivec2 &gridCoord,
 const vec2 &line0,
 const vec2 &line1
 )
{
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    bool found = false, inside = false;
    float yy = 0.0f, xx = 0.0f;

    //Get object pointer and object grid info
    int  *ptrObj        = ptrObjects + objectId * 5;
    ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
    ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
    int   objGridOffset = ptrObj[4];
    
    //Get object grid pointer and coordinate
    int  *ptrObjGrid   = ptrObjectGrids + objGridOffset;
    ivec2 objGridCoord = gridCoord - objGridOrigin;

    //Get object grid cell pointer
    int* ptrObjCell = ptrObjGrid
      + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

    //Find cell origin
    Vec2 cmin = Vec2(
      gridOrigin.x + gridCoord.x * cellSize.x,
      gridOrigin.y + gridCoord.y * cellSize.y );

    //Transform line coords into cell space
    Vec2 l0 = (line0 - cmin) / cellSize;
    Vec2 l1 = (line1 - cmin) / cellSize;

    //Check if line intersects bottom edge
    lineIntersectionY( l0, l1, 1.0f, 0.0f, 1.0f, found, xx );
    if (found)
    {
      //Walk all the cells to the left
      for (int x = (int)objGridCoord.x - 1; x >= 0; --x)
      {
        //Get object grid cell pointer
        int *ptrObjCellAux = ptrObjGrid
          + (objGridCoord.y * objGridSize.x + x) * NUM_OBJCELL_COUNTERS;

        //Increase auxiliary vertical counter
        atomicAdd( ptrObjCellAux + OBJCELL_COUNTER_AUX, 1 );
      }
      inside = true;
    }

    //Skip writing into this cell if fully occluded by another object
    //int cellDone = imageLoad( counters, ivec3( gridCoord, COUNTER_OCCLUSION ) ).x;
    //if (cellDone == 0)
    {
      //Check if line intersects right edge
      lineIntersectionY( l0.yx(), l1.yx(), 1.0f, 0.0f, 1.0f, found, yy );
      if (found)
      {
        //Add line spanning from intersection point to upper-right corner
        addLine( Vec2( 1.0f, yy ), Vec2( 1.0f, -0.25f ), ptrObjCell );
        inside = true;
      }
      
      //Check for additional conditions if these two failed
      if (!inside)
      {
        //Check if start point inside
        if (l0.x >= 0.0f && l0.x <= 1.0f && l0.y >= 0.0f && l0.y <= 1.0f)
          inside = true;
        else
        {
          //Check if end point inside
          if (l1.x >= 0.0f && l1.x <= 1.0f && l1.y >= 0.0f && l1.y <= 1.0f)
            inside = true;
          else
          {
            //Check if line intersects top edge
            lineIntersectionY( l0, l1, 0.0f, 0.0f, 1.0f, found, xx );
            if (found) inside = true;
            else
            {
              //Check if line intersects left edge
              lineIntersectionY( l0.yx(), l1.yx(), 0.0f, 0.0f, 1.0f, found, yy );
              if (found) inside = true;
            }
          }
        }
      }

      if (inside)
      {
        //Add line data to stream
        addLine( l0,l1, ptrObjCell );
      }
    }
  }
}

void Image::frag_encodeQuad
(
  const ivec2 &gridCoord,
  const Vec2 &quad0,
  const Vec2 &quad1,
  const Vec2 &quad2
)
{
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    bool found1=false, found2=false, inside=false;
    float yy1=0.0, yy2=0.0, xx1=0.0, xx2=0.0;

    //Get object pointer and object grid info
    int  *ptrObj        = ptrObjects + objectId * 5;
    ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
    ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
    int   objGridOffset = ptrObj[4];
    
    //Get object grid pointer and coordinate
    int  *ptrObjGrid   = ptrObjectGrids + objGridOffset;
    ivec2 objGridCoord = gridCoord - objGridOrigin;

    //Get object grid cell pointer
    int* ptrObjCell = ptrObjGrid
      + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

    //Find cell origin
    vec2 cmin = vec2(
      gridOrigin.x + gridCoord.x * cellSize.x,
      gridOrigin.y + gridCoord.y * cellSize.y );

    //Transform quad coords into cell space
    vec2 q0 = (quad0 - cmin) / cellSize;
    vec2 q1 = (quad1 - cmin) / cellSize;
    vec2 q2 = (quad2 - cmin) / cellSize;

    //Check if quad intersects bottom edge
    quadIntersectionY( q0, q1, q2, 1.0, 0.0, 1.0, found1, found2, xx1, xx2 );
    if (found1 != found2)
    {
      //Walk all the cells to the left
      for (int x = (int)objGridCoord.x - 1; x >= 0; --x)
      {
        //Get object grid cell pointer
        int *ptrObjCellAux = ptrObjGrid
          + (objGridCoord.y * objGridSize.x + x) * NUM_OBJCELL_COUNTERS;

        //Increase auxiliary vertical counter
        atomicAdd( ptrObjCellAux + OBJCELL_COUNTER_AUX, 1 );
      }
    }
    if (found1 || found2) inside = true;

    //Skip writing into this cell if fully occluded by another object
    //int cellDone = imageLoad( counters, ivec3( gridCoord, COUNTER_OCCLUSION ) ).x;
    //if (cellDone == 0)
    {
      //Check if quad intersects right edge
      quadIntersectionY( q0.yx(), q1.yx(), q2.yx(), 1.0, 0.0, 1.0, found1, found2, yy1, yy2 );
      if (found1)
      {
        //Add line spanning from intersection point to upper-right corner
        addLine( vec2( 1.0, yy1 ), vec2( 1.0, -0.25 ), ptrObjCell );
        inside = true;
      }
      if (found2)
      {
        //Add line spanning from intersection point to upper-right corner
        addLine( vec2( 1.0, yy2 ), vec2( 1.0, -0.25 ), ptrObjCell );
        inside = true;
      }

      //Check for additional conditions if these two failed
      if (!inside)
      {
        //Check if start point inside
        if (q0.x >= 0.0 && q0.x <= 1.0 && q0.y >= 0.0 && q0.y <= 1.0)
          inside = true;
        else
        {
          //Check if end point inside
          if (q2.x >= 0.0 && q2.x <= 1.0 && q2.y >= 0.0 && q2.y <= 1.0)
            inside = true;
          else
          {
            //Check if quad intersects top edge
            quadIntersectionY( q0, q1, q2, 0.0, 0.0, 1.0, found1, found2, xx1, xx2 );
            if (found1 || found2) inside = true;
            else
            {
              //Check if quad intersects left edge
              quadIntersectionY( q0.yx(), q1.yx(), q2.yx(), 0.0, 0.0, 1.0, found1, found2, yy1, yy2 );
              if (found1 || found2) inside = true;
            }
          }
        }
      }

      if (inside)
      {
        //Add quad data to stream
        addQuad( q0,q1,q2, ptrObjCell );
      }
    }
  }
}

void Image::frag_encodeObject
(
  const ivec2 &gridCoord,
  const Vec4 &color
)
{
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    //Get object pointer and object grid info
    int  *ptrObj        = ptrObjects + objectId * 5;
    ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
    ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
    int   objGridOffset = ptrObj[4];
    
    //Get object grid pointer and coordinate
    int  *ptrObjGrid   = ptrObjectGrids + objGridOffset;
    ivec2 objGridCoord = gridCoord - objGridOrigin;

    //Get object grid cell pointer
    int* ptrObjCell = ptrObjGrid
      + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

    //Get grid cell pointer
    int *ptrCell = ptrGrid
      + (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS;

    //Get and reset auxiliary vertical counter
    int aux = atomicExchange( ptrObjCell + OBJCELL_COUNTER_AUX, 0 );

    //Skip writing into this cell if fully occluded by another object
    //int cellDone = imageLoad( counters, ivec3( gridCoord, COUNTER_OCCLUSION ) ).x;
    //if (cellDone == 0)
    {
      bool anySegments = false;

      //Check if object has any segments in this cell
      int prevOffset = ptrObjCell[ OBJCELL_COUNTER_PREV ];
      if (prevOffset >= 0) anySegments = true;

      //Check if auxiliary counter parity is odd
      if (aux % 2 == 1)
      {
        //Add auxiliary vertical segment
        prevOffset = addLine( vec2( 1.0, 1.25f ), vec2( 1.0, -0.25f ), ptrObjCell );

        //If no other segments, mark the cell fully occluded
        //if (!anySegments) imageStore( counters, ivec3( gridCoord, 3 ), ivec4( 1 ) );
        anySegments = true;
      }
      
      //Add object data if any of its segments in this cell
      if (anySegments) addObject( color, prevOffset, ptrCell );
    }
  }
}

void Image::frag_encodeSort
(
 const ivec2 &gridCoord
)
{
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    //Get grid cell pointer
    int *ptrCell = ptrGrid
      + (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS;

    //Loop from first to last object
    int objIndex1 = ptrCell[ CELL_COUNTER_PREV ];
    while (objIndex1 > -1)
    {
      //Object pointer
      float *ptrObj1 = ptrStream + objIndex1;

      //Loop from prev to last object
      int objIndex2 = (int)ptrObj1[ 1 ];
      while (objIndex2 > -1)
      {
        //Object pointer
        float *ptrObj2 = ptrStream + objIndex2;

        //Compare object ids
        if (ptrObj2[ 2 ] < ptrObj1[ 2 ])
          swapObjects( ptrObj1, ptrObj2 );

        //Move to prev object
        objIndex2 = (int)ptrObj2[ 1 ];
      }

      //Move to prev object
      objIndex1 = (int)ptrObj1[ 1 ];
    }
  }
}

void Image::updateStream ()
{
  //Reset buffers and stream
  if (ptrInfo)        delete[] ptrInfo;
  if (ptrObjects)     delete[] ptrObjects;
  if (ptrObjectGrids) delete[] ptrObjectGrids;
  if (ptrGrid)        delete[] ptrGrid;
  if (ptrStream)      delete[] ptrStream;

  ptrInfo         = new int[ NUM_INFO_COUNTERS ];
  ptrObjects      = new int[ objects.size() * 5 ];
  ptrObjectGrids  = new int[ 500000 ];
  ptrGrid         = new int[ gridSize.x * gridSize.y * NUM_CELL_COUNTERS ];
  ptrStream       = new float[ 500000 ];

  int maxCellLen = 0;

  //Walk the cells of the grid
  for (int x=0; x < gridSize.x; ++x) {
    for (int y=0; y < gridSize.y; ++y) {

      //Fragment shader
      frag_encodeInit( ivec2(x,y) );
    }
  }

  //Sync////////////////////////////////

  //Walk the list of objects
  for (int o=0; o<(int)objects.size(); ++o)
  {
    Object *obj = objects[o];

    //Uniforms
    //pointers,
    //grid,
    objectId = o;

    //Geometry shader output
    ivec2 objGridOrigin;
    ivec2 objGridSize;
    int objGridOffset;

    //Geometry shader
    geom_encodeInitObject( obj->min, obj->max, objGridOrigin, objGridSize, objGridOffset );

    //Walk the cells within object bounds
    for (int x = objGridOrigin.x; x < objGridOrigin.x + objGridSize.x; ++x) {
      for (int y = objGridOrigin.y; y < objGridOrigin.y + objGridSize.y; ++y) {

        //Fragment shader
        frag_encodeInitObject( ivec2(x,y), objGridOrigin, objGridSize, objGridOffset );
      }
    }
  }

  //Sync////////////////////////////////

  //Walk the list of objects
  for (int o=0; o<(int)objects.size(); ++o)
  {
    Object *obj = objects[o];


    //Uniforms
    //pointers,
    //grid,
    objectId = o;

    //Walk the list of line segments
    for (Uint32 l=0; l<obj->lines.size(); ++l)
    {
      //Find line bounds
      Line &line = obj->lines[l];
      Vec2 lmin = Vec::Min( line.p0, line.p1 );
      Vec2 lmax = Vec::Max( line.p0, line.p1 );

      //Transform line bounds into grid space
      ivec2 ilmin = Vec::Floor( (lmin - gridOrigin) / cellSize ).toInt();
      ivec2 ilmax = Vec::Ceil( (lmax - gridOrigin) / cellSize ).toInt();

      //Walk the cells within line bounds
      for (int x=ilmin.x; x<ilmax.x; ++x) {
        for (int y=ilmin.y; y<ilmax.y; ++y) {

          //Fragment shader
          frag_encodeLine( ivec2(x,y), line.p0, line.p1 );

        }//y
      }//x
    }//lines


    //Uniforms
    //pointers,
    //grid,
    objectId = o;

    //Walk the list of quad segments
    for (Uint32 q=0; q < obj->quads.size(); ++q)
    {
      //Find quad bounds
      Quad &quad= obj->quads[q];
      vec2 qmin = Vec::Min( quad.p0, Vec::Min( quad.p1, quad.p2 ) );
      vec2 qmax = Vec::Max( quad.p0, Vec::Max( quad.p1, quad.p2 ) );

      //Transform quad bounds into grid space
      ivec2 iqmin = Vec::Floor( (qmin - gridOrigin) / cellSize ).toInt();
      ivec2 iqmax = Vec::Ceil( (qmax - gridOrigin) / cellSize ).toInt();

      //Walk the cells within quad bounds
      for (int x=iqmin.x; x<iqmax.x; ++x) {
        for (int y=iqmin.y; y<iqmax.y; ++y) {

          //Fragment shader
          frag_encodeQuad( ivec2(x,y), quad.p0, quad.p1, quad.p2 );

        }//y
      }//x
    }//quads

  }//objects

  //Sync////////////////////////////////

  //Walk the list of objects
  for (int o=0; o<(int)objects.size(); ++o)
  {
    Object *obj = objects[o];

    //Uniforms
    //pointers,
    //grid,
    objectId = o;

    //Transform object bounds into grid space
    ivec2 iomin = Vec::Floor( (obj->min - gridOrigin) / cellSize ).toInt();
    ivec2 iomax = Vec::Ceil( (obj->max - gridOrigin) / cellSize ).toInt();

    //Walk the cells within object bounds
    for (int x=iomin.x; x<iomax.x; ++x) {
      for (int y=iomin.y; y<iomax.y; ++y) {

        //Fragment shader
        frag_encodeObject( ivec2(x,y), obj->color );

      }//y
    }//x
  }//objects

  //Sync////////////////////////////////

  //Walk the cells of the grid
  for (int x=0; x < gridSize.x; ++x) {
    for (int y=0; y < gridSize.y; ++y) {

      //Fragment shader
      frag_encodeSort( ivec2(x,y) );
    }
  }

  int cpuGridLen = ptrInfo[ INFO_COUNTER_GRIDLEN ];
  cpuStreamLen = ptrInfo[ INFO_COUNTER_STREAMLEN ];
}

#endif
