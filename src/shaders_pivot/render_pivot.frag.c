#version 400

smooth in vec2 f_tex;
out vec4 out_color;

vec4 lookup (vec2 tex);

void main (void)
{
  out_color = lookup( f_tex );
}
