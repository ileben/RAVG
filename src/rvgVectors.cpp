#include "rvgDefs.h"
#include "rvgVectors.h"
#include "rvgMatrix.h"

/*
=========================================
Vectors
=========================================*/

Float Vec::RotationOnPlane2 (const Vec2 &v)
{
  Float norm = v.norm();
  Float cosa = v.x/norm;
  Float sina = v.y/norm;
  return sina>=0 ? ACOS(cosa) : 2*PI-ACOS(cosa);
}

Float Vec::RotationOnPlane2N (const Vec2 &v)
{
  Float cosa = v.x;
  Float sina = v.y;
  return sina>=0 ? ACOS(cosa) : 2*PI-ACOS(cosa);
}

Float Vec::RotationOnPlane3 (const Vec3 &v, const Vec3 &ux, const Vec3 &uy)
{
  Float norm = v.norm();
  Float cosa = Vec::Dot(v, ux)/norm;
  Float sina = Vec::Dot(v, uy)/norm;
  return sina>=0 ? ACOS(cosa) : -ACOS(cosa);
}

Float Vec::RotationOnPlane3N (const Vec3 &v, const Vec3 &ux, const Vec3 &uy)
{    
  Float cosa = Vec::Dot(v, ux);
  Float sina = Vec::Dot(v, uy);
  return sina>=0 ? ACOS(cosa) : -ACOS(cosa);
}

Float Vec::AreaSign2 (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3)
{
  Vec2 v1(p2.x - p1.x, p2.y - p1.y);
  Vec2 v2(p3.x - p1.x, p3.y - p1.y);
  return Vec::Cross(v1,v2)/2;
}

