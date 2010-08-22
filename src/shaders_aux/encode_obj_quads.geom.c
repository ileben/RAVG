#version 400

layout( triangles ) in;
layout( triangle_strip ) out;
layout( max_vertices = 4) out;

uniform vec2 gridOrigin;
uniform vec2 cellSize;
uniform ivec2 gridSize;
uniform mat4 modelview;
uniform mat4 projection;

flat out vec2 quad0;
flat out vec2 quad1;
flat out vec2 quad2;

void main()
{
  //Copy the input vertex coordinates to three output variables
  //These will stay the same for every output vertex (i.e. all over the primitive)
  quad0 = gl_in[0].gl_Position.xy;
  quad1 = gl_in[1].gl_Position.xy;
  quad2 = gl_in[2].gl_Position.xy;

  //Find the bounds of the input triangle
  vec2 pmin = min( quad0, min( quad1, quad2 ));
  vec2 pmax = max( quad0, max( quad1, quad2 ));
  
  //Transform and round to grid space
  pmin = floor( (pmin - gridOrigin) / cellSize );
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
