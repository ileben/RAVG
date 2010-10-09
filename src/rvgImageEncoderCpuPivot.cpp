#include "rvgMain.h"

int EncoderCpuPivot::addLine (const vec2 &l0, const vec2 &l1, int *ptrObjCell)
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

int EncoderCpuPivot::addQuad (const Vec2 &q0, const Vec2 &q1, const Vec2 &q2, int *ptrObjCell)
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

int EncoderCpuPivot::addObject (int objectId, int wind, const Vec4 &color, int lastSegmentOffset, int *ptrCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 5 );
  int prevOffset = atomicExchange( ptrCell + CELL_COUNTER_PREV, nodeOffset );
  float *ptrNode = ptrStream + nodeOffset;

  //Store object data
  ptrNode[ 0 ] = (float) NODE_TYPE_OBJECT;
  ptrNode[ 1 ] = (float) prevOffset;
  ptrNode[ 2 ] = (float) objectId;
  ptrNode[ 3 ] = (float) wind;
  ptrNode[ 4 ] = (float) lastSegmentOffset;

  return nodeOffset;
}

static void swapObjects (float *ptrObj1, float *ptrObj2)
{
  float tmp2 = ptrObj1 [2];
  float tmp3 = ptrObj1 [3];
  float tmp4 = ptrObj1 [4];

  ptrObj1 [2] = ptrObj2 [2];
  ptrObj1 [3] = ptrObj2 [3];
  ptrObj1 [4] = ptrObj2 [4];

  ptrObj2 [2] = tmp2;
  ptrObj2 [3] = tmp3;
  ptrObj2 [4] = tmp4;
}

