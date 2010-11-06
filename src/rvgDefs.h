#ifndef RVGDEFS_H
#define RVGDEFS_H 1

//Platform SDK compatibility with VC++ default headers sux
//so we disable "macro already defined" warnings
#if defined(WIN32)
#  pragma warning (disable : 4005) 
#  include <windows.h>
#  pragma warning (default : 4005)
#endif//defined(WIN32)


//External headers
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cassert>
#include <cmath>
#include <list>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

//Types
typedef long long int            Int64;
typedef unsigned long long int   Uint64;
typedef int                      Int32;
typedef unsigned int             Uint32;
typedef char                     Int8;
typedef unsigned char            Uint8;
typedef float                    Float;
typedef int                      Int;
typedef unsigned int             Uint;

//Functions
#define PI              3.1415926535f
#define SQRT(x)         std::sqrtf(x)
#define COS(x)          std::cosf(x)
#define SIN(x)          std::sinf(x)
#define TAN(x)          std::tanf(x)
#define ACOS(x)         std::acosf(x)
#define FLOOR(x)        std::floorf(x)
#define CEIL(x)         std::ceilf(x)


//OpenVG
#define VG_MOVE_TO_ABS     0
#define VG_LINE_TO_ABS     1
#define VG_QUAD_TO_ABS     2
#define VG_CUBIC_TO_ABS    3
#define VG_CLOSE_PATH      4

typedef Uint8 VGubyte;
typedef Float VGfloat;
typedef Int32 VGint;

#endif//RVGDEFS_H
