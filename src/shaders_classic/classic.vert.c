#version 400

uniform mat4 modelview;
uniform mat4 projection;

in vec3 in_pos;

void main (void)
{
  gl_Position = projection * modelview * vec4( in_pos, 1 );
}
