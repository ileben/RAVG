#ifndef __GEGLHEADERS_H
#define __GEGLHEADERS_H

#if defined(__APPLE__)
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#
#else
#
#  define GL_GLEXT_LEGACY //dont define extensions for us
#  include <GL/gl.h>
#  include <GL/glu.h>
//#  include <GL/glut.h>
#  include <GL/freeglut.h>
#
#  if !defined (WIN32)
#    include <GL/glx.h>
#  endif
#
#endif

#include "GL/glext.h"
#if defined (WIN32)
#  include "GL/wglext.h"
#endif

typedef void (*PFVOID) ();

/*******************************************************
Function pointers
********************************************************/

//GL_VERSION_1_2
extern PFNGLDRAWRANGEELEMENTSPROC         GE_glDrawRangeElements;
extern PFNGLTEXIMAGE3DPROC                GE_glTexImage3D;
extern PFNGLTEXSUBIMAGE3DEXTPROC          GE_glTexSubImage3D;

//GL_VERSION_1_3
extern PFNGLACTIVETEXTUREPROC             GE_glActiveTexture;
extern PFNGLMULTITEXCOORD2FPROC           GE_glMultiTexCoord2f;

//GL_VERSION_1_5
extern PFNGLGENBUFFERSPROC                GE_glGenBuffers;
extern PFNGLDELETEBUFFERSPROC             GE_glDeleteBuffers;
extern PFNGLBINDBUFFERPROC                GE_glBindBuffer;
extern PFNGLBUFFERDATAPROC                GE_glBufferData;
extern PFNGLBUFFERSUBDATAPROC             GE_glBufferSubData;
extern PFNGLMAPBUFFERPROC                 GE_glMapBuffer;
extern PFNGLUNMAPBUFFERPROC               GE_glUnmapBuffer;

//GL_VERSION_2_0
extern PFNGLCREATEPROGRAMPROC             GE_glCreateProgram;
extern PFNGLCREATESHADERPROC              GE_glCreateShader;
extern PFNGLATTACHSHADERPROC              GE_glAttachShader;
extern PFNGLSHADERSOURCEPROC              GE_glShaderSource;
extern PFNGLCOMPILESHADERPROC             GE_glCompileShader;
extern PFNGLLINKPROGRAMPROC               GE_glLinkProgram;
extern PFNGLUSEPROGRAMPROC                GE_glUseProgram;
extern PFNGLDETACHSHADERPROC              GE_glDetachShader;
extern PFNGLDELETESHADERPROC              GE_glDeleteShader;
extern PFNGLDELETEPROGRAMSARBPROC         GE_glDeleteProgramsARB;
extern PFNGLGETINFOLOGARBPROC             GE_glGetInfoLogARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC   GE_glGetObjectParameterivARB;
extern PFNGLGETUNIFORMLOCATIONPROC        GE_glGetUniformLocation;
extern PFNGLUNIFORM1IPROC                 GE_glUniform1i;
extern PFNGLUNIFORM2IPROC                 GE_glUniform2i;
extern PFNGLUNIFORM3IPROC                 GE_glUniform3i;
extern PFNGLUNIFORM4IPROC                 GE_glUniform4i;
extern PFNGLUNIFORM1FPROC                 GE_glUniform1f;
extern PFNGLUNIFORM2FPROC                 GE_glUniform2f;
extern PFNGLUNIFORM3FPROC                 GE_glUniform3f;
extern PFNGLUNIFORM4FPROC                 GE_glUniform4f;
extern PFNGLUNIFORM1FVPROC                GE_glUniform1fv;
extern PFNGLUNIFORM2FVPROC                GE_glUniform2fv;
extern PFNGLUNIFORM3FVPROC                GE_glUniform3fv;
extern PFNGLUNIFORM4FVPROC                GE_glUniform4fv;
extern PFNGLUNIFORMMATRIX4FVPROC          GE_glUniformMatrix4fv;
extern PFNGLBINDATTRIBLOCATIONPROC        GE_glBindAttribLocation;
extern PFNGLGETATTRIBLOCATIONPROC         GE_glGetAttribLocation;
extern PFNGLVERTEXATTRIBPOINTERPROC       GE_glVertexAttribPointer;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC  GE_glDisableVertexAttribArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC   GE_glEnableVertexAttribArray;
extern PFNGLGETSHADERIVPROC               GE_glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC          GE_glGetShaderInfoLog;
extern PFNGLGETPROGRAMIVPROC              GE_glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC         GE_glGetProgramInfoLog;

