#version 130
in vec2 in_pos;
out vec2 pos;

void main (void)
{
  pos = in_pos;
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4( in_pos, 0,1 );
}
