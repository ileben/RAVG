#include "rvgDefs.h"
#include "rvgVectors.h"
#include "rvgMatrix.h"

/*-----------------------------------------
 * Matrix4x4 operations
 *-----------------------------------------*/

Matrix4x4::Matrix4x4 ()
{
  setIdentity();
}

Matrix4x4::Matrix4x4 (const SquareMatrix<4> &mat)
{
  //ISO C++ forbids assignment of arrays
  for (int x=0; x<4; ++x)
    for (int y=0; y<4; ++y)
      m[x][y] = mat.m[x][y];
}

void Matrix4x4::setIdentity ()
{
  set (1, 0, 0, 0,
       0, 1, 0, 0,
       0, 0, 1, 0,
       0, 0, 0, 1);
}

void Matrix4x4::setRotationX (Float radang)
{
  Float cosa = COS (radang);
  Float sina = SIN (radang);
  set (1,    0,     0, 0,
       0, cosa, -sina, 0,
       0, sina,  cosa, 0,
       0,    0,     0, 1);
}

void Matrix4x4::setRotationY (Float radang)
{
  Float cosa = COS (radang);
  Float sina = SIN (radang);
  set (cosa,   0, sina, 0,
          0,   1,    0, 0,
      -sina,   0, cosa, 0,
          0,   0,    0, 1);
}

void Matrix4x4::setRotationZ (Float radang)
{
  Float cosa = COS (radang);
  Float sina = SIN (radang);
  set (cosa, -sina, 0, 0,
       sina,  cosa, 0, 0,
          0,     0, 1, 0,
          0,     0, 0, 1);
}

void Matrix4x4::setTranslation (Float x, Float y, Float z)
{
  set (1, 0, 0, x,
       0, 1, 0, y,
       0, 0, 1, z,
       0, 0, 0, 1);
}

void Matrix4x4::setTranslation (const Vec3 &v)
{
  set (1, 0, 0, v.x,
       0, 1, 0, v.y,
       0, 0, 1, v.z,
       0, 0, 0, 1);
}

void Matrix4x4::setScale (Float x, Float y, Float z)
{
  set (x, 0, 0, 0,
       0, y, 0, 0,
       0, 0, z, 0,
       0, 0, 0, 1);
}

void Matrix4x4::setScale (const Vec3 &v)
{
  set (v.x, 0,   0,   0,
       0,   v.y, 0,   0,
       0,   0,   v.z, 0,
       0,   0,   0,   1);
}

void Matrix4x4::setScale (Float k)
{
  set (k, 0, 0, 0,
       0, k, 0, 0,
       0, 0, k, 0,
       0, 0, 0, 1);
}


/*
----------------------------------------------
Left-handed projection into [-1, 1] ^ 3 cube
(looking down the positive Z axis)
----------------------------------------------*/

void Matrix4x4::setOrthoLH (Float l, Float r, Float b, Float t, Float n, Float f)
{
  Float X1 = 2 / (r - l);
  Float X2 = - (r + l) / (r - l);
  
  Float Y1 = 2 / (t - b);
  Float Y2 = - (t + b) / (t - b);
  
  Float Z1 = 2 / (f - n);
  Float Z2 = - (f + n) / (f - n);

  set (X1,  0,  0, X2,
        0, Y1,  0, Y2,
        0,  0, Z1, Z2,
        0,  0,  0,  1);
}

/*
----------------------------------------------
Left-handed projection into [-1, 1] ^ 3 cube
(looking down the positive Z axis)
----------------------------------------------*/

void Matrix4x4::setPerspectiveFovLH (Float fovY, Float aspect, Float zNear, Float zFar)
{
  Float t = 1.0f / TAN( fovY / 2 );
  Float A = t / aspect;
  Float B = t;
  Float C = (zFar + zNear) / (zFar - zNear);
  Float D = -(2 * zFar * zNear) / (zFar - zNear);
  Float E = 1;

  set (A, 0, 0, 0,
       0, B, 0, 0,
       0, 0, C, D,
       0, 0, E, 0);
}

/*
----------------------------------------------
Right-handed projection into [-1, 1] ^ 3 cube
(looking down the negative Z axis)
----------------------------------------------*/

void Matrix4x4::setPerspectiveFovRH (Float fovY, Float aspect, Float zNear, Float zFar)
{
  Float t = 1.0f / TAN( fovY / 2 );
  Float A = t / aspect;
  Float B = t;
  Float C = (zFar + zNear) / (zNear - zFar);
  Float D = (2 * zFar * zNear) / (zNear - zFar);
  Float E = -1;
  
  set (A, 0, 0, 0,
       0, B, 0, 0,
       0, 0, C, D,
       0, 0, E, 0);
}

