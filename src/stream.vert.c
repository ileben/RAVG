#version 130
in vec2 in_pos;
in vec2 in_tex;
out vec2 tex;

void main (void)
{
  tex = in_tex;
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4( in_pos, 0,1 );
}
