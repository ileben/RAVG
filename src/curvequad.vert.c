#version 130

in vec2 in_uv;
in vec3 in_abcU;
in vec3 in_abcV;
in vec2 in_pos;

out vec2 uv;
out vec4 c;
out vec3 abcU;
out vec3 abcV;

void main (void)
{
  abcU = in_abcU;
  abcV = in_abcV;

  vec2 uvs[3];
  uvs[ 0 ] = vec2( 0.0, 0.0);
  uvs[ 1 ] = vec2( 0.5, 0.0);
  uvs[ 2 ] = vec2( 1.0, 1.0);
  uv = uvs[ gl_VertexID % 3 ];
  
  vec4 cs[3];
  cs[ 0 ] = vec4( 1, 0, 0, 1 );
  cs[ 1 ] = vec4( 0, 1, 0, 1 );
  cs[ 2 ] = vec4( 0, 0, 1, 1 );
  c = cs[ gl_VertexID % 3 ];
  
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4( in_pos, 0,1 );
}
