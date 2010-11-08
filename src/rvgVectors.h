#ifndef __RVGVECTORS_H
#define __RVGVECTORS_H 1

/*
------------------------------------
Forward declarations
------------------------------------*/

template <class S> class TVec2;
template <class S> class TVec3;
template <class S> class TVec4;
class Matrix4x4;

/*
=============================================
2, 3, and 4-dimensional vectors
=============================================*/

template <class S>
class TVec2
{
public:

  S x, y;

  template <class T>
  explicit TVec2 (const TVec2<T> &v);

  TVec2<S> yx () const;

  TVec3<S> xy (S z) const;
  TVec4<S> xy (S z, S w) const;
  
  TVec2 ();
  TVec2 (S xy);
  TVec2 (S x0, S y0);
  TVec2<S>& set (S x0, S y0);
  TVec2<S>& operator= (const TVec2<S> &v);
  bool operator== (const TVec2<S> &v) const;
  bool operator!= (const TVec2<S> &v) const;
  
  TVec2<S>& operator+= (const TVec2<S> &v);
  TVec2<S>& operator-= (const TVec2<S> &v);
  TVec2<S>& operator*= (Float k);
  TVec2<S>& operator/= (Float k);
  
  TVec2<S> operator+ (const TVec2<S> &v) const;
  TVec2<S> operator- (const TVec2<S> &v) const;
  TVec2<S> operator* (const TVec2<S> &v) const;
  TVec2<S> operator/ (const TVec2<S> &v) const;
  TVec2<S> operator* (Float k) const;
  TVec2<S> operator/ (Float k) const;
  
  Float norm () const;
  Float normSq () const;
  TVec2<S>& normalize ();
  
  TVec2<S>& offset (Float ox, Float oy);
  TVec2<S>& offsetV (const TVec2<S> &v, Float d);
  TVec2<S>& offsetVN (const TVec2<S> &v, Float d);
  TVec2<S> reverse () const;
};

template <class S>
class TVec3
{
public:

  S x, y, z;

  template <class T>
  explicit TVec3 (const TVec3<T> &v);
  
  TVec2<S> xy () const;
  TVec2<S> xz () const;
  TVec2<S> yz () const;
  TVec4<S> xyz (S w) const;
  
  TVec3 ();
  TVec3 (S xyz);
  TVec3 (S x0, S y0, S z0);
  TVec3 (const TVec2<S> &v, S z0);

  TVec3<S>& set (S x0, S y0, S z0);
  TVec3<S>& operator= (const TVec3<S> &v);
  bool operator== (const TVec3<S> &v) const;
  bool operator!= (const TVec3<S> &v) const;
  
  TVec3<S>& operator+= (const TVec3<S> &v);
  TVec3<S>& operator-= (const TVec3<S> &v);
  TVec3<S>& operator*= (Float k);
  TVec3<S>& operator/= (Float k);
  
  TVec3<S> operator+ (const TVec3<S> &v) const;
  TVec3<S> operator- (const TVec3<S> &v) const;
  TVec3<S> operator* (const TVec3<S> &v) const;
  TVec3<S> operator/ (const TVec3<S> &v) const;
  TVec3<S> operator* (Float k) const;
  TVec3<S> operator/ (Float k) const;
  
  Float norm () const;
  Float normSq () const;
  TVec3<S>& normalize ();
  
  TVec3<S>& offset (Float ox, Float oy, Float oz);
  TVec3<S>& offsetV (const TVec3<S> &v, Float d);
  TVec3<S>& offsetVN (const TVec3<S> &v, Float d);
  TVec3<S> reverse () const;
};

template <class S>
class TVec4
{
public:

  S x, y, z, w;

  template <class T>
  explicit TVec4 (const TVec4<T> &v);
  
  TVec2<S> xy () const;
  TVec3<S> xyz () const;
  
  TVec4 ();
  TVec4 (S xyzw);
  TVec4 (S x0, S y0, S z0, S w0);
  TVec4 (const TVec2<S> &v, S z0, S w0 );
  TVec4 (const TVec3<S> &v, S w0 );

  TVec4<S>& set (S x0, S y0, S z0, S w0);
  TVec4<S>& operator= (const TVec4<S> &v);
  bool operator== (const TVec4<S> &v) const;
  bool operator!= (const TVec4<S> &v) const;
  
