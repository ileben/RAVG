#ifndef __RVGMATRIXSTACK_H
#define __RVGMATRIXSTACK_H 1

class MatrixStack
{
private:

  std::vector< Matrix4x4 > stack;

public:

  MatrixStack();

  void push();
  void pop();
  void clear();
  Matrix4x4& top();

  void load (const Matrix4x4 &mat);
  void translate (Float x, Float y, Float z);
  void scale (Float x, Float y, Float z);
  void rotate (Float x, Float y, Float z, Float radang );
  void identity ();
};

#endif//__RVGMATRIXSTACK_H