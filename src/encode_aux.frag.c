#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent float* ptrCellStreams;
uniform coherent layout( size1x32 ) image2DArray imgCellStreams;
uniform coherent layout( size1x32 ) iimage2DArray imgCellCounters;
uniform ivec2 gridSize;

in layout( pixel_center_integer ) vec4 gl_FragCoord;

int addLine (vec2 l0, vec2 l1, ivec2 cellCoord);

void main()
{
  ivec2 cellCoord = ivec2( gl_FragCoord );

  //Get and reset auxiliary vertical counter
  int aux = imageAtomicExchange( imgCellCounters, ivec3( cellCoord, 2 ), 0 );

  //Skip writing into this cell if fully occluded by another object
  int cellDone = imageLoad( imgCellCounters, ivec3( cellCoord, 3 ) ).r;
  if (cellDone == 0)
  {
    //Check if auxiliary counter parity is odd
    if (aux % 2 == 1)
    {
      //Add auxiliary vertical segment
      int prevIndex = addLine( vec2( 1.0, 1.25f ), vec2( 1.0, -0.25f ), cellCoord );

      //Check if previous segment was an object (i.e. no other segments in this cell
      int prevType = (int) ptrCellStreams[ prevIndex ];
      if (prevType == 3)
      {
        //Mark the cell fully occluded (further writes to this cell will be skipped)
        imageStore( imgCellCounters, ivec3( cellCoord, 3 ), ivec4( 1 ) );
      }
    }
  }
  
  discard;
}

int addLine (vec2 l0, vec2 l1, ivec2 cellCoord)
{
  int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( 0, 0, 0 ), 6 );
  int prevIndex = imageAtomicExchange( imgCellCounters, ivec3( cellCoord, 1 ), streamIndex );
  
  ptrCellStreams[ streamIndex+0 ] = 1.0;
  ptrCellStreams[ streamIndex+1 ] = l0.x;
  ptrCellStreams[ streamIndex+2 ] = l0.y;
  ptrCellStreams[ streamIndex+3 ] = l1.x;
  ptrCellStreams[ streamIndex+4 ] = l1.y;
  ptrCellStreams[ streamIndex+5 ] = (float) prevIndex;

  return prevIndex;
}
