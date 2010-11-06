#ifndef RVGIMAGE_H
#define RVGIMAGE_H 1

///////////////////////////////////////////////////////////////////
// Forward declarations

class Contour;
class Object;
class ObjectProcessor;
class Image;
class EncoderCpu;
class EncoderGpu;
class RendererRandom;
class RendererClassic;

///////////////////////////////////////////////////////////////////

namespace SegSpace {
  enum Enum {
    Absolute = (1 << 7),
    Relative = (1 << 6),
    Mask     = 0xC0
  };
};

namespace SegType {
  enum Enum {

    Close        = 0,
    MoveTo       = 1,
    LineTo       = 2,
    QuadTo       = 3,
    CubicTo      = 4,
    Mask         = 0x3F,

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

namespace ProcessFlags {
  enum Enum {
    Absolute = (1 << 0)
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
  friend class Object;
  friend class ObjectFlatten;
  friend class Image;

private:
  std::vector< Vec2 > flatPoints;

private:
  DynamicGpuBuffer bufFlatPoints;
};

class Object
{
  friend class ObjectProcessor;
  friend class ObjectFlatten;
  friend class Image;

private:

  bool flat;
  Vec2 min;
  Vec2 max;

private:

  DynamicGpuBuffer bufLines;
  DynamicGpuBuffer bufQuads;

private:

  //raw data
  Vec4 color;
  std::vector< int > segments;
  std::vector< Vec2 > points;

  //flat data
  std::vector< Contour* > contours;
  std::vector< Line > lines;
  std::vector< Cubic > cubics;
  std::vector< Quad > quads;

private:

  void updateBounds();
  void updateBuffers();
  void clearFlat();

public:

  Object();
  ~Object();

  void setColor( float r, float g, float b, float a=1.0f );
  const Vec4& getColor() { return color; }

  void moveTo( Float x, Float y,
    int space = SegSpace::Absolute );

  void lineTo( Float x, Float y,
    int space = SegSpace::Absolute );

  void quadTo( Float x1, Float y1, Float x2, Float y2,
    int space = SegSpace::Absolute );

  void cubicTo( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3,
    int space = SegSpace::Absolute );

  void close();

  void clear();

  void process (ObjectProcessor &proc, ProcessFlags::Enum flags);

  Object* cubicsToQuads();
};


class ObjectProcessor
{
  friend class Object;

private:
  Object *object;
  Vec2 pen;
  Vec2 start;

protected:
  Object *getObject() { return object; }
  const Vec2& getPen () { return pen; }
  const Vec2& getStart () { return start; }

public:

  virtual void begin() {}
  virtual void moveTo (const Vec2 &p1, int space) {}
  virtual void lineTo (const Vec2 &p1, int space) {}
  virtual void quadTo (const Vec2 &p1, const Vec2 &p2, int space) {}
  virtual void cubicTo (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int space) {}
  virtual void close () {}
  virtual void end() {}
};


class ObjectFlatten : public ObjectProcessor
{
  Contour *contour;
  bool penDown;

public:
  ObjectFlatten ();
  virtual void moveTo (const Vec2 &p1, int space);
  virtual void lineTo (const Vec2 &p1, int space);
  virtual void quadTo (const Vec2 &p1, const Vec2 &p2, int space);
  virtual void cubicTo (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int space);
  virtual void close ();
};

class ObjectClone : public ObjectProcessor
{
protected:
  Object *clone;

public:
  virtual void begin ()
  { clone = new Object(); }

  virtual void moveTo (const Vec2 &p1, int space)
  { clone->moveTo( p1.x, p1.y, space ); }

  virtual void lineTo (const Vec2 &p1, int space)
  { clone->lineTo( p1.x, p1.y, space ); }

  virtual void quadTo (const Vec2 &p1, const Vec2 &p2, int space)
  { clone->quadTo( p1.x, p1.y, p2.x, p2.y, space ); }

  virtual void cubicTo (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int space)
  { clone->cubicTo( p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, space ); }

  virtual void close ()
  { clone->close(); }

  Object* getClone()
  { return clone; }
};

class CubicsToQuads : public ObjectClone
{
  std::vector< Quad > quads;

public:
  virtual void cubicTo (const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int space);
};


class Image
{
private:

  int gridResX;
  int gridResY;

private:

  Vec2 min;
  Vec2 max;

  ivec2 gridSize;
  Vec2 gridOrigin;
  Vec2 cellSize;

private:

  std::vector< Obj > objs;
  std::vector< ObjInfo > objInfos;
  std::vector< Object* > objects;

private:

  DynamicGpuBuffer bufObjs;
  DynamicGpuBuffer bufObjInfos;

  StaticGpuBuffer bufGpuInfo;
  StaticGpuBuffer bufGpuGrid;
  StaticGpuBuffer bufGpuStream;

  int   *ptrCpuInfo;
  int   *ptrCpuGrid;
  float *ptrCpuStream;

  void flatten ();
  void updateBounds ();
  void updateBuffers ();

public:

  Image();
  
  void addObject (Object *obj);
  void removeAllObjects ();

  int getNumObjects ();
  Object* getObject (int index);

  void setGridResolution (int x, int y);

  void update ();
  void encodeCpu (EncoderCpu *encoder);
  void encodeGpu (EncoderGpu *encoder);

  void renderClassic (RendererClassic *renderer);
  void renderRandom (RendererRandom *renderer, VertexBuffer *buf, GLenum mode);

  const Vec2& getMin () { return min; }
  const Vec2& getMax () { return max; }
  Vec2 getSize () { return max - min; }
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

#define MAX_STREAM_SIZE          2000000

//Memory layout============================
//ptrInfo:        |StreamSize|
//ptrObjects:     |Objects|...|...|...
//ptrGrid:        |Width*Height|ObjW*ObjH|...|...
//ptrStream:      |Lines|Quads|...|...|...|...|...

#endif//RVGIMAGE_H