Vec4 Matrix4x4::getColumn (int col) const
{
  return Vec4 (m[col][0],
               m[col][1],
               m[col][2],
               m[col][3]);
}

Vec4 Matrix4x4::getRow (int row) const
{
  return Vec4( m[0][row], m[1][row], m[2][row], m[3][row] );
}

void Matrix4x4::setColumn (int col, const Vec4 &v)
{
  m[col][0] = v.x;
  m[col][1] = v.y;
  m[col][2] = v.z;
  m[col][3] = v.w;
}

void Matrix4x4::setColumn (int col, const Vec3 &v)
{
  m[col][0] = v.x;
  m[col][1] = v.y;
  m[col][2] = v.z;
}

void Matrix4x4::setRow (int row, const Vec4 &v)
{
  m[0][row] = v.x;
  m[1][row] = v.y;
  m[2][row] = v.z;
  m[3][row] = v.w;
}

void Matrix4x4::setRow (int row, const Vec3 &v)
{
  m[0][row] = v.x;
  m[1][row] = v.y;
  m[2][row] = v.z;
}

void Matrix4x4::set (const Vec4 &col1,
                     const Vec4 &col2,
                     const Vec4 &col3,
                     const Vec4 &col4)
{
  set (col1.x, col2.x, col3.x, col4.x,
       col1.y, col2.y, col3.y, col4.y,
       col1.z, col2.z, col3.z, col4.z,
       col1.w, col2.w, col3.w, col4.w);
}

void Matrix4x4::fromAxisAngle (Float x, Float y, Float z, Float radang)
{
  Float c = COS (radang);
  Float s = SIN (radang);
  Float t = 1 - c;
  
  set (t*x*x + c,    t*x*y - s*z,  t*x*z + s*y,  0,
       t*x*y + s*z,  t*y*y + c,    t*y*z - s*x,  0,
       t*x*z - s*y,  t*y*z + s*x,  t*z*z + c,    0,
       0,            0,            0,            1);
}

void Matrix4x4::fromAxisAngle (const Vec3 &axis, Float radang)
{
  fromAxisAngle (axis.x, axis.y, axis.z, radang);
}

void Matrix4x4::fromQuat (Float x, Float y, Float z, Float w)
{
  
  set (1-2*y*y-2*z*z,   2*x*y-2*w*z,   2*x*z+2*w*y,  0,
       2*x*y+2*w*z,   1-2*x*x-2*z*z,   2*y*z-2*w*x,  0,
       2*x*z-2*w*y,     2*y*z+2*w*x, 1-2*x*x-2*y*y,  0,
       0,               0,             0,            1);
  /*
  set (1-2*y*y-2*z*z,   2*x*y+2*w*z,   2*x*z-2*w*y,  0,
       2*x*y-2*w*z,   1-2*x*x-2*z*z,   2*y*z+2*w*x,  0,
       2*x*z+2*w*y,     2*y*z-2*w*x, 1-2*x*x-2*y*y,  0,
       0,               0,             0,            1);
 */
}

void Matrix4x4::fromQuat (const Quat &q)
{
  fromQuat (q.x, q.y, q.z, q.w);
}

Quat Matrix4x4::toQuat ()
{
  Quat q;
  q.fromMatrix (*this);
  return q;
}

Vec3 Matrix4x4::transformPoint (const Vec3 &v) const
{
  Vec3 out;
  out.x = m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0];// * 1.0;
  out.y = m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1];// * 1.0;
  out.z = m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2];// * 1.0;
  return out;
}

Vec4 Matrix4x4::transformPoint (const Vec4 &v) const
{
  Vec4 out;
  out.x = m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0] * v.w;
  out.y = m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1] * v.w;
  out.z = m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2] * v.w;
  out.w = m[0][3]*v.x + m[1][3]*v.y + m[2][3]*v.z + m[3][3] * v.w;
  return out;
}

Vec3 Matrix4x4::transformVec (const Vec3 &v) const
{
  Vec3 out;
  out.x = m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z;// + m[3][0] * 0.0;
  out.y = m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z;// + m[3][1] * 0.0;
  out.z = m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z;// + m[3][2] * 0.0;
  return out;
}

