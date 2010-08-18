#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform int* ptrCpuIndirection;
uniform float* ptrCpuStream;

uniform vec2 cellSize;
uniform ivec2 gridSize;
uniform vec2 gridOrigin;
in vec2 tex;
out vec4 out_color;

int streamIndex = 0;

float nextStreamToken ()
{
  float streamToken = ptrCpuStream[ streamIndex ];
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

void main (void)
{
  vec2 p = tex; 
  vec3 finalColor = vec3( 1,1,1 );

  //Init samples
  vec2 samples[16];
  samples[0] = vec2( -0.109808, -0.409808 );
  samples[1] = vec2( -0.209808, -0.236603 );
  samples[2] = vec2( -0.309808, -0.0633975 );
  samples[3] = vec2( -0.409808, 0.109808 );
  samples[4] = vec2( 0.0633975, -0.309808 );
  samples[5] = vec2( -0.0366025, -0.136603 );
  samples[6] = vec2( -0.136603, 0.0366025 );
  samples[7] = vec2( -0.236603, 0.209808 );
  samples[8] = vec2( 0.236603, -0.209808 );
  samples[9] = vec2( 0.136603, -0.0366025 );
  samples[10] = vec2( 0.0366025, 0.136603 );
  samples[11] = vec2( -0.0633975, 0.309808 );
  samples[12] = vec2( 0.409808, -0.109808 );
  samples[13] = vec2( 0.309808, 0.0633975 );
  samples[14] = vec2( 0.209808, 0.236603 );
  samples[15] = vec2( 0.109808, 0.409808 );

  int numSamples = 16;
  int w[16];
  w[0] = 0;
  w[1] = 0;
  w[2] = 0;
  w[3] = 0;
  w[4] = 0;
  w[5] = 0;
  w[6] = 0;
  w[7] = 0;
  w[8] = 0;
  w[9] = 0;
  w[10] = 0;
  w[11] = 0;
  w[12] = 0;
  w[13] = 0;
  w[14] = 0;
  w[15] = 0;

  //Find grid coordinate and pivot
  ivec2 gridCoord = ivec2(floor( (p - gridOrigin) / cellSize ));
  vec2 pivot = gridOrigin + (vec2(gridCoord) + vec2(0.5,0.5)) * cellSize;

  //Check if coordinate in range
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    //Find cell bounds
    vec2 cmin = vec2(
      gridOrigin.x + gridCoord.x * cellSize.x,
      gridOrigin.y + gridCoord.y * cellSize.y );

    vec2 cmax = vec2(
      gridOrigin.x + (gridCoord.x + 1.0f) * cellSize.x,
      gridOrigin.y + (gridCoord.y + 1.0f) * cellSize.y );

    //Init stream index
    int gridIndex = gridCoord.y * gridSize.x + gridCoord.x;
    streamIndex = ptrCpuIndirection[ gridIndex ];

    //Loop until end of stream
    int safetyCounter = 0;
    while (true)
    {
      if (++safetyCounter == 10000) { finalColor = vec3(1,0,0); break; }
      int segType = (int) nextStreamToken();

      if (segType == 1)
      {
        vec2 l0 = vec2( nextStreamToken(), nextStreamToken() );
        vec2 l1 = vec2( nextStreamToken(), nextStreamToken() );
        
        for (int s=0; s<numSamples; ++s)
          w[s] += lineWinding( l0,l1, pivot, p+samples[s] );
      }
      else if (segType == 2)
      {
        vec2 q0 = vec2( nextStreamToken(), nextStreamToken() );
        vec2 q1 = vec2( nextStreamToken(), nextStreamToken() );
        vec2 q2 = vec2( nextStreamToken(), nextStreamToken() );

        for (int s=0; s<numSamples; ++s)
          w[s] += quadWinding( q0,q1,q2, pivot, p+samples[s] );
      }
      else if (segType == 3)
      {
        int objWind = (int) nextStreamToken();
        vec4 objColor = vec4( nextStreamToken(), nextStreamToken(), nextStreamToken(), nextStreamToken() );
        
        float alpha = 0.0;
        float afract = 1.0 / float(numSamples);
        for (int s=0; s<numSamples; ++s)
        {
          if ((objWind + w[s]) % 2 == 1) alpha += afract;
          w[s] = 0;
        }

        finalColor = (1.0 - alpha) * finalColor + alpha * objColor.rgb;
      }
      else break;
    }
  }

  out_color = vec4( finalColor, 1 );
}
