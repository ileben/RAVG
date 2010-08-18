#version 400

//What this basically does is half of the perspective-correct triangle interpolation,
//up to the point where the ABC values for interpolation of u/z and v/z are computed.
//u/z, v/z and 1/z is proved to be linear in screen space so we can interpolate linearly
//regardless of the differences in depth (z) at the corners. A and B basically give us
//the change of interpolated value when the input coordinate changes by 1 in x and y
//respectively. We can calculate inverse of the interpolated texture coordinate in the
//fragment shader and then apply the change to find the correct texture coordinate of
//the nearby samples.

//All this is just doing what the graphics card does internally right after the geometry
//shader anyway, but unfortunatelly we do not have the access to calculated interpolation
//gradients from the fragment shader. There is a bultin support for gradient calculation
//but that is slow and ugly and doesn't do the job :( (see dFdx, dFdx GLSL functions)

layout( triangles ) in;
layout( triangle_strip ) out;
layout( max_vertices = 3) out;

smooth in vec2 v_tex[3];

smooth out vec2 f_tex;
smooth noperspective out float f_invZ;
smooth noperspective out float f_uZ;
smooth noperspective out float f_vZ;

flat out vec2 f_dxTexZ;
flat out vec2 f_dyTexZ;

uniform vec2 viewOrigin;
uniform vec2 viewSize;

void main()
{
  //Shorter names for inputs (I guess they get optimized away)
  vec2 t0 = v_tex[0];
  vec2 t1 = v_tex[1];
  vec2 t2 = v_tex[2];

  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;
  vec4 p2 = gl_in[2].gl_Position;

  //Divide texture coords by z
  vec2 tz0 = t0 / gl_in[0].gl_Position.z;
  vec2 tz1 = t1 / gl_in[1].gl_Position.z;
  vec2 tz2 = t2 / gl_in[2].gl_Position.z;

  //Apply viewport transformation from normalized [-1,1] to screen coords
  vec2 pv0 = viewOrigin + (p0.xy + vec2(1,1)) * 0.5 * viewSize;
  vec2 pv1 = viewOrigin + (p1.xy + vec2(1,1)) * 0.5 * viewSize;
  vec2 pv2 = viewOrigin + (p2.xy + vec2(1,1)) * 0.5 * viewSize;

  //From triangle vertices and their tex coords follows:
  //(where tz stands for (u/z,v/z))

  //A*x0 + B*y0 + C = tz0
  //A*x1 + B*y1 + C = tz1
  //A*x2 + B*y2 + C = tz2

  //A*(x0-x1) + B*(y0-y1) = tz0-tz1
  //A*(x0-x2) + B*(y0-y2) = tz0-tz2

  //Let:

  float P = pv0.x - pv1.x; float Q = pv0.y - pv1.y;
  float S = pv0.x - pv2.x; float T = pv0.y - pv2.y;
  vec2 R = vec2( tz0.x - tz1.x, tz0.y - tz1.y );
  vec2 U = vec2( tz0.x - tz2.x, tz0.y - tz2.y );

  //Solution by Cramer's rule:
  //(replace each column in matrix with right hand side and find determinant;
  //note that the right column is a vec2 since we need to find ABC for
  //both u/z and v/z which yields two sets of equations)

  float det = P * T - S * Q;
  float idet = 1.0 / det;

  f_dxTexZ = (R * T - U * Q) * idet; //= A = "Change of t/z when x changes by 1"
  f_dyTexZ = (P * U - S * R) * idet; //= B = "Change of t/z when y changes by 1"
  //We could calculate C by backsubstitution but we only need the change in tex
  
  //Emit triangle vertices
  f_tex  = t0;
  f_invZ = 1.0  / p0.z;
  f_uZ   = t0.x / p0.z;
  f_vZ   = t0.y / p0.z;
  gl_Position = p0;
  EmitVertex();
  
  f_tex  = t1;
  f_invZ = 1.0  / p1.z;
  f_uZ   = t1.x / p1.z;
  f_vZ   = t1.y / p1.z;
  gl_Position = p1;
  EmitVertex();
  
  f_tex  = t2;
  f_invZ = 1.0  / p2.z;
  f_uZ   = t2.x / p2.z;
  f_vZ   = t2.y / p2.z;
  gl_Position = p2;
  EmitVertex();

  EndPrimitive();
}
