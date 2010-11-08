#ifndef RVGVERTEXBUFFER_H
#define RVGVERTEXBUFFER_H 1

class Vertex
{
public:
  Vec3 coord;
  Vec2 texcoord;
  Vec3 normal;
};

class VertexBuffer
{
public:
  bool onGpu;
  GLuint gpuId;
  std::vector< Vertex > verts;

  VertexBuffer();
  void toGpu();
  void render (Shader *shader, GLenum mode);
};

#endif
