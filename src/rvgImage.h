#ifndef RVGIMAGE_H
#define RVGIMAGE_H 1

///////////////////////////////////////////////////////////////////
// Forward declarations

class Contour;
class Object;
class ImageEncoderCpu;
class ImageEncoderGpu;
class RendererRandom;
class RendererClassic;

///////////////////////////////////////////////////////////////////

namespace SegSpace {
  enum Enum {
    Absolute = (1 << 7),
    Relative = (1 << 7)
  };
};

namespace SegType {
  enum Enum {

    Close        = 0,
    MoveTo       = 1,
    LineTo       = 2,
    QuadTo       = 3,
    CubicTo      = 4,

    MoveToAbs    = (SegType::MoveTo  | SegSpace::Absolute),
    LineToAbs    = (SegType::LineTo  | SegSpace::Absolute),
    QuadToAbs    = (SegType::QuadTo  | SegSpace::Absolute),
    CubicToAbs   = (SegType::CubicTo | SegSpace::Absolute),

    MoveToRel    = (SegType::MoveTo  | SegSpace::Relative),
    LineToRel    = (SegType::LineTo  | SegSpace::Relative),
    QuadToRel    = (SegType::QuadTo  | SegSpace::Relative),
    CubicToRel   = (SegType::CubicTo | SegSpace::Relative)
  };
};

struct Line
{
  Vec2 p0;
  Vec2 p1;

  Line() {};
  Line( Float x1, Float y1, Float x2, Float y2 ) {
    p0.set( x1,y1 ); p1.set( x2,y2 );
  }
};

struct Quad
{
  Vec2 p0;
  Vec2 p1;
  Vec2 p2;

  Quad() {}

  Quad( const Vec2 &pp0, const Vec2 &pp1, const Vec2 &pp2 ) {
    p0 = pp0; p1 = pp1; p2 = pp2;
  }
  Quad( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3 ) {
    p0.set( x1,y1 ); p1.set( x2,y2 ); p2.set( x3,y3 );
  }
};

struct Cubic
{
  Vec2 p0;
  Vec2 p1;
  Vec2 p2;
  Vec2 p3;

  Cubic() {};
  Cubic( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3, Float x4, Float y4 ) {
    p0.set( x1,y1 ); p1.set( x2,y2 ); p2.set( x3,y3 ); p3.set( x4,y4 );
  }
};

struct Obj
{
  vec3 min;
  vec3 max;
};

struct ObjInfo
{
  ivec2 gridOrigin;
  ivec2 gridSize;
  int   gridOffset;
  vec4  color;
};

class Contour
{
public:
  friend class Object;

  std::vector< int > segments;
  std::vector< Vec2 > points;
  std::vector< Vec2 > flatPoints;
  
  GLuint bufFlatPoints;

public:
};

class Object
{
  Contour *contour;
  bool penDown;
  Vec2 pen;
  Vec2 start;

public:

  Vec2 min;
  Vec2 max;

public:

  bool buffersInit;
  GLuint bufLines;
  GLuint bufQuads;
  GLuint bufLinesQuads;
  GLuint64 ptrLinesQuads;

public:

  Vec4 color;
  std::vector< Contour* > contours;
  std::vector< Line > lines;
  std::vector< Cubic > cubics;
  std::vector< Quad > quads;

public:

  Object();
  ~Object();

  void moveTo( Float x, Float y,
    SegSpace::Enum space = SegSpace::Absolute );

  void lineTo( Float x, Float y,
    SegSpace::Enum space = SegSpace::Absolute );

  void quadTo( Float x1, Float y1, Float x2, Float y2,
    SegSpace::Enum space = SegSpace::Absolute );

  void cubicTo( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3,
    SegSpace::Enum space = SegSpace::Absolute );

  void close();

  Object* cubicsToQuads();
  void updateBounds();
  void updateBuffers();
};

class Image
{
public:

  Vec2 min;
  Vec2 max;
  ivec2 gridSize;
  Vec2 gridOrigin;
  Vec2 cellSize;

private:

  bool buffersInit;

  GLuint   bufObjs;
  GLuint   bufObjInfos;
  GLuint64 ptrObjInfos;

  GLuint   bufGpuInfo;
  GLuint64 ptrGpuInfo;
  GLuint   bufGpuGrid;
  GLuint64 ptrGpuGrid;
  GLuint   bufGpuStream;
  GLuint64 ptrGpuStream;
  
  int   *ptrCpuInfo;
  int   *ptrCpuGrid;
  float *ptrCpuStream;

public:

  std::vector< Obj > objs;
  std::vector< ObjInfo > objInfos;
  std::vector< Object* > objects;

public:

  Image();

  void updateBounds (int gridResX, int gridResY);
  void updateBuffers();

  void encodeCpu (ImageEncoderCpu *encoder);
  void encodeGpu (ImageEncoderGpu *encoder);

  void renderClassic (RendererClassic *renderer);
  void renderRandom (RendererRandom *renderer, VertexBuffer *buf, GLenum mode);
};

//Defs
////////////////////////////////////////////

#define USE_IMAGE     0
#define CPU_USE_AUX   1
#define GPU_USE_AUX   1

#define INFO_COUNTER_STREAMLEN   0
#define NUM_INFO_COUNTERS        1

#define OBJCELL_COUNTER_PREV     0
#define OBJCELL_COUNTER_AUX      1
#define OBJCELL_COUNTER_WIND     1
#define NUM_OBJCELL_COUNTERS     2

#define CELL_COUNTER_PREV        0
#define NUM_CELL_COUNTERS        1

#define NODE_TYPE_LINE           1
#define NODE_TYPE_QUAD           2
#define NODE_TYPE_OBJECT         3

#define NODE_SIZE_OBJINFO        9
#define NODE_SIZE_LINE           //TODO
#define NODE_SIZE_QUAD           //TODO
#define NODE_SIZE_OBJECT         //TODO

#define MAX_STREAM_SIZE          1000000

//Memory layout============================
//ptrInfo:        |StreamSize|
//ptrObjects:     |Objects|...|...|...
//ptrGrid:        |Width*Height|ObjW*ObjH|...|...
//ptrStream:      |Lines|Quads|...|...|...|...|...

#endif//RVGIMAGE_H