//GL_EXT_framebuffer_object
extern PFNGLGENFRAMEBUFFERSPROC           GE_glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC           GE_glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC        GE_glDeleteFramebuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC    GE_glCheckFramebufferStatus;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC      GE_glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC   GE_glFramebufferRenderbuffer;
extern PFNGLISRENDERBUFFERPROC            GE_glIsRenderbuffer;
extern PFNGLBINDRENDERBUFFERPROC          GE_glBindRenderbuffer;
extern PFNGLDELETERENDERBUFFERSPROC       GE_glDeleteRenderbuffers;
extern PFNGLGENRENDERBUFFERSPROC          GE_glGenRenderbuffers;
extern PFNGLRENDERBUFFERSTORAGEPROC       GE_glRenderbufferStorage;


//GL_ARB_draw_buffers
extern PFNGLDRAWBUFFERSPROC               GE_glDrawBuffers;

//GL_ARB_occlusion_query
extern PFNGLGENQUERIESPROC                GE_glGenQueries;
extern PFNGLDELETEQUERIESPROC             GE_glDeleteQueries;
extern PFNGLISQUERYPROC                   GE_glIsQuery;
extern PFNGLBEGINQUERYPROC                GE_glBeginQuery;
extern PFNGLENDQUERYPROC                  GE_glEndQuery;
extern PFNGLGETQUERYIVPROC                GE_glGetQueryiv;
extern PFNGLGETQUERYOBJECTIVPROC          GE_glGetQueryObjectiv;
extern PFNGLGETQUERYOBJECTUIVPROC         GE_glGetQueryObjectuiv;

//GL_ARB_vertex_array_object
extern PFNGLBINDVERTEXARRAYPROC           GE_glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC        GE_glDeleteVertexArrays;
extern PFNGLGENVERTEXARRAYSPROC           GE_glGenVertexArrays;
extern PFNGLISVERTEXARRAYPROC             GE_glIsVertexArray;

//GL_VERSION_3_0
extern PFNGLVERTEXATTRIBIPOINTERPROC      GE_glVertexAttribIPointer;

//GL_EXT_shader_image_load_store
extern PFNGLBINDIMAGETEXTUREEXTPROC       GE_glBindImageTexture;
extern PFNGLMEMORYBARRIEREXTPROC          GE_glMemoryBarrier;

//GL_NV_shader_buffer_load
extern PFNGLMAKEBUFFERRESIDENTNVPROC        GE_glMakeBufferResident;
extern PFNGLMAKEBUFFERNONRESIDENTNVPROC     GE_glMakeBufferNonResident;
extern PFNGLGETBUFFERPARAMETERUI64VNVPROC   GE_glGetBufferParameterui64v;
extern PFNGLUNIFORMUI64NVPROC               GE_glUniformui64;

//GL_ARB_sample_shading
extern PFNGLMINSAMPLESHADINGARBPROC         GE_glMinSampleShading;

//Vertical sync
extern PFNWGLSWAPINTERVALEXTPROC          GE_wglSwapInterval;

/*******************************************************
Function re-routing
********************************************************/

#ifndef GE_NO_EXTENSION_ROUTING

//GL_VERSION_1_2
#define glDrawRangeElements          GE_glDrawRangeElements
#define glTexImage3D                 GE_glTexImage3D
#define glTexSubImage3D              GE_glTexSubImage3D

//GL_VERSION_1_3
#define glActiveTexture              GE_glActiveTexture
#define glMultiTexCoord2f            GE_glMultiTexCoord2f

//GL_VERSION_1_5
#define glGenBuffers                GE_glGenBuffers
#define glDeleteBuffers             GE_glDeleteBuffers
#define glBindBuffer                GE_glBindBuffer
#define glBufferData                GE_glBufferData
#define glBufferSubData             GE_glBufferSubData
#define glMapBuffer                 GE_glMapBuffer
#define glUnmapBuffer               GE_glUnmapBuffer

