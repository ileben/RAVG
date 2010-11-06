#ifndef RVGMAIN_H
#define RVGMAIN_H 1

////////////////////////////////////////////////////////////////////
//Internal headers

#include "rvgDefs.h"
#include "rvgGLHeaders.h"
#include "rvgGpuBuffer.h"
#include "rvgShaders.h"
#include "rvgVectors.h"
#include "rvgMatrix.h"
#include "rvgMatrixStack.h"
#include "rvgVertexBuffer.h"

///////////////////////////////////////////////////////////////////
//Typedef for vector names to match GLSL (easier translation)

typedef Vec2 vec2;
typedef Vec3 vec3;
typedef Vec4 vec4;

typedef IVec2 ivec2;
typedef IVec3 ivec3;
typedef IVec4 ivec4;

///////////////////////////////////////////////////////////////////
//Kinda hax but this would be "global" anyway before OpenGL 3.0

extern MatrixStack matModelView;
extern MatrixStack matProjection;
extern MatrixStack matTexture;

#include "rvgImage.h"
#include "rvgEncoderCpu.h"
#include "rvgEncoderGpu.h"
#include "rvgRenderer.h"
#include "rvgFont.h"
#include "rvgSvg.h"

/////////////////////////////////////////////////////////////////////
//GL utilities

void checkGlError (const std::string &text);
void renderFullScreenQuad (Shader *shader);
void renderQuad (Shader *shader, Vec2 min, Vec2 max);

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

////////////////////////////////////////////
//Tiger

extern VGint     pathCount;
extern VGint     commandCounts[];
extern const VGubyte*  commandArrays[];
extern const VGfloat*  dataArrays[];
extern const VGfloat*  styleArrays[];

#endif//RVGMAIN_H
