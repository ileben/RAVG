#include "rvgMain.h"
#include "rvgVectors.h"

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