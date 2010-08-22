#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent float* ptrCellStreams;
uniform coherent layout( size1x32 ) image2DArray imgCellStreams;
uniform coherent layout( size1x32 ) iimage2DArray imgCellCounters;
uniform ivec2 gridSize;
uniform vec4 color;

in layout( pixel_center_integer ) vec4 gl_FragCoord;

void addLine (vec2 l0, vec2 l1, ivec2 gridCoord);
void addObject (vec4 color, ivec2 gridCoord);


void main()
{
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    //Get and reset auxiliary vertical counter
    int aux = imageAtomicExchange( imgCellCounters, ivec3( gridCoord, 2 ), 0 );

    //Skip writing into this cell if fully occluded by another object
    int cellDone = imageLoad( imgCellCounters, ivec3( gridCoord, 3 ) ).r;
    if (cellDone == 0)
    {
      bool anySegments = false;

      //Get index of the previous node
      int prevIndex = imageLoad( imgCellCounters, ivec3( gridCoord, 1 ) ).r;
      if (prevIndex >= 0)
      {
        //Check if previous node other than an object start
        int prevType = (int) ptrCellStreams[ prevIndex ];
        if (prevType != 3) anySegments = true;
      }

      //Check if auxiliary counter parity is odd
      if (aux % 2 == 1)
      {
        //Add auxiliary vertical segment
        addLine( vec2( 1.0, 1.25f ), vec2( 1.0, -0.25f ), gridCoord );

        //If no other segments, mark the cell fully occluded
        if (!anySegments) imageStore( imgCellCounters, ivec3( gridCoord, 3 ), ivec4( 1 ) );
        anySegments = true;
      }
      
      //Add object data if any of its segments in this cell
      if (anySegments) addObject( color, gridCoord );
    }
  }

  discard;
}

void addLine (vec2 l0, vec2 l1, ivec2 gridCoord)
{
  int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( 0, 0, 0 ), 6 );
  int prevIndex = imageAtomicExchange( imgCellCounters, ivec3( gridCoord, 1 ), streamIndex );
  
  ptrCellStreams[ streamIndex+0 ] = 1.0;
  ptrCellStreams[ streamIndex+1 ] = l0.x;
  ptrCellStreams[ streamIndex+2 ] = l0.y;
  ptrCellStreams[ streamIndex+3 ] = l1.x;
  ptrCellStreams[ streamIndex+4 ] = l1.y;
  ptrCellStreams[ streamIndex+5 ] = (float) prevIndex;
}

void addObject (vec4 color, ivec2 gridCoord)
{
  int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( 0, 0, 0 ), 6 );
  int prevIndex = imageAtomicExchange( imgCellCounters, ivec3( gridCoord, 1 ), streamIndex );
  
  ptrCellStreams[ streamIndex+0 ] = 3.0;
  ptrCellStreams[ streamIndex+1 ] = color.r;
  ptrCellStreams[ streamIndex+2 ] = color.g;
  ptrCellStreams[ streamIndex+3 ] = color.b;
  ptrCellStreams[ streamIndex+4 ] = color.a;
  ptrCellStreams[ streamIndex+5 ] = (float) prevIndex;
}
