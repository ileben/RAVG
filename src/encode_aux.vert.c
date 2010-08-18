#version 400

in vec2 in_pos;

void main()
{
  gl_Position = vec4( in_pos, 0, 1 );
}
