#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent int* ptrInfo;
uniform coherent int* ptrGrid;
uniform ivec2 gridSize;

in layout( pixel_center_integer ) vec4 gl_FragCoord;

void main()
{
  ptrInfo[ INFO_COUNTER_STREAMLEN ] = 0; //This should be done only once instead
  ptrInfo[ INFO_COUNTER_GRIDLEN ] = 0; //This should be done only once instead
  
  //Check grid coordinate in range
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { discard; return; }

  //Get grid cell pointer
  coherent int *ptrCell = ptrGrid
    + (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS;

  //Init grid cell data
  ptrCell[ CELL_COUNTER_PREV ] = -1;

  discard;
}
