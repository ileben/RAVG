#version 400
#extension GL_NV_gpu_shader5 : enable

smooth in vec2 f_tex;

out vec4 out_color;

void main (void)
{
  float f = f_tex.x * f_tex.x - f_tex.y;
  if (f < 0.0) out_color = vec4( 0,0,0, 1 );
  else discard;
}