Float Vec::AreaSign3 (const Vec3 &p1, const Vec3 &p2, const Vec3 &p3)
{
  Vec3 v1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
  Vec3 v2(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
  return Vec::Cross(v1,v2).norm()/2;
}

Float Vec::Area2 (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3)
{
  Vec2 v1(p2.x - p1.x, p2.y - p1.y);
  Vec2 v2(p3.x - p1.x, p3.y - p1.y);
  Float signarea = Vec::Cross(v1,v2)/2;
  return signarea < 0.0f ? -signarea : signarea;
}

Float Vec::Area2V (const Vec2 &v1, const Vec2 &v2)
{
  Float signarea = Vec::Cross(v1,v2)/2;
  return signarea < 0.0f ? -signarea : signarea;
}

Float Vec::Area3 (const Vec3 &p1, const Vec3 &p2, const Vec3 &p3)
{
  Vec3 v1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
  Vec3 v2(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
  Float signarea = Vec::Cross(v1,v2).norm()/2;
  return signarea < 0.0f ? -signarea : signarea;
}

Float Vec::Area3V (const Vec3 &v1, const Vec3 &v2)
{
  Float signarea = Vec::Cross(v1,v2).norm()/2;
  return signarea < 0.0f ? -signarea : signarea;
}

bool Vec::InsideTriangle (const Vec2 &p, const Vec2 &p1,
                             const Vec2 &p2, const Vec2 &p3,
                             Float maxerror)
{
  Float whole = Vec::Area2(p1,p2,p3);
  
  Vec2 v1(p1.x - p.x, p1.y - p.y);
  Vec2 v2(p2.x - p.x, p2.y - p.y);
  Vec2 v3(p3.x - p.x, p3.y - p.y);
  
  return (Vec::Area2V(v1, v2) +
          Vec::Area2V(v2, v3) +
          Vec::Area2V(v3, v1) <
          whole + maxerror);
}

/*
=========================================
Quaternion
=========================================*/

void Quat::fromAxisAngle (Float xx, Float yy, Float zz, Float radang)
{
  Float halfang = radang * 0.5f;
  Float sinha = SIN (halfang);
  x = xx * sinha;
  y = yy * sinha;
  z = zz * sinha;
  w = COS (halfang);
}

void Quat::fromAxisAngle (const Vec3 &axis, Float radang)
{
  fromAxisAngle (axis.x, axis.y, axis.z, radang);
}

Quat operator* (const Quat &q1, const Quat &q2)
{
  Quat out;
  out.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
  out.y = q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x;
  out.z = q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w;
  out.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
  return out;
}

/*
--------------------------------------------------------
See Matrix::fromQuat for the relation of matrix
elements with the quaternion components. From that we
can derive the following relations:

M(0,1) + M(1,0) = 4xy
M(2,1) + M(1,2) = 4yz
M(0,2) + M(2,0) = 4xz
M(1,2) - M(2,1) = 4wx
M(2,0) - M(0,2) = 4wy
M(0,1) - M(1,0) = 4wz

The sum of the M diagonal is:
4 * (1 - (q.x*q.x + q.y*q.y + q.z*q.z));

Quaternion is defined as:
(x*sinA, y*sinA, z*sinA, cosA)

Since (x,y,z) has to be some unit vector:
4 * (1 - (q.x*q.x + q.y*q.y + q.z*q.z)) =
4 * (1 - (sq(x) + sq(y) + sq(z)) * sq(sinA)) =
4 * (1 - sq(sinA)) =
4 * (sq(cosA)) =
4 * w * w

From which we derive:
w = sqrt (diagonal) / 2

Similarly we can subtract two of the diagonal
components from the third one instead of taking a sum
to obtain values for x, y or z.
--------------------------------------------------------*/

void Quat::fromMatrix (const Matrix4x4 &m)
{
  Float trace, s, inv2s;
  
  //Check if solving for W is possible
  trace = m.m[0][0] + m.m[1][1] + m.m[2][2] + 1.0f;
  if (trace >= 1.0f)
  {
    s = SQRT (trace);
    inv2s = 0.5f / s;
    this->w = s * 0.5f;
    this->x = (m.m[1][2] - m.m[2][1]) * inv2s;
    this->y = (m.m[2][0] - m.m[0][2]) * inv2s;
    this->z = (m.m[0][1] - m.m[1][0]) * inv2s;
  }
  else
  {
    //Solve for X, Y, Z, depending on which component of the
    //diagonal is the biggest to maximize numberic stability

    if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2]) {
      
      //m00 is the biggest
			s = SQRT (m.m[0][0] - m.m[1][1] - m.m[2][2] + 1.0f);
      inv2s = 0.5f / s;
		  this->w = (m.m[1][2] - m.m[2][1]) * inv2s;
			this->x = s * 0.5f;
      this->y = (m.m[0][1] + m.m[1][0]) * inv2s;
			this->z = (m.m[0][2] + m.m[2][0]) * inv2s;

    }else if (m.m[1][1] > m.m[2][2]) {
      
      //m11 is the biggest
      s = SQRT (m.m[1][1] - m.m[0][0] - m.m[2][2] + 1.0f);
      inv2s = 0.5f / s;
      this->w = (m.m[2][0] - m.m[0][2] ) * inv2s;
      this->x = (m.m[0][1] + m.m[1][0] ) * inv2s;
      this->y = s * 0.5f;
      this->z = (m.m[2][1] + m.m[1][2] ) * inv2s;

    }else {
      
      //m22 is the biggest
      s = SQRT (m.m[2][2] - m.m[0][0] - m.m[1][1] + 1.0f);
      inv2s = 0.5f / s;
      this->w = (m.m[0][1] - m.m[1][0] ) * inv2s;
      this->x = (m.m[0][2] + m.m[2][0] ) * inv2s;
      this->y = (m.m[2][1] + m.m[1][2] ) * inv2s;
      this->z = s * 0.5f;
    }
  }
}

Matrix4x4 Quat::toMatrix ()
{
  Matrix4x4 m;
  m.fromQuat (*this);
  return m;
}

Quat Quat::Slerp (const Quat &q1, const Quat &q2, Float t)
{
  Float angle = ACOS (Quat::Dot (q1, q2));
  Float sina = SIN (angle);
  if (sina == 0.0f) return q1;

  Float a = SIN ((1-t) * angle) / sina;
  Float b = SIN (  t   * angle) / sina;

  Quat qout;
  qout.x =  a * q1.x  +  b * q2.x;
  qout.y =  a * q1.y  +  b * q2.y;
  qout.z =  a * q1.z  +  b * q2.z;
  qout.w =  a * q1.w  +  b * q2.w;
  return qout;
}

Quat Quat::Nlerp (const Quat &q1, const Quat &q2, Float t)
{
  Float a = 1 - t;
  Float b = t;
  
  Quat qout;
  qout.x =  a * q1.x  +  b * q2.x;
  qout.y =  a * q1.y  +  b * q2.y;
  qout.z =  a * q1.z  +  b * q2.z;
  qout.w =  a * q1.w  +  b * q2.w;
  return qout.normalize();
}
