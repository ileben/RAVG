#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_NV_shader_buffer_load : enable

layout( lines ) in;
layout( triangle_strip ) out;
layout( max_vertices = 4) out;

uniform vec2 gridOrigin;
uniform ivec2 gridSize;
uniform vec2 cellSize;

flat out int objectId;

void main()
{
  objectId = (int) gl_in[0].gl_Position.z;
  vec2 pmin = gl_in[0].gl_Position.xy;
  vec2 pmax = gl_in[1].gl_Position.xy;

  //Transform object bounds into grid space
  vec2 gridMin = floor( (pmin - gridOrigin) / cellSize );
  vec2 gridMax = ceil( (pmax - gridOrigin) / cellSize );

  //Transform into [-1,1] normalized coordinates (glViewport will transform back)
  gridMin = (gridMin / gridSize) * 2.0 - vec2(1.0);
  gridMax = (gridMax / gridSize) * 2.0 - vec2(1.0);
  
  //Emit triangle strip forming the bounding box
  gl_Position = vec4( gridMin.x, gridMin.y, 0, 1 ); EmitVertex();
  gl_Position = vec4( gridMax.x, gridMin.y, 0, 1 ); EmitVertex();
  gl_Position = vec4( gridMin.x, gridMax.y, 0, 1 ); EmitVertex();
  gl_Position = vec4( gridMax.x, gridMax.y, 0, 1 ); EmitVertex();
  EndPrimitive();
}
