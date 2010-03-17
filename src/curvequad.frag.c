in vec2 uv;
in vec4 c;
in vec3 abcU;
in vec3 abcV;

void main (void)
{
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
  
  float alpha = 0.0;
  for (int s=0; s<16; ++s) {

    vec2 p = gl_FragCoord.xy + samples[s];
    float u = abcU.x * p.x + abcU.y * p.y + abcU.z;
    float v = abcV.x * p.x + abcV.y * p.y + abcV.z;
    if (u * u - v < 0.0)
      alpha += 0.0625;
  }

  if (alpha == 0.0)
    discard;
  
  gl_FragColor = vec4( 1.0 - alpha, 1.0 - alpha, 1.0 - alpha, 1.0 );
  
  /*
  vec2 p = gl_FragCoord.xy;
  float u = abcU.x * p.x + abcU.y * p.y + abcU.z;
  float v = abcV.x * p.x + abcV.y * p.y + abcV.z;
  if (u * u - v < 0.0)
    gl_FragColor = c;
  else discard;
  */
  
  /*
  if (uv.x * uv.x - uv.y < 0.0)
    gl_FragColor = c;
  else discard;
  */

  //gl_FragColor = c;
  //gl_FragColor = vec4( uv.x, 0, uv.y, 1 );
}
