#version 130

in vec2 in_pos;

void main (void)
{
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4( in_pos, 0,1 );
}
