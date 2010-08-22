#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent int* ptrObjects;
uniform coherent int* ptrInfo;
uniform coherent int* ptrGrid;
uniform coherent float* ptrStream;

uniform coherent int* ptrObjGrid;
uniform ivec2 objGridOrigin;
uniform ivec2 objGridSize;

uniform vec2 gridOrigin;
uniform ivec2 gridSize;
uniform vec2 cellSize;
uniform int objectId;

uniform vec4 color;

in layout( pixel_center_integer ) vec4 gl_FragCoord;

int addLine (vec2 l0, vec2 l1, coherent int *ptrObjCell);
int addObject (vec4 color, int lastSegmentOffset, coherent int *ptrCell);

void main()
{
  //Check grid coordinate in range
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { discard; return; }
/*
  //Get object pointer and object grid info
  coherent int *ptrObj = ptrObjects + objectId * 5;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
  ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
  int   objGridOffset = ptrObj[4];
  
  //Get object grid pointer and coordinate
  coherent int *ptrObjGrid = ptrObjectGrids + objGridOffset; */

  //Check object grid coordinate in range
  ivec2 objGridCoord = gridCoord - objGridOrigin;
  if (objGridCoord.x < 0 || objGridCoord.x >= objGridSize.x ||
      objGridCoord.y < 0 || objGridCoord.y >= objGridSize.y)
  { discard; return; }

  //Get object grid cell pointer
  coherent int* ptrObjCell = ptrObjGrid
    + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

  //Get grid cell pointer
  coherent int *ptrCell = ptrGrid
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
      prevOffset = addLine( vec2( 1.0, 1.25 ), vec2( 1.0, -0.25 ), ptrObjCell );

      //If no other segments, mark the cell fully occluded
      //if (!anySegments) imageStore( counters, ivec3( gridCoord, 3 ), ivec4( 1 ) );
      anySegments = true;
    }
    
    //Add object data if any of its segments in this cell
    if (anySegments) addObject( color, prevOffset, ptrCell );
  }

  discard;
}

int addLine (vec2 l0, vec2 l1, coherent int *ptrObjCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 6 );
  int prevOffset = atomicExchange( ptrObjCell + OBJCELL_COUNTER_PREV, nodeOffset );
  coherent float *ptrNode = ptrStream + nodeOffset;

  //Store line data
  ptrNode[ 0 ] = (float) NODE_TYPE_LINE;
  ptrNode[ 1 ] = (float) prevOffset;
  ptrNode[ 2 ] = l0.x;
  ptrNode[ 3 ] = l0.y;
  ptrNode[ 4 ] = l1.x;
  ptrNode[ 5 ] = l1.y;

  return nodeOffset;
}

int addObject (vec4 color, int lastSegmentOffset, coherent int *ptrCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 8 );
  int prevOffset = atomicExchange( ptrCell + CELL_COUNTER_PREV, nodeOffset );
  coherent float *ptrNode = ptrStream + nodeOffset;

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
