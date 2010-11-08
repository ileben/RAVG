#version 400

smooth in vec2 f_tex;
smooth in vec3 f_normal;
out vec4 out_color;

vec4 lookup (vec2 tex);

void main (void)
{
  vec4 tex_color = lookup( f_tex );
  
  float ambient = 0.2;
  vec3 lit_color = ambient * tex_color.rgb;

  vec3 light = normalize( vec3( 1, 1, -1 ) );
  float NdotL = dot( f_normal, light );
  
  if (NdotL > 0.0)
    lit_color += NdotL * tex_color.rgb;
  
  out_color = vec4( lit_color.rgb, tex_color.a );
}
