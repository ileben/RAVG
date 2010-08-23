#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent int* ptrGrid;
uniform coherent float* ptrStream;
uniform ivec2 gridSize;

in layout( pixel_center_integer ) vec4 gl_FragCoord;

void swapObjects (coherent float *ptrObj1, coherent float *ptrObj2);

void main()
{
  //Check grid coordinate in range
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { discard; return; }

  //Get grid cell pointer
  coherent int *ptrCell = ptrGrid
    + (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS;

  //Loop from last to first object
  int objIndex1 = ptrCell[ CELL_COUNTER_PREV ];
  while (objIndex1 > -1)
  {
    //Object pointer
    coherent float *ptrObj1 = ptrStream + objIndex1;

    //Loop from prev to first object
    int objIndex2 = (int)ptrObj1[ 1 ];
    while (objIndex2 > -1)
    {
      //Object pointer
      coherent float *ptrObj2 = ptrStream + objIndex2;

      //Compare object ids
      if (ptrObj2[ 2 ] < ptrObj1[ 2 ])
        swapObjects( ptrObj1, ptrObj2 );

      //Move to prev object
      objIndex2 = (int)ptrObj2[ 1 ];
    }

    //Move to prev object
    objIndex1 = (int)ptrObj1[ 1 ];
  }

  discard;
}

void swapObjects (coherent float *ptrObj1, coherent float *ptrObj2)
{
  float tmp2 = ptrObj1 [2];
  float tmp3 = ptrObj1 [3];
  float tmp4 = ptrObj1 [4];
  float tmp5 = ptrObj1 [5];
  float tmp6 = ptrObj1 [6];
  float tmp7 = ptrObj1 [7];
  float tmp8 = ptrObj1 [8];

  ptrObj1 [2] = ptrObj2 [2];
  ptrObj1 [3] = ptrObj2 [3];
  ptrObj1 [4] = ptrObj2 [4];
  ptrObj1 [5] = ptrObj2 [5];
  ptrObj1 [6] = ptrObj2 [6];
  ptrObj1 [7] = ptrObj2 [7];
  ptrObj1 [8] = ptrObj2 [8];

  ptrObj2 [2] = tmp2;
  ptrObj2 [3] = tmp3;
  ptrObj2 [4] = tmp4;
  ptrObj2 [5] = tmp5;
  ptrObj2 [6] = tmp6;
  ptrObj2 [7] = tmp7;
  ptrObj2 [8] = tmp8;
}
