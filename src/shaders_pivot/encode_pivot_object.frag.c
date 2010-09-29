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

int addObject (int wind, vec4 color, int lastSegmentOffset, coherent int *ptrCell);

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
  coherent int *ptrObjGrid = ptrGrid + objGridOffset; */

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

  //Get winding counter
  int wind = ptrObjCell[ OBJCELL_COUNTER_WIND ];

  //Check if object has any segments or fully occludes this cell
  int prevOffset = ptrObjCell[ OBJCELL_COUNTER_PREV ];
  if (prevOffset >= 0 || (wind % 2) == 1)
  {
    //Add object data
    addObject( wind, color, prevOffset, ptrCell );
  }

  discard;
}

int addObject (int wind, vec4 color, int lastSegmentOffset, coherent int *ptrCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 9 );
  int prevOffset = atomicExchange( ptrCell + CELL_COUNTER_PREV, nodeOffset );
  coherent float *ptrNode = ptrStream + nodeOffset;

  //Store object data
  
  ptrNode[ 0 ] = (float) NODE_TYPE_OBJECT;
  ptrNode[ 1 ] = (float) prevOffset;
  ptrNode[ 2 ] = (float) objectId;
  ptrNode[ 3 ] = (float) wind;
  ptrNode[ 4 ] = color.x;
  ptrNode[ 5 ] = color.y;
  ptrNode[ 6 ] = color.z;
  ptrNode[ 7 ] = color.w;
  ptrNode[ 8 ] = (float) lastSegmentOffset;

  return nodeOffset;
}
