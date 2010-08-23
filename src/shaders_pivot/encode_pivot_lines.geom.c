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

void main()
{
  //Copy the input vertex coordinates to three output variables
  //These will stay the same for every output vertex (i.e. all over the primitive)
  line0 = gl_in[0].gl_Position.xy;
  line1 = gl_in[1].gl_Position.xy;
/*
  //Get object pointer and grid info
  int *ptrObj = ptrObjects + objectId * 5;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
  ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );

  //Get object bounds
  vec2 pmin = vec2( objGridOrigin );
  vec2 pmax = vec2( objGridSize - objGridOrigin );

  //Transform into [-1,1] normalized coordinates (glViewport will transform back)
  pmin = (pmin / gridSize) * 2.0 - vec2(1.0);
  pmax = (pmax / gridSize) * 2.0 - vec2(1.0);

  //pmin = vec2(-1,-1);
  //pmax = vec2(1,1);
*/

  //Find the bounds of the input line
  vec2 pmin = min( line0, line1 );
  vec2 pmax = max( line0, line1 );
  
  //Transform and round to grid space
  pmin = vec2(0,0);
  //pmin = floor( (pmin - gridOrigin) / cellSize );
  pmax = ceil( (pmax - gridOrigin) / cellSize );

  //Transform into [-1,1] normalized coordinates (glViewport will transform back)
  pmin = (pmin / gridSize) * 2.0 - vec2(1.0);
  pmax = (pmax / gridSize) * 2.0 - vec2(1.0);

  //Emit triangle strip forming the bounding box
  gl_Position = vec4( pmin.x, pmin.y, 0, 1 ); EmitVertex();
  gl_Position = vec4( pmax.x, pmin.y, 0, 1 ); EmitVertex();
  gl_Position = vec4( pmin.x, pmax.y, 0, 1 ); EmitVertex();
  gl_Position = vec4( pmax.x, pmax.y, 0, 1 ); EmitVertex();
  EndPrimitive();
}