Matrix4x4& Matrix4x4::operator*= (const Matrix4x4 &R)
{
  Matrix4x4 temp;
  for (int r=0; r<4; ++r) {
    for (int c=0; c<4; ++c) {
      temp.m[c][r] = (m[0][r]*R.m[c][0] +
                      m[1][r]*R.m[c][1] +
                      m[2][r]*R.m[c][2] +
                      m[3][r]*R.m[c][3]); }}
  *this = temp;
  return *this;
}

Matrix4x4 Matrix4x4::operator* (const Matrix4x4 &R) const
{
  Matrix4x4 out;
  for (int r=0; r<4; ++r) {
    for (int c=0; c<4; ++c) {
      out.m[c][r] = (m[0][r]*R.m[c][0] +
                     m[1][r]*R.m[c][1] +
                     m[2][r]*R.m[c][2] +
                     m[3][r]*R.m[c][3]); }}
  return out;
}

/*
This would be a way to optimize 4x4 matrix determinant / inverse
if ever needed. It's an expansion of the generic implementation
in SquareMatrix. At the moment can't be bothered to figure out the
whole thing, since affineInverse is being used in pretty much every
case so far.

Float Matrix4x4::determinant () const
{
  //minor00 = (determinant of submatrix with 0-col 0-row dropped)
  m[1][1] * (+1) * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) +
  m[2][1] * (-1) * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) +
  m[3][1] * (+1) * (m[1][3] * m[2][3] - m[1][3] * m[2][2]);

  //minor10 = (determinant of submatrix with 1-col 0-row dropped)
  //m[0][1] * (+1) * ...
  //m[2][1] * (-1) * ...
  //m[3][1] * (+1) * ...

  //minor20 = (determinant of submatrix with 2-col 0-row dropped)
  //m[0][1] * (+1) * ...
  //m[1][1] * (-1) * ...
  //m[3][1] * (+1) * ...

  //minor30 = (determinant of submatrix with 3-col 0-row dropped)
  //m[0][1] * (+1) * ...
  //m[1][1] * (-1) * ...
  //m[2][1] * (+1) * ...

  //det = (m[0][0] * (+1) * minor00 +
  //       m[1][0] * (-1) * minor10 +
  //       m[2][0] * (+1) * minor20 );
}

Matrix4x4 Matrix4x4::inverse () const
{
}
*/

/*
An affine transformation matrix equals to the product of
its translation and rotation parts in that order (T * R).
Since inverse (A*B) = inverse (B) * inverse (A)
therefore inverse (affine) = inverse (R) * inverse (T).
Note that since rotation part is orthonormal its inverse
equals its transpose, while translation part is inverted
by negating the offset.

Rot part transposed  Trans part negated
  |m00 m01 m02 0|     | 1  0  0 -m30|
  |m10 m11 m12 0|     | 0  1  0 -m31|
  |m20 m21 m22 0|  *  | 0  0  1 -m32|
  | 0   0   0  1|     | 0  0  0  1  |
*/

Matrix4x4 Matrix4x4::affineInverse () const
{
  Matrix4x4 out;
  out.set
    (m[0][0], m[0][1], m[0][2], -m[0][0]*m[3][0] - m[0][1]*m[3][1] - m[0][2]*m[3][2],
     m[1][0], m[1][1], m[1][2], -m[1][0]*m[3][0] - m[1][1]*m[3][1] - m[1][2]*m[3][2],
     m[2][0], m[2][1], m[2][2], -m[2][0]*m[3][0] - m[2][1]*m[3][1] - m[2][2]*m[3][2],
     0,       0,       0,       1);
  
  return out;
}

/*
Normalizes the rotation part columns.
*/

Matrix4x4& Matrix4x4::affineNormalize ()
{
  Float norm0inv = 1.0f / SQRT (m[0][0]*m[0][0] + m[0][1]*m[0][1] + m[0][2]*m[0][2]);
  Float norm1inv = 1.0f / SQRT (m[1][0]*m[1][0] + m[1][1]*m[1][1] + m[1][2]*m[1][2]);
  Float norm2inv = 1.0f / SQRT (m[2][0]*m[2][0] + m[2][1]*m[2][1] + m[2][2]*m[2][2]);
  
  m[0][0] *= norm0inv; m[1][0] *= norm1inv; m[2][0] *= norm2inv;
  m[0][1] *= norm0inv; m[1][1] *= norm1inv; m[2][1] *= norm2inv;
  m[0][2] *= norm0inv; m[1][2] *= norm1inv; m[2][2] *= norm2inv;

  return *this;
}