  TVec4<S>& operator+= (const TVec4<S> &v);
  TVec4<S>& operator-= (const TVec4<S> &v);
  TVec4<S>& operator*= (Float k);
  TVec4<S>& operator/= (Float k);

  TVec4<S> operator+ (const TVec4<S> &v) const;
  TVec4<S> operator- (const TVec4<S> &v) const;
  TVec4<S> operator* (const TVec4<S> &v) const;
  TVec4<S> operator/ (const TVec4<S> &v) const;
  TVec4<S> operator* (Float k) const;
  TVec4<S> operator/ (Float k) const;
  
  Float norm () const;
  Float normSq () const;
  TVec4<S>& normalize ();
  
  TVec4<S>& offset (Float ox, Float oy, Float ow, Float oz);
  TVec4<S>& offsetV (const TVec4<S> &v, Float d);
  TVec4<S>& offsetVN (const TVec4<S> &v, Float d);
  TVec4<S> reverse () const;
};

template <class S>
class TVec
{
public:

  static TVec2<S> Floor (const TVec2<S> &v);
  static TVec3<S> Floor (const TVec3<S> &v);
  static TVec4<S> Floor (const TVec4<S> &v);

  static TVec2<S> Ceil (const TVec2<S> &v);
  static TVec3<S> Ceil (const TVec3<S> &v);
  static TVec4<S> Ceil (const TVec4<S> &v);

  static TVec2<S> Fract (const TVec2<S> &v);
  static TVec3<S> Fract (const TVec3<S> &v);
  static TVec4<S> Fract (const TVec4<S> &v);

  static TVec2<S> Min (const TVec2<S> &v1, const TVec2<S> &v2);
  static TVec3<S> Min (const TVec3<S> &v1, const TVec3<S> &v2);
  static TVec4<S> Min (const TVec4<S> &v1, const TVec4<S> &v2);

  static TVec2<S> Max (const TVec2<S> &v1, const TVec2<S> &v2);
  static TVec3<S> Max (const TVec3<S> &v1, const TVec3<S> &v2);
  static TVec4<S> Max (const TVec4<S> &v1, const TVec4<S> &v2);
  
  static Float Dot (const TVec2<S> &v1, const TVec2<S> &v2);
  static Float Dot (const TVec3<S> &v1, const TVec3<S> &v2);
  static Float Dot (const TVec4<S> &v1, const TVec4<S> &v2);
  
  static Float Angle (const TVec2<S> &v1, const TVec2<S> &v2);
  static Float Angle (const TVec3<S> &v1, const TVec3<S> &v2);
  static Float Angle (const TVec4<S> &v1, const TVec4<S> &v2);
  
  static Float AngleN (const TVec2<S> &v1, const TVec2<S> &v2);
  static Float AngleN (const TVec3<S> &v1, const TVec3<S> &v2);
  static Float AngleN (const TVec4<S> &v1, const TVec4<S> &v2);
  
  static Float Cross (const TVec2<S> &v1, const TVec2<S> &v2);
  static TVec3<S> Cross (const TVec3<S> &v1, const TVec3<S> &v2);

  template <class V> static V Lerp (const V &v1, const V &v2, Float t);
  
  static Float RotationOnPlane2 (const TVec2<S> &v);
  static Float RotationOnPlane2N (const TVec2<S> &v);
  static Float RotationOnPlane3 (const TVec3<S> &v, const TVec3<S> &ux, const TVec3<S> &uy);
  static Float RotationOnPlane3N (const TVec3<S> &v, const TVec3<S> &ux, const TVec3<S> &uy);
  
  static Float AreaSign2 (const TVec2<S> &p1, const TVec2<S> &p2, const TVec2<S> &p3);
  static Float AreaSign3 (const TVec3<S> &p1, const TVec3<S> &p2, const TVec3<S> &p3);
  static Float Area2 (const TVec2<S> &p1, const TVec2<S> &p2, const TVec2<S> &p3);
  static Float Area2V (const TVec2<S> &v1, const TVec2<S> &v2);
  static Float Area3 (const TVec3<S> &p1, const TVec3<S> &p2, const TVec3<S> &p3);
  static Float Area3V (const TVec3<S> &v1, const TVec3<S> &v2);
  static bool InsideTriangle (const TVec2<S> &p, const TVec2<S> &p1, const TVec2<S> &p2,
                              const TVec2<S> &p3, Float maxerror=0.0001);
};