void EncoderCpuPivot::frag_encodeInit ()
{
  ptrInfo[ INFO_COUNTER_STREAMLEN ] = 0; //This should be done only once instead

  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
      return;

  //Get grid cell pointer
  int *ptrCell = ptrGrid
    + (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS;

  //Init grid cell data
  ptrCell[ CELL_COUNTER_PREV ] = -1;
}

void EncoderCpuPivot::frag_encodeInitObject ()
{
 //Check grid coordinate in range
 if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { return; }

  //Get object pointer and object grid info
  int  *ptrObj        = ptrObjects + objectId * NODE_SIZE_OBJINFO;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
  ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
  int   objGridOffset = ptrObj[4];

  //Get object grid pointer
  int *ptrObjGrid = ptrGrid + objGridOffset;

  //Check object grid coordinate in range
  ivec2 objGridCoord = gridCoord - objGridOrigin;
  if (objGridCoord.x < 0 || objGridCoord.x >= objGridSize.x ||
      objGridCoord.y < 0 || objGridCoord.y >= objGridSize.y)
  { return; }

  //Get object grid cell pointer
  int* ptrObjCell = ptrObjGrid
    + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

  //Init object grid cell data
  ptrObjCell[ OBJCELL_COUNTER_PREV ] = -1;
  ptrObjCell[ OBJCELL_COUNTER_WIND ] = 0;
}

void EncoderCpuPivot::frag_encodeLine ()
{
 //Check grid coordinate in range
 if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { return; }

  bool found = false, inside = false;
  float yy = 0.0, xx = 0.0;

  //Get object pointer and grid info
  int *ptrObj = ptrObjects + objectId * NODE_SIZE_OBJINFO;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
  ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
  int   objGridOffset = ptrObj[4];
  
  //Get object grid pointer
  int *ptrObjGrid = ptrGrid + objGridOffset;

  //Check object grid coordinate in range
  ivec2 objGridCoord = gridCoord - objGridOrigin;
  if (objGridCoord.x < 0 || objGridCoord.x >= objGridSize.x ||
      objGridCoord.y < 0 || objGridCoord.y >= objGridSize.y)
  { return; }

  //Get object grid cell pointer
  int* ptrObjCell = ptrObjGrid
    + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

  //Find cell origin
  vec2 cmin = gridOrigin + vec2(gridCoord) * cellSize;

  //Transform line coords into cell space
  vec2 l0 = (line0 - cmin) / cellSize;
  vec2 l1 = (line1 - cmin) / cellSize;

  //Check if line intersects horizontal ray from pivot towards +x
  lineIntersectionY( l0, l1, 0.5, found, xx );
  if (found && xx >= 0.5)
  {
    //Increase winding number
    atomicAdd( ptrObjCell + OBJCELL_COUNTER_WIND, 1 );
  }

  //Check if cell within line bounds
  if (gridCoord.x >= lineMin.x)
  {
    //Check if start point inside
    if (l0.x >= 0.0 && l0.x <= 1.0 && l0.y >= 0.0 && l0.y <= 1.0)
      inside = true;
    else
    {
      //Check if end point inside
      if (l1.x >= 0.0 && l1.x <= 1.0 && l1.y >= 0.0 && l1.y <= 1.0)
        inside = true;
      else
      {
        //Check if line intersects top edge
        lineIntersectionY( l0, l1, 0.0, found, xx );
        if (found && xx >= 0.0 && xx <= 1.0) inside = true;
        else
        {
          //Check if line intersects bottom edge
          lineIntersectionY( l0, l1, 1.0, found, xx );
          if (found && xx >= 0.0 && xx <= 1.0) inside = true;
          else
          {
            //Check if line intersects left edge
            lineIntersectionY( l0.yx(), l1.yx(), 0.0, found, yy );
            if (found && yy >= 0.0 && yy <= 1.0) inside = true;
            else
            {
              //Check if line intersects right edge
              lineIntersectionY( l0.yx(), l1.yx(), 1.0, found, yy );
              if (found && yy >= 0.0 && yy <= 1.0) inside = true;
            }
          }
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

void EncoderCpuPivot::frag_encodeQuad ()
{
  //Check grid coordinate in range
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { return; }

  bool found1=false, found2=false, inside=false;
  float yy1=0.0, yy2=0.0, xx1=0.0, xx2=0.0;

  //Get object pointer and object grid info
  int *ptrObj = ptrObjects + objectId * NODE_SIZE_OBJINFO;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
  ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
  int   objGridOffset = ptrObj[4];
  
  //Get object grid pointer
  int *ptrObjGrid = ptrGrid + objGridOffset;

  //Check object grid coordinate in range
  ivec2 objGridCoord = gridCoord - objGridOrigin;
  if (objGridCoord.x < 0 || objGridCoord.x >= objGridSize.x ||
      objGridCoord.y < 0 || objGridCoord.y >= objGridSize.y)
  { return; }

  //Get object grid cell pointer
  int* ptrObjCell = ptrObjGrid
    + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

  //Find cell origin
  vec2 cmin = gridOrigin + vec2(gridCoord) * cellSize;

  //Transform quad coords into cell space
  vec2 q0 = (quad0 - cmin) / cellSize;
  vec2 q1 = (quad1 - cmin) / cellSize;
  vec2 q2 = (quad2 - cmin) / cellSize;

  //Check if quad intersects horizontal ray from pivot towards +x
  quadIntersectionY( q0, q1, q2, 0.5, found1, found2, xx1, xx2 );
  if ((found1 && xx1 >= 0.5) != (found2 && xx2 >= 0.5))
  {
    //Increase winding number
    atomicAdd( ptrObjCell + OBJCELL_COUNTER_WIND, 1 );
  }

  //Check if cell within quad bounds
  if (gridCoord.x >= quadMin.x)
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
        quadIntersectionY( q0, q1, q2, 0.0, found1, found2, xx1, xx2 );
        if ((found1 && xx1 >= 0.0 && xx1 <= 1.0) || (found2 && xx2 >= 0.0 && xx2 <= 1.0))
          inside = true;
        else
        {
          //Check if quad intersects bottom edge
          quadIntersectionY( q0, q1, q2, 1.0, found1, found2, xx1, xx2 );
          if ((found1 && xx1 >= 0.0 && xx1 <= 1.0) || (found2 && xx2 >= 0.0 && xx2 <= 1.0))
            inside = true;
          else
          {
            //Check if quad intersects left edge
            quadIntersectionY( q0.yx(), q1.yx(), q2.yx(), 0.0, found1, found2, yy1, yy2 );
            if ((found1 && yy1 >= 0.0 && yy1 <= 1.0) || (found2 && yy2 >= 0.0 && yy2 <= 1.0))
              inside = true;
            else
            {
              //Check if quad intersects right edge
              quadIntersectionY( q0.yx(), q1.yx(), q2.yx(), 1.0, found1, found2, yy1, yy2 );
              if ((found1 && yy1 >= 0.0 && yy1 <= 1.0) || (found2 && yy2 >= 0.0 && yy2 <= 1.0))
                inside = true;
            }
          }
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

void EncoderCpuPivot::frag_encodeObject ()
{
  //Check grid coordinate in range
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { return; }

  //Get object pointer and object grid info
  int *ptrObj = ptrObjects + objectId * NODE_SIZE_OBJINFO;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
  ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
  int   objGridOffset = ptrObj[4];
  
  //Get object grid pointer and coordinate
  int *ptrObjGrid = ptrGrid + objGridOffset;

  //Check object grid coordinate in range
  ivec2 objGridCoord = gridCoord - objGridOrigin;
  if (objGridCoord.x < 0 || objGridCoord.x >= objGridSize.x ||
      objGridCoord.y < 0 || objGridCoord.y >= objGridSize.y)
  { return; }

  //Get object grid cell pointer
  int* ptrObjCell = ptrObjGrid
    + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

  //Get grid cell pointer
  int *ptrCell = ptrGrid
    + (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS;

  //Get winding counter
  int wind = ptrObjCell[ OBJCELL_COUNTER_WIND ];

  //Check if object has any segments or fully occludes this cell
  int prevOffset = ptrObjCell[ OBJCELL_COUNTER_PREV ];
  if (prevOffset >= 0 || (wind % 2) == 1)
  {
    //Add object data
    addObject( objectId, wind, color, prevOffset, ptrCell );
  }
}

void EncoderCpuPivot::frag_encodeSort ()
{
  //Check grid coordinate in range
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { return; }

  //Get grid cell pointer
  int *ptrCell = ptrGrid
    + (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS;
  
  //Loop from last to first object
  int objIndex1 = ptrCell[ CELL_COUNTER_PREV ];
  while (objIndex1 > -1)
  {
    //Object pointer
    float *ptrObj1 = ptrStream + objIndex1;

    //Loop from prev to first object
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

    //If object has no segments it must occlude the cell entirely
    //so discard the ones before it by relinking the indirection index
    if ((int)ptrObj1[ 4 ] == -1)
      ptrCell[ CELL_COUNTER_PREV ] = objIndex1;

    //Move to prev object
    objIndex1 = (int)ptrObj1[ 1 ];
  }
}

void EncoderCpuPivot::encodeInit ()
{
  //Walk the cells of the grid
  for (int x=0; x < gridSize.x; ++x) {
    for (int y=0; y < gridSize.y; ++y) {

      //Fragment shader
      gridCoord = ivec2(x,y);
      frag_encodeInit();
    }
  }
}

void EncoderCpuPivot::encodeInitObject ()
{
  //Transform object bounds into grid space
  ivec2 gridMin = (ivec2) Vec::Floor( (objMin - gridOrigin) / cellSize );
  ivec2 gridMax = (ivec2) Vec::Ceil( (objMax - gridOrigin) / cellSize );

  //Walk the cells within object bounds
  for (int x=gridMin.x; x<gridMax.x; ++x) {
    for (int y=gridMin.y; y<gridMax.y; ++y) {

      //Fragment shader
      gridCoord = ivec2(x,y);
      frag_encodeInitObject();
    }
  }
}

void EncoderCpuPivot::encodeLine ()
{
  //Get object pointer and grid info
  int *ptrObj = ptrObjects + objectId * NODE_SIZE_OBJINFO;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );

  //Find line bounds
  Vec2 lmin = Vec::Min( line0, line1 );
  Vec2 lmax = Vec::Max( line0, line1 );

  //Transform line bounds into grid space
  ivec2 gridMin = (ivec2) Vec::Floor( (lmin - gridOrigin) / cellSize );
  ivec2 gridMax = (ivec2) Vec::Ceil( (lmax - gridOrigin) / cellSize );
  lineMin = gridMin;

  //Extend left side to object boundary
  gridMin.x = objGridOrigin.x;

  //Walk the cells within line bounds
  for (int x=gridMin.x; x<gridMax.x; ++x) {
    for (int y=gridMin.y; y<gridMax.y; ++y) {

      //Fragment shader
      gridCoord = ivec2(x,y);
      frag_encodeLine();

    }//y
  }//x
}

void EncoderCpuPivot::encodeQuad ()
{
  //Get object pointer and grid info
  int *ptrObj = ptrObjects + objectId * NODE_SIZE_OBJINFO;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );

  //Find quad bounds
  vec2 qmin = Vec::Min( quad0, Vec::Min( quad1, quad2 ) );
  vec2 qmax = Vec::Max( quad0, Vec::Max( quad1, quad2 ) );

  //Transform quad bounds into grid space
  ivec2 gridMin = (ivec2) Vec::Floor( (qmin - gridOrigin) / cellSize );
  ivec2 gridMax = (ivec2) Vec::Ceil( (qmax - gridOrigin) / cellSize );
  quadMin = gridMin;

  //Extend left side to object boundary
  gridMin.x = objGridOrigin.x;

  //Walk the cells within quad bounds
  for (int x=gridMin.x; x<gridMax.x; ++x) {
    for (int y=gridMin.y; y<gridMax.y; ++y) {

      //Fragment shader
      gridCoord = ivec2(x,y);
      frag_encodeQuad();

    }//y
  }//x
}

void EncoderCpuPivot::encodeObject ()
{
  //Transform object bounds into grid space
  ivec2 gridMin = (ivec2) Vec::Floor( (objMin - gridOrigin) / cellSize );
  ivec2 gridMax = (ivec2) Vec::Ceil( (objMax - gridOrigin) / cellSize );

  //Walk the cells within object bounds
  for (int x=gridMin.x; x<gridMax.x; ++x) {
    for (int y=gridMin.y; y<gridMax.y; ++y) {

      //Fragment shader
      gridCoord = ivec2(x,y);
      frag_encodeObject();

    }//y
  }//x
}

void EncoderCpuPivot::encodeSort ()
{
  //Walk the cells of the grid
  for (int x=0; x < gridSize.x; ++x) {
    for (int y=0; y < gridSize.y; ++y) {

      //Fragment shader
      gridCoord = ivec2(x,y);
      frag_encodeSort();
    }
  }
}

void EncoderCpuPivot::getTotalStreamInfo (Uint32 &length)
{
  length = ptrInfo[ INFO_COUNTER_STREAMLEN ];
}

void EncoderCpuPivot::getCellStreamInfo (int x, int y, Uint32 &length, Uint32 &objects, Uint32 &segments)
{
  length = 0;
  objects = 0;
  segments = 0;

  //Loop until end of object list
  int objIndex = ptrGrid[ (y * gridSize.x + x) * NUM_CELL_COUNTERS ];
  while (objIndex != -1)
  {
    //Sanity check
    //if (++safetyCounter >= 10000) break;

    //Get object type and link to previous object
    float *ptrObj = ptrStream + objIndex;
    int objType = (int) ptrObj[0];
    objIndex = (int) ptrObj[1];

    //Check object type
    if (objType == NODE_TYPE_OBJECT)
    {
      length += 5;
      objects += 1;

      //Get color of the object
      int objId = (int) ptrObj[2];

      //Loop until end of segment list
      int segIndex = (int) ptrObj[4];
      while (segIndex != -1)
      {
        //Get segment type and link to previous segment
        float *ptrSeg = ptrStream + segIndex;
        int segType = (int) ptrSeg[0];
        segIndex = (int) ptrSeg[1];

        //Check segment type
        if (segType == NODE_TYPE_LINE)
        {
          length += 6;
          segments += 1;
        }
        else if (segType == NODE_TYPE_QUAD)
        {
          length += 8;
          segments += 1;
        }
        else break;
      }
    }
    else break;
  }
}
