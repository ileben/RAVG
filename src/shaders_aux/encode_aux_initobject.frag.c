#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform int* ptrObjects;
coherent uniform int* ptrGrid;
uniform ivec2 gridSize;

flat in int objectId;

in layout( pixel_center_integer ) vec4 gl_FragCoord;

void main()
{
  //Check grid coordinate in range
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { discard; return; }

  //Get object pointer and grid info
  int *ptrObj = ptrObjects + objectId * NODE_SIZE_OBJINFO;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
  ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
  int   objGridOffset = ptrObj[4];
  
  //Get object grid pointer
  coherent int *ptrObjGrid = ptrGrid + objGridOffset;

  //Check object grid coordinate in range
  ivec2 objGridCoord = gridCoord - objGridOrigin;
  if (objGridCoord.x < 0 || objGridCoord.x >= objGridSize.x ||
      objGridCoord.y < 0 || objGridCoord.y >= objGridSize.y)
  { discard; return; }

  //Get object grid cell pointer
  coherent int* ptrObjCell = ptrObjGrid
    + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

  //Init object grid cell data
  ptrObjCell[ OBJCELL_COUNTER_PREV ] = -1;
  ptrObjCell[ OBJCELL_COUNTER_AUX ] = 0;

  discard;
}