//GL_VERSION_2_0
#define glCreateProgram              GE_glCreateProgram
#define glCreateShader               GE_glCreateShader
#define glAttachShader               GE_glAttachShader
#define glShaderSource               GE_glShaderSource
#define glCompileShader              GE_glCompileShader
#define glLinkProgram                GE_glLinkProgram
#define glUseProgram                 GE_glUseProgram
#define glDetachShader               GE_glDetachShader
#define glDeleteShader               GE_glDeleteShader
#define glDeleteProgramsARB          GE_glDeleteProgramsARB
#define glDeleteProgram(p)          {GE_glDeleteProgramsARB(1, &p);}
#define glGetInfoLogARB              GE_glGetInfoLogARB
#define glGetObjectParameterivARB    GE_glGetObjectParameterivARB
#define glGetUniformLocation         GE_glGetUniformLocation
#define glUniform1i                  GE_glUniform1i
#define glUniform2i                  GE_glUniform2i
#define glUniform3i                  GE_glUniform3i
#define glUniform4i                  GE_glUniform4i
#define glUniform1f                  GE_glUniform1f
#define glUniform2f                  GE_glUniform2f
#define glUniform3f                  GE_glUniform3f
#define glUniform4f                  GE_glUniform4f
#define glUniform1fv                 GE_glUniform1fv
#define glUniform2fv                 GE_glUniform2fv
#define glUniform3fv                 GE_glUniform3fv
#define glUniform4fv                 GE_glUniform4fv
#define glUniformMatrix4fv           GE_glUniformMatrix4fv
#define glBindAttribLocation         GE_glBindAttribLocation
#define glGetAttribLocation          GE_glGetAttribLocation
#define glVertexAttribPointer        GE_glVertexAttribPointer
#define glDisableVertexAttribArray   GE_glDisableVertexAttribArray
#define glEnableVertexAttribArray    GE_glEnableVertexAttribArray
#define glGetShaderiv                GE_glGetShaderiv
#define glGetShaderInfoLog           GE_glGetShaderInfoLog
#define glGetProgramiv               GE_glGetProgramiv
#define glGetProgramInfoLog          GE_glGetProgramInfoLog

//GL_EXT_framebuffer_object
#define glGenFramebuffers           GE_glGenFramebuffers
#define glBindFramebuffer           GE_glBindFramebuffer
#define glDeleteFramebuffers        GE_glDeleteFramebuffers
#define glCheckFramebufferStatus    GE_glCheckFramebufferStatus
#define glFramebufferTexture2D      GE_glFramebufferTexture2D
#define glFramebufferRenderbuffer   GE_glFramebufferRenderbuffer
#define glIsRenderbuffer            GE_glIsRenderbuffer
#define glBindRenderbuffer          GE_glBindRenderbuffer
#define glDeleteRenderbuffers       GE_glDeleteRenderbuffers
#define glGenRenderbuffers          GE_glGenRenderbuffers
#define glRenderbufferStorage       GE_glRenderbufferStorage

//GL_ARB_draw_buffers
#define glDrawBuffers               GE_glDrawBuffers

//GL_ARB_occlusion_query
#define glGenQueries                GE_glGenQueries
#define glDeleteQueries             GE_glDeleteQueries
#define glIsQuery                   GE_glIsQuery
#define glBeginQuery                GE_glBeginQuery
#define glEndQuery                  GE_glEndQuery
#define glGetQueryiv                GE_glGetQueryiv
#define glGetQueryObjectiv          GE_glGetQueryObjectiv
#define glGetQueryObjectuiv         GE_glGetQueryObjectuiv

//GL_ARB_vertex_array_object
#define glBindVertexArray           GE_glBindVertexArray
#define glDeleteVertexArrays        GE_glDeleteVertexArrays
#define glGenVertexArrays           GE_glGenVertexArrays
#define glIsVertexArray             GE_glIsVertexArray

//GL_VERSION_3_0
#define glVertexAttribIPointer      GE_glVertexAttribIPointer

//GL_EXT_shader_image_load_store
#define glBindImageTexture          GE_glBindImageTexture
#define glMemoryBarrier             GE_glMemoryBarrier

//GL_NV_shader_buffer_load
#define glMakeBufferResident        GE_glMakeBufferResident
#define glMakeBufferNonResident     GE_glMakeBufferNonResident
#define glGetBufferParameterui64v   GE_glGetBufferParameterui64v
#define glUniformui64               GE_glUniformui64

//GL_ARB_sample_shading
#define glMinSampleShading          GE_glMinSampleShading

//Vertical sync
#define wglSwapInterval             GE_wglSwapInterval

#endif//!GE_NO_EXTENSION_ROUTING

/****************************************************
Initialization
*****************************************************/

void rvgGLInit();

#endif /* __GEGLHEADERS_H */
