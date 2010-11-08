#version 400

smooth in vec2 f_tex;
smooth in vec3 f_normal;
out vec4 out_color;

vec4 lookup (vec2 tex);

float lighting (vec3 normal, vec3 light)
{
  //float NdotL = max( dot( normal, light ), 0.0 );
  float NdotL = abs( dot( normal, light ));
  return NdotL;
}

void main (void)
{
  vec3 normal = normalize( f_normal );
  vec4 texColor = lookup( f_tex );  
  
  //Ambient light
  vec3 ambientColor = vec3( 0.2, 0.2, 0.2 );
  vec3 litColor = ambientColor * texColor.rgb;

  //Front light
  vec3 lightFront = 0.7 * normalize( vec3( 0,1,-1 ));
  litColor += lighting( normal, lightFront ) * texColor.rgb;

  //Back light
  vec3 lightBack = 0.7 * normalize( vec3( 0,-1,-1 ));
  litColor += lighting( normal, lightBack ) * texColor.rgb;
  
  out_color = vec4( litColor.rgb, texColor.a );
}
