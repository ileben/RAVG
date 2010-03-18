#version 130

uniform sampler1D sampStream;
in vec2 tex;

vec4 streamTexel;
int streamIndex;

float nextStreamToken()
{
  int c = streamIndex % 4;
  if (c == 0) streamTexel = texelFetch( sampStream, streamIndex / 4, 0 );
  streamIndex++;
  return streamTexel[ c ];
}

int lineWinding (vec2 p0, vec2 p1, vec2 p)
{
  //Find t for p.y, check [0,1] range
  vec2 v = p1 - p0;
  float t = (p.y - p0.y) / v.y;
  if (t < 0.0 || t > 1.0) return 0;

  //Find x, check if on the right side
  float x = p0.x + t * v.x;
  if (p.x < x) return 1;

  return 0;
}

int quadWinding (vec2 p0, vec2 p1, vec2 p2, vec2 p)
{
  int w = 0;

  //Quadratic equation
  float a = (p0.y - 2*p1.y + p2.y);
  float b = (-2*p0.y + 2*p1.y);
  float c = p0.y - p.y;

  //Discriminant
  float d = b*b - 4*a*c;

  //Find roots
  if (d > 0.0) {

    float sd = sqrt( d );
    float t1 = (-b - sd) / (2*a);
    float t2 = (-b + sd) / (2*a);
    
    if (t1 >= 0.0 && t1 <= 1.0) {
      float t = t1;
      float one_t = 1.0 - t;
      float x = one_t*one_t * p0.x + 2*t*one_t * p1.x + t*t * p2.x;
      if (p.x < x) w++;
    }

    if (t2 >= 0.0 && t2 <= 1.0) {
      float t = t2;
      float one_t = 1.0 - t;
      float x = one_t*one_t * p0.x + 2*t*one_t * p1.x + t*t * p2.x;
      if (p.x < x) w++;
    }
  }
  else if (d == 0.0)
  {
    float t = -b / (2*a);

    if (t >= 0.0 && t <= 1.0) {
      float one_t = 1.0 - t;
      float x = one_t*one_t * p0.x + 2*t*one_t * p1.x + t*t * p2.x;
      if (p.x < x) w++;
    }
  }

  return w;
}

void main (void)
{
  float colr = 0.0, colb = 0.0;
  vec2 p = tex; 

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

  //Init stream
  //int w = 0;
  streamIndex = 0;

  //Get number of lines and number of quadratics
  float numLines = nextStreamToken();
  float numQuads = nextStreamToken();
/*
  //Debug
  vec2 p0 = vec2( nextStreamToken(), nextStreamToken() );
  vec2 p1 = vec2( nextStreamToken(), nextStreamToken() );
  vec2 p2 = vec2( nextStreamToken(), nextStreamToken() );
  vec2 p3 = vec2( nextStreamToken(), nextStreamToken() );
  */

  //Intersect lines
  for (int l=0; l<numLines; ++l)
  {
    vec2 p0 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 p1 = vec2( nextStreamToken(), nextStreamToken() );
    
    for (int s=0; s<16; ++s)
      w[s] += lineWinding( p0,p1, p+samples[s] );
  }

  //Intersect quadratics
  for (int q=0; q<numQuads; ++q)
  {    
    vec2 p0 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 p1 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 p2 = vec2( nextStreamToken(), nextStreamToken() );

    for (int s=0; s<16; ++s)
      w[s] += quadWinding( p0,p1,p2, p+samples[s] );
  }

  //Check winding number parity
  float alpha = 0.0;
  for (int s=0; s<16; ++s)
    if (w[s] % 2 == 1) alpha += 0.0625;

  gl_FragColor = vec4( 0,0,0, alpha );
  //gl_FragColor = vec4( 1.0 - alpha, 1.0 - alpha, 1.0 - alpha, 1.0 );
  //if (numLines == 2.0) r = 1.0;
  //if (numQuads == 2.0) b = 1.0;
  //if (p3.x == 100.0 && p3.y == 80.0) b = 1.0;
  //gl_FragColor = vec4( colr,0,colb,1 ) * alpha;
}
