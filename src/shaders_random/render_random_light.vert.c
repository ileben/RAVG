#version 150

uniform mat4 modelview;
uniform mat4 projection;
uniform mat4 matTexture;

in vec3 in_pos;
in vec2 in_tex;
in vec3 in_normal;

smooth out vec2 f_tex;
smooth out vec3 f_normal;

void main (void)
{
  mat3 normalview = transpose( inverse( mat3( modelview )));
  f_normal = normalview * in_normal;

  f_tex = (matTexture * vec4( in_tex, 0.0, 1.0 )).xy;
  gl_Position = projection * modelview * vec4( in_pos, 1 );
}
