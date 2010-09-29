#ifndef RVGIMAGEENCODER_H
#define RVGIMAGEENCODER_H 1

class Image;

class ImageEncoder
{
public:

  ImageEncoder();

  //Emulated uniform and varying variables
  int *ptrObjects;
  int *ptrInfo;
  int *ptrGrid;
  float *ptrStream;

  vec2 gridOrigin;
  ivec2 gridSize;
  vec2 cellSize;

  int objectId;
  vec2 objMin;
  vec2 objMax;
  vec4 color;

  vec2 line0;
  vec2 line1;
  ivec2 lineMin;

  vec2 quad0;
  vec2 quad1;
  vec2 quad2;
  ivec2 quadMin;

  ivec2 gridCoord;

  //Emulated GLSL functions
  int atomicAdd (int *ptr, int value);
  int atomicExchange (int *ptr, int value);

  //Emulated vertex / geometry shader
  virtual void encodeInit () = 0;
  virtual void encodeInitObject () = 0;
  virtual void encodeLine () = 0;
  virtual void encodeQuad () = 0;
  virtual void encodeObject () = 0;
  virtual void encodeSort () = 0;
};

class ImageEncoderAux : public ImageEncoder
{
  //Emulated shader utility functions
  int addLine (const Vec2 &l0, const Vec2 &l1, int *ptrObjCell);
  int addQuad (const Vec2 &q0, const Vec2 &q1, const Vec2 &q2, int *ptrObjCell);
  int addObject (int objectId, int occlusion, const Vec4 &color, int lastSegmentOffset, int *ptrCell);

  //Emulated shader main() functions
  void frag_encodeInit ();
  void frag_encodeInitObject ();
  void frag_encodeLine ();
  void frag_encodeQuad ();
  void frag_encodeObject ();
  void frag_encodeSort ();

  //Emulated vertex / geometry shader
  virtual void encodeInit ();
  virtual void encodeInitObject ();
  virtual void encodeLine ();
  virtual void encodeQuad ();
  virtual void encodeObject ();
  virtual void encodeSort ();
};

class ImageEncoderPivot : public ImageEncoder
{
  //Emulated shader utility functions
  int addLine (const Vec2 &l0, const Vec2 &l1, int *ptrObjCell);
  int addQuad (const Vec2 &q0, const Vec2 &q1, const Vec2 &q2, int *ptrObjCell);
  int addObject (int objectId, int wind, const Vec4 &color, int lastSegmentOffset, int *ptrCell);

  //Emulated shader main() functions
  void frag_encodeInit ();
  void frag_encodeInitObject ();
  void frag_encodeLine ();
  void frag_encodeQuad ();
  void frag_encodeObject ();
  void frag_encodeSort ();

  //Emulated vertex / geometry shader
  virtual void encodeInit ();
  virtual void encodeInitObject ();
  virtual void encodeLine ();
  virtual void encodeQuad ();
  virtual void encodeObject ();
  virtual void encodeSort ();
};

#endif//RVGIMAGEENCODER_H