/*
=============================================
Commond vector types
=============================================*/

typedef TVec2<Float> Vec2;
typedef TVec3<Float> Vec3;
typedef TVec4<Float> Vec4;

typedef TVec2<Int32> IVec2;
typedef TVec3<Int32> IVec3;
typedef TVec4<Int32> IVec4;

typedef TVec<Float> Vec;
typedef TVec<Int32> IVec;


/*
=============================================
Quaternion
=============================================*/

class Quat
{
public:

  Float x,y,z,w;

  Quat ();
  Quat (Float x, Float y, Float z, Float w);
  void set (Float x, Float y, Float z, Float w);

  Float norm () const;
  Float normSq () const;
  Quat& normalize ();

  void setIdentity ();
  void fromAxisAngle (Float x, Float y, Float z, Float radang);
  void fromAxisAngle (const Vec3 &axis, Float radang);
  void fromMatrix (const Matrix4x4 &m);
  Matrix4x4 toMatrix ();
  
  static Quat Slerp (const Quat &q1, const Quat &q2, Float t);
  static Quat Nlerp (const Quat &q1, const Quat &q2, Float t);
  inline static Float Dot (const Quat &q1, const Quat &q2);

  bool operator== (const Quat &q) const;
  bool operator!= (const Quat &q) const;
};

Quat operator* (const Quat &q1, const Quat &q2);

/*
=============================================
Vec inline functions
=============================================*/

//Scalar conversions
template <class S>
template <class T>
inline TVec2<S>::TVec2 (const TVec2<T> &v) {
  x=(S)v.x; y=(S)v.y;
}

template <class S>
template <class T>
inline TVec3<S>::TVec3 (const TVec3<T> &v) {
  x=(S)v.x; y=(S)v.y; z=(S)v.z;
}

template <class S>
template <class T>
inline TVec4<S>::TVec4 (const TVec4<T> &v) {
  x=(S)v.x; y=(S)v.y; z=(S)v.z; w=(S)v.w;
}


//Swizzle
template <class S>
inline TVec2<S> TVec2<S>::yx () const
  { return TVec2<S> (y,x); }


//Component conversions
template <class S>
inline TVec3<S> TVec2<S>::xy (S z) const
  { return TVec3<S> (x,y,z); }

template <class S>
inline TVec4<S> TVec2<S>::xy (S z, S w) const
  { return TVec4<S> (x,y,z,w); }

template <class S>
inline TVec4<S> TVec3<S>::xyz (S w) const
  { return TVec4<S> (x,y,z,w); }


template <class S>
inline TVec2<S> TVec3<S>::xy () const
  { return TVec2<S> (x,y); }

template <class S>
inline TVec2<S> TVec3<S>::yz () const
  { return TVec2<S> (y,z); }

template <class S>
inline TVec2<S> TVec3<S>::xz () const
  { return TVec2<S> (x,z); }


template <class S>
inline TVec2<S> TVec4<S>::xy () const
  { return TVec2<S> (x,y); }

template <class S>
inline TVec3<S> TVec4<S>::xyz () const
  { return TVec3<S>(x,y,z); }


//Default constructor
template <class S>
inline TVec2<S>::TVec2 ()
  { x=(S)0; y=(S)0; }

template <class S>
inline TVec3<S>::TVec3 ()
  { x=(S)0; y=(S)0; z=(S)0; }

template <class S>
inline TVec4<S>::TVec4 ()
  { x=(S)0; y=(S)0; z=(S)0; w=(S)0; }


//All-members constructor
template <class S>
inline TVec2<S>::TVec2 (S xy)
  { x=xy; y=xy; }

template <class S>
inline TVec3<S>::TVec3 (S xyz)
  { x=xyz; y=xyz; z=xyz; }

template <class S>
inline TVec4<S>::TVec4 (S xyzw)
  { x=xyzw; y=xyzw; z=xyzw; w=xyzw; }


//Separate members constructor
template <class S>
inline TVec2<S>::TVec2 (S x0, S y0)
  { x=x0; y=y0; }

template <class S>
inline TVec3<S>::TVec3 (S x0, S y0, S z0)
  { x=x0; y=y0; z=z0; }

