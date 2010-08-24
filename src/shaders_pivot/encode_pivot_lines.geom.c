#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_NV_shader_buffer_load : enable

layout( lines ) in;
layout( triangle_strip ) out;
layout( max_vertices = 4) out;

uniform int* ptrObjects;
uniform int objectId;

uniform vec2 gridOrigin;
uniform ivec2 gridSize;
uniform vec2 cellSize;

flat out vec2 line0;
flat out vec2 line1;
flat out ivec2 lineMin;

void main()
{
  //Copy the input vertex coordinates to three output variables
  //These will stay the same for every output vertex (i.e. all over the primitive)
  line0 = gl_in[0].gl_Position.xy;
  line1 = gl_in[1].gl_Position.xy;

  //Get object pointer and grid info
  int *ptrObj = ptrObjects + objectId * 5;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );

  //Find the bounds of the input line
  vec2 gridMin = min( line0, line1 );
  vec2 gridMax = max( line0, line1 );
  
  //Transform and round to grid space
  gridMin = floor( (gridMin - gridOrigin) / cellSize );
  gridMax = ceil( (gridMax - gridOrigin) / cellSize );
  lineMin = ivec2( gridMin );

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
