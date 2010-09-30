#version 400
#extension GL_NV_gpu_shader5 : enable

uniform vec4 color;
out vec4 out_color;

void main (void)
{
  out_color = color;
}
