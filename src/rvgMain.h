#ifndef RVGMAIN_H
#define RVGMAIN_H 1

////////////////////////////////////////////////////////////////////
//Internal headers

#include "rvgDefs.h"
#include "rvgGLHeaders.h"
#include "rvgShaders.h"
#include "rvgVectors.h"
#include "rvgMatrix.h"
#include "rvgMatrixStack.h"

///////////////////////////////////////////////////////////////////
//Typedef for vector names to match GLSL (easier translation)

typedef Vec2 vec2;
typedef Vec3 vec3;
typedef Vec4 vec4;

typedef IVec2 ivec2;
typedef IVec3 ivec3;
typedef IVec4 ivec4;


///////////////////////////////////////////////////////////////////
// Forward declarations

class Contour;
class Object;

///////////////////////////////////////////////////////////////////
// Classes

namespace StreamSegType {
  enum Enum {
    Line = 1,
    Quad = 2,
    Object = 3,
    End = -1
  };
};

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
  
  int *gridWinding;

public:

  bool buffersInit;
  GLuint bufLines;
  GLuint bufQuads;
  GLuint bufABC;
  GLuint bufPivotWind;
  GLuint texGrid;
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
  void updateGrid();
  void updateBuffers();
};

#include "rvgImageEncoder.h"
#include "rvgShader.h"

class Image
{
public:

  Vec2 min;
  Vec2 max;
  ivec2 gridSize;
  Vec2 gridOrigin;
  Vec2 cellSize;
  
  int *gridWinding;
  Vec2 *gridPivots;

public:

  bool buffersInit;
  GLuint bufPivotPos;
  GLuint bufPivotWind;
  GLuint texGrid;

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

  int cpuStreamLen;

public:

  std::vector< Obj > objs;
  std::vector< ObjInfo > objInfos;
  std::vector< Object* > objects;

public:

  Image();

  void updateBounds();
  void updateGrid();
  void updateBuffers();
  void encodeCpu (ImageEncoder *encoder);
};

class Vertex
{
public:
  Vec3 coord;
  Vec2 texcoord;
};

class VertexBuffer
{
public:
  bool onGpu;
  GLuint gpuId;
  std::vector< Vertex > verts;

  VertexBuffer();
  void toGpu();
};

/////////////////////////////////////////////////////////////////////////////////
//Winding utilities

int lineWinding (Vec2 l0, Vec2 l1, Vec2 p0, Vec2 p1);
int quadWinding (Vec2 q0, Vec2 q1, Vec2 q2, Vec2 p0, Vec2 p1);

void lineIntersectionY (Vec2 l0, Vec2 l1, float y,
                        bool &found, float &x);

void quadIntersectionY (Vec2 q0, Vec2 q1, Vec2 q2, float y,
                        bool &found1, bool &found2, float &x1, float &x2);

bool pointInRect (const Vec2 &min, const Vec2 &max, const Vec2 &p);

bool lineInCell (const Vec2 &min, const Vec2 &max, const Line &line);

bool quadInCell (const Vec2 &min, const Vec2 &max, const Quad &quad);


//Tiger
////////////////////////////////////////////

extern VGint     pathCount;
extern VGint     commandCounts[];
extern const VGubyte*  commandArrays[];
extern const VGfloat*  dataArrays[];
extern const VGfloat*  styleArrays[];

//Defs
////////////////////////////////////////////

#define USE_IMAGE     0
#define CPU_USE_AUX   1
#define GPU_USE_AUX   1

#define COUNTER_LEN               4
#define MAX_STREAM_LEN            2000
#define MAX_COMBINED_STREAM_LEN   500000

#define MIX_OBJECT_LEN           500000
#define MIX_STREAM_LEN           500000

#define INFO_COUNTER_STREAMLEN   0
#define INFO_COUNTER_GRIDLEN     1
#define NUM_INFO_COUNTERS        2

#define OBJCELL_COUNTER_PREV     0
#define OBJCELL_COUNTER_AUX      1
#define OBJCELL_COUNTER_WIND     1
#define NUM_OBJCELL_COUNTERS     2

#define CELL_COUNTER_PREV        0
#define NUM_CELL_COUNTERS        1

#define NODE_TYPE_LINE           1
#define NODE_TYPE_QUAD           2
#define NODE_TYPE_OBJECT         3

//Memory layout============================
//ptrInfo:        |StreamSize|GridSize|
//ptrObjects:     |Objects|...|...|...
//ptrGrid:        |Width*Height|ObjW*ObjH|...|...
//ptrStream:      |Lines|Quads|...|...|...|...|...

#endif//RVGMAIN_H
