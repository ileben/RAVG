#version 150

uniform mat4 modelview;
uniform mat4 projection;
in vec2 in_pos;
in int in_wind;
out vec3 color;

void main (void)
{
  if (in_wind % 2 == 1)
    color = vec3(0,1,0);
  else
    color = vec3(1,0,0);

  gl_Position = projection * modelview * vec4( in_pos, 0,1 );
}
