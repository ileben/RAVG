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

#endif//RVGDEFS_H
