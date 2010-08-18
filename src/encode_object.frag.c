#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent float* ptrCellStreams;
uniform coherent layout( size1x32 ) image2DArray imgCellStreams;
uniform coherent layout( size1x32 ) iimage2DArray imgCellCounters;
uniform float* ptrLinesQuads;

uniform sampler1D sampGrid;
uniform vec2 gridOrigin;
uniform vec2 cellSize;
uniform ivec2 gridSize;
uniform vec4 color;

in layout( pixel_center_integer ) vec4 gl_FragCoord;

int streamIndex = 0;

float nextStreamToken ()
{
  float streamToken = ptrLinesQuads[ streamIndex ];
  streamIndex++;
  return streamToken;
}

int lineWinding (vec2 l0, vec2 l1, vec2 p0, vec2 p1)
{
  int w = 0;

  //Line equation
  float AX = (l1.x - l0.x);
  float BX = l0.x;

  float AY = (l1.y - l0.y);
  float BY = l0.y;

  //Line from pivot to fragment point
  float ax = (p1.x - p0.x);
  float bx = p0.x;

  float ay = (p1.y - p0.y);
  float by = p0.y;

  //Check for vertical line
  if (ax == 0.0) {
    float tmp;
    tmp = ax; ax = ay; ay = tmp;
    tmp = bx; bx = by; by = tmp;
    tmp = AX; AX = AY; AY = tmp;
    tmp = BX; BX = BY; BY = tmp;
  }

  //Intersection equation
  float r = ay / ax;
  float a = (AY - r * AX);
  float b = (BY - r * BX) - (by - r * bx);

  //Find root
  float t = -b/a;
  if (t >= 0.0 && t <= 1.0) {
    float k = t * AX/ax + BX/ax - bx/ax;
    if (k >= 0.0 && k <= 1.0) w++;
  }
  
  return w;
}

int quadWinding (vec2 q0, vec2 q1, vec2 q2, vec2 p0, vec2 p1)
{
  int w = 0;

  //Quadratic equation
  float AX = (q0.x - 2*q1.x + q2.x);
  float BX = (-2*q0.x + 2*q1.x);
  float CX = q0.x;

  float AY = (q0.y - 2*q1.y + q2.y);
  float BY = (-2*q0.y + 2*q1.y);
  float CY = q0.y;

  //Line from pivot to fragment point
  float ax = (p1.x - p0.x);
  float bx = p0.x;

  float ay = (p1.y - p0.y);
  float by = p0.y;

  //Check for vertical line
  if (ax == 0.0) {
    float tmp;
    tmp = ax; ax = ay; ay = tmp;
    tmp = bx; bx = by; by = tmp;
    tmp = AX; AX = AY; AY = tmp;
    tmp = BX; BX = BY; BY = tmp;
    tmp = CX; CX = CY; CY = tmp;
  }

  //Intersection equation
  float r = ay / ax;
  float a = (AY - r * AX);
  float b = (BY - r * BX);
  float c = (CY - r * CX) - (by - r * bx);

  //Discriminant
  float d = b*b - 4*a*c;

  //Find roots
  if (d > 0.0) {

    float sd = sqrt( d );
    float t1 = (-b - sd) / (2*a);
    float t2 = (-b + sd) / (2*a);
    
    if (t1 >= 0.0 && t1 <= 1.0) {
      float k1 = t1*t1 * AX/ax + t1 * BX/ax + CX/ax - bx/ax;
      if (k1 >= 0.0 && k1 <= 1.0) w++;
    }

    if (t2 >= 0.0 && t2 <= 1.0) {
      float k2 = t2*t2 * AX/ax + t2 * BX/ax + CX/ax - bx/ax;
      if (k2 >= 0.0 && k2 <= 1.0) w++;
    }
  }

  return w;
}

void main()
{
  ivec2 cellCoord = ivec2( gl_FragCoord.xy );
  if (cellCoord.x >= 0 && cellCoord.x < gridSize.x &&
      cellCoord.y >= 0 && cellCoord.y < gridSize.y)
  {
    vec2 pivot = gridOrigin + (vec2(cellCoord) + vec2(0.5,0.5)) * cellSize;
    vec2 testPivot = gridOrigin - vec2( 1.0, 1.0 );
    int gridWinding = 0;
    streamIndex = 0;

    int numLines = (int) nextStreamToken();
    int numQuads = (int) nextStreamToken();

    for (int l=0; l<numLines; ++l)
    {
      vec2 l0 = vec2( nextStreamToken(), nextStreamToken() );
      vec2 l1 = vec2( nextStreamToken(), nextStreamToken() );
      gridWinding += lineWinding( l0,l1, pivot, testPivot );
    }

    for (int q=0; q<numQuads; ++q)
    {
      vec2 q0 = vec2( nextStreamToken(), nextStreamToken() );
      vec2 q1 = vec2( nextStreamToken(), nextStreamToken() );
      vec2 q2 = vec2( nextStreamToken(), nextStreamToken() );
      gridWinding += quadWinding( q0,q1,q2, pivot, testPivot );
    }

    int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( 0, 0, 0 ), 7 );
    //int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( cellCoord, 0 ), 7 );
    int prevIndex = imageAtomicExchange( imgCellCounters, ivec3( cellCoord, 1 ), streamIndex );
    imageAtomicAdd( imgCellCounters, ivec3( cellCoord, 2 ), 7 );
    /*
    int gridIndex = cellCoord.y * gridSize.x + cellCoord.x;
    vec4 gridTexel = texelFetch( sampGrid, gridIndex, 0 );
    float gridWinding = gridTexel[0];
    */
/*
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+0 ), vec4( 3.0 ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+1 ), vec4( gridWinding ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+2 ), vec4( color.r ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+3 ), vec4( color.g ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+4 ), vec4( color.b ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+5 ), vec4( color.a ) );
    imageStore( imgCellStreams, ivec3( cellCoord, streamIndex+6 ), vec4( (float)prevIndex ) );
*/
    
    ptrCellStreams[ streamIndex+0 ] = 3.0;
    ptrCellStreams[ streamIndex+1 ] = gridWinding;
    ptrCellStreams[ streamIndex+2 ] = color.r;
    ptrCellStreams[ streamIndex+3 ] = color.g;
    ptrCellStreams[ streamIndex+4 ] = color.b;
    ptrCellStreams[ streamIndex+5 ] = color.a;
    ptrCellStreams[ streamIndex+6 ] = (float) prevIndex;
  }
  discard;
}
