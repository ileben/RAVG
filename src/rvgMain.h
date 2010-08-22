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

  void moveTo( Float x, Float y,
    SegSpace::Enum space = SegSpace::Absolute );

  void lineTo( Float x, Float y,
    SegSpace::Enum space = SegSpace::Absolute );

  void quadTo( Float x1, Float y1, Float x2, Float y2,
    SegSpace::Enum space = SegSpace::Absolute );

  void cubicTo( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3,
    SegSpace::Enum space = SegSpace::Absolute );

  void close();

  void cubicsToQuads();
  void updateBounds();
  void updateGrid();
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
  
  int *gridWinding;
  Vec2 *gridPivots;

public:

  bool buffersInit;
  GLuint bufPivotPos;
  GLuint bufPivotWind;
  GLuint texGrid;

  GLuint bufObjs;
  GLuint bufObjInfos;
  GLuint64 ptrObjInfos;
  
  GLuint bufAuxGrid;
  GLuint64 ptrAuxGrid;
  GLuint bufAuxStream;
  GLuint64 ptrAuxStream;

  GLuint bufGpuAuxInfo;
  GLuint64 ptrGpuAuxInfo;
  GLuint bufGpuAuxGrid;
  GLuint64 ptrGpuAuxGrid;
  GLuint bufGpuAuxStream;
  GLuint64 ptrGpuAuxStream;
  
  int cpuStreamLen;

public:

  std::vector< Obj > objs;
  std::vector< ObjInfo > objInfos;
  std::vector< Object* > objects;
  int *cpuObjects;
  int *cpuCounters;
  float *cpuStream;

#if(0)

  ivec4 imageLoad (int* counters, const IVec3 &coord);
  void imageStore (int* counters, const IVec3 &coord, const ivec4 &value);
  int imageAtomicAdd (int* counters, const IVec3 &coord, int value);
  int imageAtomicExchange (int* counters, const IVec3 &coord, int value);

  void addLine (const Vec2 &l0, const Vec2 &l1, const IVec2 &gridCoord);
  void addQuad (const Vec2 &q0, const Vec2 &q1, const Vec2 &q2, const IVec2 &gridCoord);
  void addObject (const Vec4 &color, const IVec2 &gridCoord);

  void encodeLine (const Vec2 &line0, const Vec2 &line1, const IVec2 &gridCoord, float *stream, int *counters);
  void encodeQuad (const Vec2 &quad0, const Vec2 &quad1, const Vec2 &quad2, const IVec2 &gridCoord, float *stream, int *counters);
  void encodeObject (const Vec4 &color, const IVec2 &gridCoord, float *stream, int *counters);

#endif

#if (1)

  //Emulated GLSL functions
  int atomicAdd (int *ptr, int value);
  int atomicExchange (int *ptr, int value);

  //Emulated uniform variables
  int *ptrObjects;
  int *ptrInfo;
  int *ptrGrid;
  float *ptrStream;

  //Emulated shader utility functions
  int addLine (const Vec2 &l0, const Vec2 &l1, int *ptrObjCell);
  int addQuad (const Vec2 &q0, const Vec2 &q1, const Vec2 &q2, int *ptrObjCell);
  int addObject (int objectId, const Vec4 &color, int lastSegmentOffset, int *ptrCell);

  //Emulated shader main() functions
  void frag_encodeInit
    (
    const ivec2 &gridCoord
    );

  void frag_encodeInitObject
    (
    int objectId,
    const ivec2 &gridCoord
    );

  void frag_encodeLine
    (
    int objectId,
    const ivec2 &gridCoord,
    const Vec2 &line0,
    const Vec2 &line1
    );

  void frag_encodeQuad
    (
    int objectId,
    const ivec2 &gridCoord,
    const Vec2 &quad0,
    const Vec2 &quad1,
    const Vec2 &quad2
    );
  void frag_encodeObject
    (
    int objectId,
    const Vec4 &color,
    const ivec2 &gridCoord
    );
  void frag_encodeSort
    (
    const ivec2 &gridCoord
    );

#endif

public:

  Image();

  void updateBounds();
  void updateGrid();
  void updateBuffers();
  void encodeCpuAux();
};

class Shader
{
  //Static /////////////
private:

  struct Def  { std::string key; std::string value; };
  static std::vector< Def > defs;
  static std::string ApplyDefs (const std::string &source);

public:

  static void Define( const std::string &key, const std::string &value );

  //Non-static /////////
private:

  std::string vertFile;
  std::string geomFile;
  std::string fragFile;

public:

  GLProgram *program;
  GLShader *vertex;
  GLShader *geometry;
  GLShader *fragment;

public:

  Shader (const std::string &vertFile, const std::string &fragFile);
  Shader (const std::string &vertFile, const std::string &geomFile, const std::string &fragFile);
  ~Shader ();
  bool load ();
  void use();
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

void lineIntersectionY (Vec2 l0, Vec2 l1, float y, float minX, float maxX,
                        bool &found, float &x);

void quadIntersectionY (Vec2 q0, Vec2 q1, Vec2 q2, float y, float minX, float maxX,
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

#define COUNTER_MASTER     0
#define COUNTER_PREV       1
#define COUNTER_AUX        2
#define COUNTER_OCCLUSION  3

#define MIX_OBJECT_LEN           500000
#define MIX_STREAM_LEN           500000

#define INFO_COUNTER_STREAMLEN   0
#define INFO_COUNTER_GRIDLEN     1
#define NUM_INFO_COUNTERS        2

#define OBJCELL_COUNTER_PREV     0
#define OBJCELL_COUNTER_AUX      1
#define NUM_OBJCELL_COUNTERS     2

#define CELL_COUNTER_PREV        0
#define NUM_CELL_COUNTERS        1

#define NODE_TYPE_LINE           1
#define NODE_TYPE_QUAD           2
#define NODE_TYPE_OBJECT         3

//Memory layout============================
//ptrInfo:        |StreamSize|GridSize|
//ptrObjects:     |Objects|...|...|...
//ptrObjectGrids: |Grids|...|...|...
//ptrGrid:        |Width*Height|
//ptrStream:      |Lines|Quads|...|...|...

#endif//RVGMAIN_H
