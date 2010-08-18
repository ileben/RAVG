#version 150

uniform sampler1D sampStream;
uniform vec2 pivot;
in vec2 tex;
out vec4 out_color;

vec4 streamTexel;
int streamIndex;

float nextStreamToken()
{
  int c = streamIndex % 4;
  if (c == 0) streamTexel = texelFetch( sampStream, streamIndex / 4, 0 );
  streamIndex++;
  return streamTexel[ c ];
}
/*
float nextStreamToken()
{
  streamTexel = texelFetch( sampStream, streamIndex, 0 );
  streamIndex++;
  return streamTexel.g;
}*/

/*
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

  return w;
}
*/

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

  const int wStart = 1;
  int w[16];
  w[0] = wStart;
  w[1] = wStart;
  w[2] = wStart;
  w[3] = wStart;
  w[4] = wStart;
  w[5] = wStart;
  w[6] = wStart;
  w[7] = wStart;
  w[8] = wStart;
  w[9] = wStart;
  w[10] = wStart;
  w[11] = wStart;
  w[12] = wStart;
  w[13] = wStart;
  w[14] = wStart;
  w[15] = wStart;
  //vec2 pivot = vec2(100,20);

  //Init stream
  //int w = 0;
  streamIndex = 0;

  //Get number of lines and number of quadratics
  float numLines = nextStreamToken();
  float numQuads = nextStreamToken();
  //float numCubics = nextStreamToken();
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
    vec2 l0 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 l1 = vec2( nextStreamToken(), nextStreamToken() );
    
    for (int s=0; s<16; ++s)
      w[s] += lineWinding( l0,l1, pivot, p+samples[s] );
      //w[s] += lineWinding( l0,l1, p+samples[s] );
  }

  //Intersect quadratics
  for (int q=0; q<numQuads; ++q)
  {    
    vec2 q0 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 q1 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 q2 = vec2( nextStreamToken(), nextStreamToken() );

    for (int s=0; s<16; ++s)
      w[s] += quadWinding( q0,q1,q2, pivot, p+samples[s] );
      //w[s] += quadWinding( q0,q1,q2, p+samples[s] );
  }

  //for (int i=0; i<numCubics; ++i)
  //{
  //}
  /*
  for (int i=0; i<numCubics; ++i)
  {
    vec2 p0 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 p1 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 p2 = vec2( nextStreamToken(), nextStreamToken() );
    vec2 p3 = vec2( nextStreamToken(), nextStreamToken() );

    vec2 p01 = (p0 + p1) * 0.5;
    vec2 p12 = (p1 + p2) * 0.5;
    vec2 p23 = (p2 + p3) * 0.5;

    vec2 p001 = (p0 + p01) * 0.5;
    vec2 a = (p001 + p01) * 0.5;

    vec2 p233 = (p23 + p3) * 0.5;
    vec2 d = (p23 + p233) * 0.5;

    vec2 p0112 = (p01 + p12) * 0.5;
    vec2 p1223 = (p12 + p23) * 0.5;
    vec2 B = (p0112 + p1223) * 0.5;

    vec2 p0112B = (p0112 + B) * 0.5;
    vec2 b = (p0112 + p0112B) * 0.5;

    vec2 pB1223 = (B + p1223) * 0.5;
    vec2 c = (pB1223 + p1223) * 0.5;

    vec2 A = (a + b) * 0.5;
    vec2 C = (c + d) * 0.5;

    for (int s=0; s<16; ++s)
    {
      w[s] += quadWinding( p0,a,A, p+samples[s] );
      w[s] += quadWinding( A,b,B, p+samples[s] );
      w[s] += quadWinding( B,c,C, p+samples[s] );
      w[s] += quadWinding( C,d,p3, p+samples[s] );
    }
  }*/

  //Check winding number parity
  float alpha = 0.0;
  for (int s=0; s<16; ++s)
    if (w[s] % 2 == 1) alpha += 0.0625;

  out_color = vec4( 0,0,0, alpha );
  //gl_FragColor = vec4( 0,0,0, alpha );
  //gl_FragColor = vec4( 1.0 - alpha, 1.0 - alpha, 1.0 - alpha, 1.0 );
  //if (numLines == 2.0) r = 1.0;
  //if (numQuads == 2.0) b = 1.0;
  //if (p3.x == 100.0 && p3.y == 80.0) b = 1.0;
  //gl_FragColor = vec4( colr,0,colb,1 ) * alpha;
}
