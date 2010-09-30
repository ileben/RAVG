#version 400

uniform mat4 modelview;
uniform mat4 projection;

in vec3 in_pos;
in vec2 in_tex;

smooth out vec2 f_tex;

void main (void)
{
  //NOTE: Vertex data needs to come from a buffer
  //for gl_VertexID to be defined in shaders

  vec2 texCoords[3];
  texCoords[0] = vec2( 0.0, 0.0 );
  texCoords[1] = vec2( 0.5, 0.0 );
  texCoords[2] = vec2( 1.0, 1.0 );
  f_tex = texCoords[ gl_VertexID % 3 ];

  gl_Position = projection * modelview * vec4( in_pos, 1 );
}
