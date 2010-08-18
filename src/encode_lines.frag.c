#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent float* ptrCellStreams;
uniform coherent layout( size1x32 ) image2DArray imgCellStreams;
uniform coherent layout( size1x32 ) iimage2DArray imgCellCounters;
uniform ivec2 gridSize;

in layout( pixel_center_integer ) vec4 gl_FragCoord;
in vec2 line0;
in vec2 line1;

void main()
{
  ivec2 cellCoord = ivec2( gl_FragCoord.xy );
  if (cellCoord.x >= 0 && cellCoord.x < gridSize.x &&
      cellCoord.y >= 0 && cellCoord.y < gridSize.y)
  {
    int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( 0, 0, 0 ), 6 );
    //int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( cellCoord, 0 ), 6 );
    int prevIndex = imageAtomicExchange( imgCellCounters, ivec3( cellCoord, 1 ), streamIndex );
/*
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+0 ), vec4( 1.0 ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+1 ), vec4( line0.x ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+2 ), vec4( line0.y ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+3 ), vec4( line1.x ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+4 ), vec4( line1.y ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+5 ), vec4( (float)prevIndex ) );
    */
    
    ptrCellStreams[ streamIndex+0 ] = 1.0;
    ptrCellStreams[ streamIndex+1 ] = line0.x;
    ptrCellStreams[ streamIndex+2 ] = line0.y;
    ptrCellStreams[ streamIndex+3 ] = line1.x;
    ptrCellStreams[ streamIndex+4 ] = line1.y;
    ptrCellStreams[ streamIndex+5 ] = (float) prevIndex;
  }

  discard;
}
