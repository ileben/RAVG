#include "rvgDefs.h"
#include "rvgVectors.h"
#include "rvgMatrix.h"
#include "rvgMatrixStack.h"

MatrixStack::MatrixStack()
{
  stack.push_back( Matrix4x4() );
}

void MatrixStack::push()
{
  stack.push_back( stack.back() );
}

void MatrixStack::pop()
{
  if (stack.size() > 1)
    stack.pop_back();
}

Matrix4x4& MatrixStack::top()
{
  return stack.back();
}

void MatrixStack::clear()
{
  stack.clear();
  stack.push_back( Matrix4x4() );
}

void MatrixStack::translate (Float x, Float y, Float z)
{
  Matrix4x4 m;
  m.setTranslation( x, y, z );
  stack.back() = stack.back() * m;
}

void MatrixStack::scale (Float x, Float y, Float z)
{
  Matrix4x4 m;
  m.setScale( x, y, z );
  stack.back() = stack.back() * m;
}

void MatrixStack::rotate( Float x, Float y, Float z, Float radang )
{
  Matrix4x4 m;
  m.fromAxisAngle( x, y, z, radang );
  stack.back() = stack.back() * m;
}

void MatrixStack::identity ()
{
  stack.back().setIdentity();
}

void MatrixStack::load (const Matrix4x4 &mat)
{
  stack.back() = mat;
}
