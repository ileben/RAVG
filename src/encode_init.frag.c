#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent float* ptrCellStreams;
uniform coherent layout( size1x32 ) image2DArray imgCellStreams;
uniform coherent layout( size1x32 ) iimage2DArray imgCellCounters;
uniform ivec2 gridSize;

in layout( pixel_center_integer ) vec4 gl_FragCoord;

void main()
{
  ivec2 gridCoord = ivec2( gl_FragCoord );
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    imageStore( imgCellCounters, ivec3( gridCoord, 0 ), ivec4( 0 ) );
    imageStore( imgCellCounters, ivec3( gridCoord, 1 ), ivec4( -1 ) );
    imageStore( imgCellCounters, ivec3( gridCoord, 2 ), ivec4( 0 ) );
    imageStore( imgCellCounters, ivec3( gridCoord, 3 ), ivec4( 0 ) );
  }

  discard;
}