template <class S>
inline TVec4<S>::TVec4 (S x0, S y0, S z0, S w0)
  { x=x0; y=y0; z=z0; w=w0; }


//Partial vector constructor
template <class S>
inline TVec3<S>::TVec3(const TVec2<S> &v, S z0)
  { x=v.x; y=v.y; z=z0; }

template <class S>
inline TVec4<S>::TVec4(const TVec2<S> &v, S z0, S w0)
  { x=v.x; y=v.y; z=z0; w=w0; }

template <class S>
inline TVec4<S>::TVec4(const TVec3<S> &v, S w0)
  { x=v.x; y=v.y; z=v.z; w=w0; }


//Separate members assignment
template <class S>
inline TVec2<S>& TVec2<S>::set (S x0, S y0)
  { x=x0; y=y0; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::set (S x0, S y0, S z0)
  { x=x0; y=y0; z=z0; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::set (S x0, S y0, S z0, S w0)
  { x=x0; y=y0; z=z0; w=w0; return *this; }


template <class S>
inline TVec2<S>& TVec2<S>::operator= (const TVec2<S> &v)
  { x=v.x; y=v.y; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::operator= (const TVec3<S> &v)
  { x=v.x; y=v.y; z=v.z; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::operator= (const TVec4<S> &v)
  { x=v.x; y=v.y; z=v.z; w=v.w; return *this; }


template <class S>
inline bool TVec2<S>::operator== (const TVec2<S> &v) const
  { return x==v.x && y==v.y; }

template <class S>
inline bool TVec3<S>::operator== (const TVec3<S> &v) const
  { return x==v.x && y==v.y && z==v.z; }

template <class S>
inline bool TVec4<S>::operator== (const TVec4<S> &v) const
  { return x==v.x && y==v.y && z==v.z && w==v.w; }


template <class S>
inline bool TVec2<S>::operator!= (const TVec2<S> &v) const
  { return x!=v.x || y!=v.y; }

template <class S>
inline bool TVec3<S>::operator!= (const TVec3<S> &v) const
  { return x!=v.x || y!=v.y || z!=v.z; }

template <class S>
inline bool TVec4<S>::operator!= (const TVec4<S> &v) const
  { return x!=v.x || y!=v.y || z!=v.z || w!=v.w; }


template <class S>
inline TVec2<S>& TVec2<S>::operator+= (const TVec2<S> &v)
  { x+=v.x; y+=v.y; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::operator+= (const TVec3<S> &v)
  { x+=v.x; y+=v.y; z+=v.z; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::operator+= (const TVec4<S> &v)
  { x+=v.x; y+=v.y; z+=v.z; w+=v.w; return *this; }


template <class S>
inline TVec2<S>& TVec2<S>::operator-= (const TVec2<S> &v)
  { x-=v.x; y-=v.y; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::operator-= (const TVec3<S> &v)
  { x-=v.x; y-=v.y; z-=v.z; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::operator-= (const TVec4<S> &v)
  { x-=v.x; y-=v.y; z-=v.z; w-=v.w; return *this; }


template <class S>
inline TVec2<S>& TVec2<S>::operator*= (Float k)
  { x*=k; y*=k; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::operator*= (Float k)
  { x*=k; y*=k; z*=k; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::operator*= (Float k)
  { x*=k; y*=k; z*=k; w*=k; return *this; }


template <class S>
inline TVec2<S>& TVec2<S>::operator/= (Float k)
  { x/=k; y/=k; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::operator/= (Float k)
  { x/=k; y/=k; z/=k; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::operator/= (Float k)
  { x/=k; y/=k; z/=k; w/=k; return *this; }


template <class S>
inline TVec2<S> TVec2<S>::operator+ (const TVec2<S> &v) const
  { return TVec2<S> (x+v.x, y+v.y); }

template <class S>
inline TVec3<S> TVec3<S>::operator+ (const TVec3<S> &v) const
  { return TVec3<S> (x+v.x, y+v.y, z+v.z); }

template <class S>
inline TVec4<S> TVec4<S>::operator+ (const TVec4<S> &v) const
  { return TVec4<S> (x+v.x, y+v.y, z+v.z, w+v.w); }


template <class S>
inline TVec2<S> TVec2<S>::operator- (const TVec2<S> &v) const
  { return TVec2<S> (x-v.x, y-v.y); }

template <class S>
inline TVec3<S> TVec3<S>::operator- (const TVec3<S> &v) const
  { return TVec3<S> (x-v.x, y-v.y, z-v.z); }

template <class S>
inline TVec4<S> TVec4<S>::operator- (const TVec4<S> &v) const
  { return TVec4<S> (x-v.x, y-v.y, z-v.z, w-v.w); }


template <class S>
inline TVec2<S> TVec2<S>::operator* (const TVec2<S> &v) const
  { return TVec2<S> (x*v.x, y*v.y); }

template <class S>
inline TVec3<S> TVec3<S>::operator* (const TVec3<S> &v) const
  { return TVec3<S> (x*v.x, y*v.y, z*v.z); }

template <class S>
inline TVec4<S> TVec4<S>::operator* (const TVec4<S> &v) const
  { return TVec4<S> (x*v.x, y*v.y, z*v.z, w*v.w); }


template <class S>
inline TVec2<S> TVec2<S>::operator/ (const TVec2<S> &v) const
  { return TVec2<S> (x/v.x, y/v.y); }

template <class S>
inline TVec3<S> TVec3<S>::operator/ (const TVec3<S> &v) const
  { return TVec3<S> (x/v.x, y/v.y, z/v.z); }

template <class S>
inline TVec4<S> TVec4<S>::operator/ (const TVec4<S> &v) const
  { return TVec4<S> (x/v.x, y/v.y, z/v.z, w/v.w); }


template <class S>
inline TVec2<S> TVec2<S>::operator* (Float k) const
  { return TVec2<S> (x*k, y*k); }

template <class S>
inline TVec3<S> TVec3<S>::operator* (Float k) const
  { return TVec3<S> (x*k, y*k, z*k); }

template <class S>
inline TVec4<S> TVec4<S>::operator* (Float k) const
  { return TVec4<S> (x*k, y*k, z*k, w*k); }


template <class S>
inline TVec2<S> TVec2<S>::operator/ (Float k) const
  { return TVec2<S> (x/k, y/k); }

template <class S>
inline TVec3<S> TVec3<S>::operator/ (Float k) const
  { return TVec3<S> (x/k, y/k, z/k); }

template <class S>
inline TVec4<S> TVec4<S>::operator/ (Float k) const
  { return TVec4<S> (x/k, y/k, z/k, w/k); }


template <class S>
inline Float TVec2<S>::norm () const
  { return SQRT (x*x + y*y); }

template <class S>
inline Float TVec3<S>::norm () const
  { return SQRT (x*x + y*y + z*z); }

template <class S>
inline Float TVec4<S>::norm () const
  { return SQRT (x*x + y*y + z*z + w*w); }


template <class S>
inline Float TVec2<S>::normSq () const
  { return x*x + y*y; }

template <class S>
inline Float TVec3<S>::normSq () const
  { return x*x + y*y + z*z; }

template <class S>
inline Float TVec4<S>::normSq () const
  { return x*x + y*y + z*z + w*w; }


template <class S>
inline TVec2<S>& TVec2<S>::normalize ()
  { Float k=1.0f/norm(); x = k*x; y = k*y; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::normalize ()
  { Float k=1.0f/norm(); x = k*x; y = k*y; z = k*z; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::normalize ()
  { Float k=1.0f/norm(); x = k*x; y = k*y; z = k*z; w = k*w; return *this; }


template <class S>
inline TVec2<S>& TVec2<S>::offset (Float ox, Float oy)
  { x+=ox; y+=oy; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::offset (Float ox, Float oy, Float oz)
  { x+=ox; y+=oy; z+=oz; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::offset (Float ox, Float oy, Float oz, Float ow)
  { x+=ox; y+=oy; z+=oz; w+=ow; return *this; }


template <class S>
inline TVec2<S>& TVec2<S>::offsetV (const TVec2<S> &v, Float d)
  { Float K = d/v.norm(); x += v.x*K; y += v.y*K; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::offsetV (const TVec3<S> &v, Float d)
  { Float K = d/v.norm(); x += v.x*K; y += v.y*K; z += v.z*K; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::offsetV (const TVec4<S> &v, Float d)
  { Float K = d/v.norm(); x += v.x*K; y += v.y*K; z += v.z*K; w += v.w*K; return *this; }


template <class S>
inline TVec2<S>& TVec2<S>::offsetVN (const TVec2<S> &v, Float d)
  { x += v.x*d; y += v.y*d; return *this; }

template <class S>
inline TVec3<S>& TVec3<S>::offsetVN (const TVec3<S> &v, Float d)
  { x += v.x*d; y += v.y*d; z += v.z*d; return *this; }

template <class S>
inline TVec4<S>& TVec4<S>::offsetVN (const TVec4<S> &v, Float d)
  { x += v.x*d; y += v.y*d; z += v.z*d; w += v.w*d; return *this; }


template <class S>
inline TVec2<S> TVec2<S>::reverse () const
  { return TVec2<S> (-x, -y); }

template <class S>
inline TVec3<S> TVec3<S>::reverse () const
  { return TVec3<S> (-x, -y, -z); }

template <class S>
inline TVec4<S> TVec4<S>::reverse () const
  { return TVec4<S> (-x, -y, -z, -w); }


template <class S>
inline Float TVec<S>::Dot (const TVec2<S> &v1, const TVec2<S> &v2)
  { return v1.x*v2.x + v1.y*v2.y; }

template <class S>
inline Float TVec<S>::Dot (const TVec3<S> &v1, const TVec3<S> &v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }

template <class S>
inline Float TVec<S>::Dot (const TVec4<S> &v1, const TVec4<S> &v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }


template <class S>
inline Float TVec<S>::Angle (const TVec2<S> &v1, const TVec2<S> &v2)
  { return ACOS (TVec<S>::Dot (v1,v2) / (v1.norm() * v2.norm())); }

template <class S>
inline Float TVec<S>::Angle (const TVec3<S> &v1, const TVec3<S> &v2)
  { return ACOS (TVec<S>::Dot (v1,v2) / (v1.norm() * v2.norm())); }

template <class S>
inline Float TVec<S>::Angle (const TVec4<S> &v1, const TVec4<S> &v2)
  { return ACOS (TVec<S>::Dot (v1,v2) / (v1.norm() * v2.norm())); }


template <class S>
inline Float TVec<S>::AngleN (const TVec2<S> &v1, const TVec2<S> &v2)
  { return ACOS (TVec<S>::Dot (v1,v2)); }

template <class S>
inline Float TVec<S>::AngleN (const TVec3<S> &v1, const TVec3<S> &v2)
  { return ACOS (TVec<S>::Dot (v1,v2)); }

template <class S>
inline Float TVec<S>::AngleN (const TVec4<S> &v1, const TVec4<S> &v2)
  { return ACOS (TVec<S>::Dot (v1,v2)); }


template <class S>
inline Float TVec<S>::Cross (const TVec2<S> &v1, const TVec2<S> &v2) {
  return v1.x*v2.y - v2.x*v1.y;
}

template <class S>
inline TVec3<S> TVec<S>::Cross (const TVec3<S> &v1, const TVec3<S> &v2) {
  return TVec3<S> (v1.y*v2.z - v2.y*v1.z,
                  v2.x*v1.z - v1.x*v2.z,
                  v1.x*v2.y - v2.x*v1.y);
}

template <class S>
template <class V> inline V TVec<S>::Lerp (const V &v1, const V &v2, Float t) {
  return v1 * (1.0f - t) + v2 * t;
}


template <class S>
inline TVec2<S> TVec<S>::Floor (const TVec2<S> &v) {
  return TVec2<S>( FLOOR(v.x), FLOOR(v.y) );
}

template <class S>
inline TVec3<S> TVec<S>::Floor (const TVec3<S> &v) {
  return TVec3<S>( FLOOR(v.x), FLOOR(v.y), FLOOR(v.z) );
}

template <class S>
inline TVec4<S> TVec<S>::Floor (const TVec4<S> &v) {
  return TVec4<S>( FLOOR(v.x), FLOOR(v.y), FLOOR(v.z), FLOOR(v.w) );
}


template <class S>
inline TVec2<S> TVec<S>::Ceil (const TVec2<S> &v) {
  return TVec2<S>( CEIL(v.x), CEIL(v.y) );
}

template <class S>
inline TVec3<S> TVec<S>::Ceil (const TVec3<S> &v) {
  return TVec3<S>( CEIL(v.x), CEIL(v.y), CEIL(v.z) );
}

template <class S>
inline TVec4<S> TVec<S>::Ceil (const TVec4<S> &v) {
  return TVec4<S>( CEIL(v.x), CEIL(v.y), CEIL(v.z), CEIL(v.w) );
}


template <class S>
inline TVec2<S> TVec<S>::Fract (const TVec2<S> &v) {
  return TVec2<S>( v.x - FLOOR(v.x), v.y - FLOOR(v.y) );
}

template <class S>
inline TVec3<S> TVec<S>::Fract (const TVec3<S> &v) {
  return TVec3<S>( v.x - FLOOR(v.x), v.y - FLOOR(v.y), v.z - FLOOR(v.z) );
}

template <class S>
inline TVec4<S> TVec<S>::Fract (const TVec4<S> &v) {
  return TVec4<S>( v.x - FLOOR(v.x), v.y - FLOOR(v.y), v.z - FLOOR(v.z), v.w - FLOOR(v.w) );
}


template <class S>
inline TVec2<S> TVec<S>::Min (const TVec2<S> &v1, const TVec2<S> &v2) {
  return TVec2<S>(
    v1.x < v2.x ? v1.x : v2.x,
    v1.y < v2.y ? v1.y : v2.y );
}

template <class S>
inline TVec3<S> TVec<S>::Min (const TVec3<S> &v1, const TVec3<S> &v2) {
  return TVec3<S>(
    v1.x < v2.x ? v1.x : v2.x,
    v1.y < v2.y ? v1.y : v2.y,
    v1.z < v2.z ? v1.z : v2.z );
}

template <class S>
inline TVec4<S> TVec<S>::Min (const TVec4<S> &v1, const TVec4<S> &v2) {
  return TVec4<S>(
    v1.x < v2.x ? v1.x : v2.x,
    v1.y < v2.y ? v1.y : v2.y,
    v1.z < v2.z ? v1.z : v2.z,
    v1.w < v2.w ? v1.w : v2.w );
}


template <class S>
inline TVec2<S> TVec<S>::Max (const TVec2<S> &v1, const TVec2<S> &v2) {
  return TVec2<S>(
    v1.x > v2.x ? v1.x : v2.x,
    v1.y > v2.y ? v1.y : v2.y );
}

template <class S>
inline TVec3<S> TVec<S>::Max (const TVec3<S> &v1, const TVec3<S> &v2) {
  return TVec3<S>(
    v1.x > v2.x ? v1.x : v2.x,
    v1.y > v2.y ? v1.y : v2.y,
    v1.z > v2.z ? v1.z : v2.z );
}

template <class S>
inline TVec4<S> TVec<S>::Max (const TVec4<S> &v1, const TVec4<S> &v2) {
  return TVec4<S>(
    v1.x > v2.x ? v1.x : v2.x,
    v1.y > v2.y ? v1.y : v2.y,
    v1.z > v2.z ? v1.z : v2.z,
    v1.w > v2.w ? v1.w : v2.w );
}

/*
=============================================
Quat inline functions
=============================================*/

inline Quat::Quat ()
  { x=0.0f; y=0.0f; z=0.0f; w=1.0f; }

inline Quat::Quat (Float xx, Float yy, Float zz, Float ww)
  { x=xx; y=yy; z=zz; w=ww; }

inline void Quat::set (Float xx, Float yy, Float zz, Float ww)
  { x=xx; y=yy; z=zz; w=ww; }

inline void Quat::setIdentity ()
  { x=0.0f; y=0.0f; z=0.0f; w=1.0f; }

inline Float Quat::norm () const
  { return SQRT (x*x + y*y + z*z + w*w); }

inline Float Quat::normSq () const
  { return x*x + y*y + z*z + w*w; }

inline Quat& Quat::normalize ()
  { Float n=norm(); x/=n; y/=n; z/=n; w/=n; return *this; }

inline Float Quat::Dot (const Quat &q1, const Quat &q2)
  { return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w; }

inline bool Quat::operator== (const Quat &q) const
  { return x==q.x && y==q.y && z==q.z && w==q.w; }

inline bool Quat::operator!= (const Quat &q) const
  { return x!=q.x || y!=q.y || z!=q.z || w!=q.w; }

#endif//__RVGVECTORS_H
