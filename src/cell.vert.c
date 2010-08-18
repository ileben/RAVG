#version 150

uniform mat4 modelview;
uniform mat4 projection;
in vec3 in_pos;
in vec2 in_tex;
out vec2 tex;

void main (void)
{
  tex = in_tex;
  gl_Position = projection * modelview * vec4( in_pos, 1 );
}
