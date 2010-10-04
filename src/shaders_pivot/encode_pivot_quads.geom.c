#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_NV_shader_buffer_load : enable

layout( triangles ) in;
layout( triangle_strip ) out;
layout( max_vertices = 4) out;

uniform int* ptrObjects;
uniform int objectId;

uniform vec2 gridOrigin;
uniform vec2 cellSize;
uniform ivec2 gridSize;

flat out vec2 quad0;
flat out vec2 quad1;
flat out vec2 quad2;
flat out ivec2 quadMin;

void main()
{
  //Copy the input vertex coordinates to three output variables
  //These will stay the same for every output vertex (i.e. all over the primitive)
  quad0 = gl_in[0].gl_Position.xy;
  quad1 = gl_in[1].gl_Position.xy;
  quad2 = gl_in[2].gl_Position.xy;

  //Get object pointer and grid info
  int *ptrObj = ptrObjects + objectId * NODE_SIZE_OBJINFO;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );

  //Find the bounds of the input triangle
  vec2 gridMin = min( quad0, min( quad1, quad2 ));
  vec2 gridMax = max( quad0, max( quad1, quad2 ));
  
  //Transform and round to grid space
  gridMin = floor( (gridMin - gridOrigin) / cellSize );
  gridMax = ceil( (gridMax - gridOrigin) / cellSize );
  quadMin = ivec2( gridMin );

  //Extend left side to object boundary
  gridMin.x = objGridOrigin.x;

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
