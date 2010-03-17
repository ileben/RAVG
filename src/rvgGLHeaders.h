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

#ifndef APIENTRY
#  define APIENTRY
#endif

typedef void (*PFVOID) ();

/*******************************************************
GLX_VERSION_1_4
********************************************************/

#if !defined(WIN32) && !defined(__APPLE__)
#include <dlfcn.h>
typedef PFVOID (*GE_PFGLXGETPROCADDRESS) (const GLubyte *);
#endif

/*******************************************************
GL_EXT_separate_specular_color
********************************************************/

#ifndef GL_EXT_separate_specular_color
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA
#endif

/*******************************************************
GL_VERSION_1_2
********************************************************/

#ifndef GL_VERSION_1_2
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_CLAMP_TO_EDGE                  0x812F
#endif

#ifndef GL_VERSION_1_2
typedef void
  (APIENTRY* GE_PFGLDRAWRANGEELEMENTS)
  (GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
#endif

/*******************************************************
GL_VERSION_1_3
********************************************************/

#ifndef GL_VERSION_1_3
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_COMPRESSED_RGB                 0x84ED
#define GL_COMPRESSED_RGBA                0x84EE
#define GL_TEXTURE_COMPRESSED             0x86A1
#endif

#ifndef GL_VERSION_1_3

typedef void
  (APIENTRY* GE_PFGLACTIVETEXTURE)
  (GLenum);

typedef void
  (APIENTRY* GE_PFGLMULTITEXCOORD2F)
  (GLenum, GLfloat, GLfloat);

#endif

/*******************************************************
GL_VERSION_1_5
********************************************************/

#ifndef GL_VERSION_1_5
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
typedef ptrdiff_t                         GLintptr;
typedef ptrdiff_t                         GLsizeiptr;
#endif

#ifndef GL_VERSION_1_5

typedef void
  (APIENTRY *GE_PFGLGENBUFFERS)
  (GLsizei n, GLuint * buffers);

typedef void
  (APIENTRY *GE_PFGLBINDBUFFER)
  (GLenum target, GLuint buffer);

typedef void
  (APIENTRY *GE_PFGLBUFFERDATA)
  (GLenum, GLsizeiptr, const GLvoid *, GLenum);

typedef void
  (APIENTRY *GE_PFGLBUFFERSUBDATA)
  (GLenum, GLintptr, GLsizeiptr, const GLvoid *);

#endif

/*******************************************************
GL_ARB_vertex_array_object
********************************************************/

#ifndef GL_ARB_vertex_array_object

typedef void
  (APIENTRY* GE_PFGLBINDVERTEXARRAY)
  (GLuint array);

typedef void
  (APIENTRY* GE_PFGLDELETEVERTEXARRAYS)
  (GLsizei n, const GLuint *arrays);

typedef void
  (APIENTRY* GE_PFGLGENVERTEXARRAYS)
  (GLsizei n, GLuint *arrays);

typedef GLboolean
  (APIENTRY* GE_PFGLISVERTEXARRAY)
  (GLuint array);

#endif


/*******************************************************
GL_ARB_shader_objects
********************************************************/

#ifndef GL_ARB_shader_objects
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
#define GL_OBJECT_INFO_LOG_LENGTH_ARB     0x8B84
typedef char                              GLcharARB;
typedef unsigned int                      GLhandleARB;
#endif

#ifndef GL_VERSION_2_0
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
typedef char                              GLchar;
#endif

/*******************************************************
GL_VERSION_2_0
********************************************************/

#ifndef GL_VERSION_2_0
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#endif

#ifndef GL_VERSION_2_0

typedef GLuint
  (APIENTRY* GE_PFGLCREATEPROGRAM)
  (void);

typedef GLuint
  (APIENTRY* GE_PFGLCREATESHADER)
  (GLenum);

typedef void
  (APIENTRY* GE_PFGLATTACHSHADER)
  (GLuint, GLuint);

typedef void
  (APIENTRY* GE_PFGLSHADERSOURCE)
  (GLuint, GLsizei, const GLchar* *, const GLint *);

typedef void
  (APIENTRY* GE_PFGLCOMPILESHADER)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLLINKPROGRAM)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLUSEPROGRAM)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLDETACHSHADER)
  (GLuint, GLuint);

typedef void
  (APIENTRY* GE_PFGLDELETESHADER)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLDELETEPROGRAMSARB)
  (GLsizei, const GLuint *);

typedef void
  (APIENTRY* GE_PFGLGETINFOLOGARB)
  (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);

typedef void
  (APIENTRY* GE_PFGLGETOBJECTPARAMETERIVARB)
  (GLhandleARB, GLenum, GLint *);

typedef GLint
  (APIENTRY* GE_PFGLGETUNIFORMLOCATION)
  (GLuint, const GLchar *);

typedef void
  (APIENTRY* GE_PFGLUNIFORM1I)
  (GLint, GLint);

typedef void
  (APIENTRY* GE_PFGLUNIFORM2I)
  (GLint, GLint, GLint);

typedef void
  (APIENTRY* GE_PFGLUNIFORM3I)
  (GLint, GLint, GLint, GLint);

typedef void
  (APIENTRY* GE_PFGLUNIFORM4I)
  (GLint, GLint, GLint, GLint, GLint);

typedef void
  (APIENTRY* GE_PFGLUNIFORM1F)
  (GLint, GLfloat);

typedef void
  (APIENTRY* GE_PFGLUNIFORM2F)
  (GLint, GLfloat, GLfloat);

typedef void
  (APIENTRY* GE_PFGLUNIFORM3F)
  (GLint, GLfloat, GLfloat, GLfloat);

typedef void
  (APIENTRY* GE_PFGLUNIFORM4F)
  (GLint, GLfloat, GLfloat, GLfloat, GLfloat);

typedef void
  (APIENTRY* GE_PFGLUNIFORM1FV)
  (GLint, GLsizei, const GLfloat *);

typedef void
  (APIENTRY* GE_PFGLUNIFORM2FV)
  (GLint, GLsizei, const GLfloat *);

typedef void
  (APIENTRY* GE_PFGLUNIFORM3FV)
  (GLint, GLsizei, const GLfloat *);

typedef void
  (APIENTRY* GE_PFGLUNIFORM4FV)
  (GLint, GLsizei, const GLfloat *);

typedef void
  (APIENTRY* GE_PFGLUNIFORMMATRIX4FV)
  (GLint, GLsizei, GLboolean, const GLfloat *);

typedef void
  (APIENTRY* GE_PFGLBINDATTRIBLOCATION)
  (GLuint, GLuint, const GLchar *);

typedef GLint
  (APIENTRY* GE_PFGLGETATTRIBLOCATION)
  (GLuint, const GLchar *);

typedef void
  (APIENTRY* GE_PFGLVERTEXATTRIBPOINTER)
  (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);

typedef void
  (APIENTRY* GE_PFGLDISABLEVERTEXATTRIBARRAY)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLENABLEVERTEXATTRIBARRAY)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLGETSHADERIV)
  (GLuint, GLenum, GLint *);

typedef void
  (APIENTRY* GE_PFGLGETSHADERINFOLOG)
  (GLuint, GLsizei, GLsizei *, GLchar *);

typedef void
  (APIENTRY* GE_PFGLGETPROGRAMIV)
  (GLuint, GLenum, GLint *);

typedef void
  (APIENTRY* GE_PFGLGETPROGRAMINFOLOG)
  (GLuint, GLsizei, GLsizei *, GLchar *);

#endif


/**********************************************
GL_EXT_framebuffer_object
***********************************************/

#ifndef GL_EXT_framebuffer_object
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_STENCIL_INDEX1                 0x8D46
#define GL_STENCIL_INDEX4                 0x8D47
#define GL_STENCIL_INDEX8                 0x8D48
#define GL_STENCIL_INDEX16                0x8D49
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT           0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT   0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER          0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER          0x8CDC
#endif

#ifndef GL_EXT_framebuffer_object

typedef void
  (APIENTRY* GE_PFGLGENFRAMEBUFFERS)
  (GLsizei, GLuint *);

typedef void
  (APIENTRY* GE_PFGLBINDFRAMEBUFFER)
  (GLenum, GLuint);

typedef void
  (APIENTRY* GE_PFGLDELETEFRAMEBUFFERS)
  (GLsizei, const GLuint *);

typedef GLenum
  (APIENTRY* GE_PFGLCHECKFRAMEBUFFERSTATUS)
  (GLenum);

typedef void
  (APIENTRY* GE_PFGLFRAMEBUFFERTEXTURE2D)
  (GLenum, GLenum, GLenum, GLuint, GLint);

typedef void
  (APIENTRY* GE_PFGLFRAMEBUFFERRENDERBUFFER)
  (GLenum, GLenum, GLenum, GLuint);

typedef GLboolean
  (APIENTRY* GE_PFGLISRENDERBUFFER)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLBINDRENDERBUFFER)
  (GLenum, GLuint);

typedef void
  (APIENTRY* GE_PFGLDELETERENDERBUFFERS)
  (GLsizei, const GLuint *);

typedef void
  (APIENTRY* GE_PFGLGENRENDERBUFFERS)
  (GLsizei, GLuint *);

typedef void
  (APIENTRY* GE_PFGLRENDERBUFFERSTORAGE)
  (GLenum, GLenum, GLsizei, GLsizei);

#endif


/**********************************************
GL_EXT_packed_depth_stencil
***********************************************/

#ifndef GL_EXT_packed_depth_stencil
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_TEXTURE_STENCIL_SIZE           0x88F1
#endif


/**********************************************
GL_ARB_draw_buffers
***********************************************/

#ifndef GL_ARB_draw_buffers
#define GL_MAX_DRAW_BUFFERS               0x8824

typedef void
  (APIENTRY* GE_PFGLDRAWBUFFERS)
  (GLsizei, const GLenum *);

#endif

/**********************************************
GL_ARB_texture_float
***********************************************/

#ifndef GL_ARB_texture_float
#define GL_RGBA32F                        0x8814
#define GL_LUMINANCE32F                   0x8818
#define GL_RGBA16F                        0x881A
#endif

/***********************************************
GL_ARB_pixel_buffer_object
***********************************************/

#ifndef GL_EXT_pixel_buffer_object
#define GL_PIXEL_PACK_BUFFER              0x88EB
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
#endif

/***********************************************
GL_ARB_occlusion_query
***********************************************/

#ifndef GL_ARB_occlusion_query
#define GL_QUERY_COUNTER_BITS             0x8864
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_SAMPLES_PASSED                 0x8914

typedef void
  (APIENTRY* GE_PFGLGENQUERIES)
  (GLsizei n, GLuint *ids);

typedef void
  (APIENTRY* GE_PFGLDELETEQUERIES)
  (GLsizei n, const GLuint *ids);

typedef GLboolean
  (APIENTRY* GE_PFGLISQUERY)
  (GLuint id);

typedef void
  (APIENTRY* GE_PFGLBEGINQUERY)
  (GLenum target, GLuint id);

typedef void
  (APIENTRY* GE_PFGLENDQUERY)
  (GLenum target);

typedef void
  (APIENTRY* GE_PFGLGETQUERYIV)
  (GLenum target, GLenum pname, GLint *params);

typedef void
  (APIENTRY* GE_PFGLGETQUERYOBJECTIV)
  (GLuint id, GLenum pname, GLint *params);

typedef void
  (APIENTRY* GE_PFGLGETQUERYOBJECTUIV)
  (GLuint id, GLenum pname, GLuint *params);

#endif

/*******************************************************
Function pointers
********************************************************/

#ifndef GL_VERSION_1_2
extern GE_PFGLDRAWRANGEELEMENTS         GE_glDrawRangeElements;
#endif

#ifndef GL_VERSION_1_3
extern GE_PFGLACTIVETEXTURE             GE_glActiveTexture;
extern GE_PFGLMULTITEXCOORD2F           GE_glMultiTexCoord2f;
#endif

#ifndef GL_VERSION_1_5
extern GE_PFGLGENBUFFERS                GE_glGenBuffers;
extern GE_PFGLBINDBUFFER                GE_glBindBuffer;
extern GE_PFGLBUFFERDATA                GE_glBufferData;
extern GE_PFGLBUFFERSUBDATA             GE_glBufferSubData;
#endif

#ifndef GL_VERSION_2_0
extern GE_PFGLCREATEPROGRAM             GE_glCreateProgram;
extern GE_PFGLCREATESHADER              GE_glCreateShader;
extern GE_PFGLATTACHSHADER              GE_glAttachShader;
extern GE_PFGLSHADERSOURCE              GE_glShaderSource;
extern GE_PFGLCOMPILESHADER             GE_glCompileShader;
extern GE_PFGLLINKPROGRAM               GE_glLinkProgram;
extern GE_PFGLUSEPROGRAM                GE_glUseProgram;
extern GE_PFGLDETACHSHADER              GE_glDetachShader;
extern GE_PFGLDELETESHADER              GE_glDeleteShader;
extern GE_PFGLDELETEPROGRAMSARB         GE_glDeleteProgramsARB;
extern GE_PFGLGETINFOLOGARB             GE_glGetInfoLogARB;
extern GE_PFGLGETOBJECTPARAMETERIVARB   GE_glGetObjectParameterivARB;
extern GE_PFGLGETUNIFORMLOCATION        GE_glGetUniformLocation;
extern GE_PFGLUNIFORM1I                 GE_glUniform1i;
extern GE_PFGLUNIFORM2I                 GE_glUniform2i;
extern GE_PFGLUNIFORM3I                 GE_glUniform3i;
extern GE_PFGLUNIFORM4I                 GE_glUniform4i;
extern GE_PFGLUNIFORM1F                 GE_glUniform1f;
extern GE_PFGLUNIFORM2F                 GE_glUniform2f;
extern GE_PFGLUNIFORM3F                 GE_glUniform3f;
extern GE_PFGLUNIFORM4F                 GE_glUniform4f;
extern GE_PFGLUNIFORM1FV                GE_glUniform1fv;
extern GE_PFGLUNIFORM2FV                GE_glUniform2fv;
extern GE_PFGLUNIFORM3FV                GE_glUniform3fv;
extern GE_PFGLUNIFORM4FV                GE_glUniform4fv;
extern GE_PFGLUNIFORMMATRIX4FV          GE_glUniformMatrix4fv;
extern GE_PFGLBINDATTRIBLOCATION        GE_glBindAttribLocation;
extern GE_PFGLGETATTRIBLOCATION         GE_glGetAttribLocation;
extern GE_PFGLVERTEXATTRIBPOINTER       GE_glVertexAttribPointer;
extern GE_PFGLDISABLEVERTEXATTRIBARRAY  GE_glDisableVertexAttribArray;
extern GE_PFGLENABLEVERTEXATTRIBARRAY   GE_glEnableVertexAttribArray;
extern GE_PFGLGETSHADERIV               GE_glGetShaderiv;
extern GE_PFGLGETSHADERINFOLOG          GE_glGetShaderInfoLog;
extern GE_PFGLGETPROGRAMIV              GE_glGetProgramiv;
extern GE_PFGLGETPROGRAMINFOLOG         GE_glGetProgramInfoLog;
#endif

#ifndef GL_EXT_framebuffer_object
extern GE_PFGLGENFRAMEBUFFERS           GE_glGenFramebuffers;
extern GE_PFGLBINDFRAMEBUFFER           GE_glBindFramebuffer;
extern GE_PFGLDELETEFRAMEBUFFERS        GE_glDeleteFramebuffers;
extern GE_PFGLCHECKFRAMEBUFFERSTATUS    GE_glCheckFramebufferStatus;
extern GE_PFGLFRAMEBUFFERTEXTURE2D      GE_glFramebufferTexture2D;
extern GE_PFGLFRAMEBUFFERRENDERBUFFER   GE_glFramebufferRenderbuffer;
extern GE_PFGLISRENDERBUFFER            GE_glIsRenderbuffer;
extern GE_PFGLBINDRENDERBUFFER          GE_glBindRenderbuffer;
extern GE_PFGLDELETERENDERBUFFERS       GE_glDeleteRenderbuffers;
extern GE_PFGLGENRENDERBUFFERS          GE_glGenRenderbuffers;
extern GE_PFGLRENDERBUFFERSTORAGE       GE_glRenderbufferStorage;
#endif

#ifndef GL_ARB_draw_buffers
extern GE_PFGLDRAWBUFFERS               GE_glDrawBuffers;
#endif

#ifndef GL_ARB_occlusion_query
extern GE_PFGLGENQUERIES                GE_glGenQueries;
extern GE_PFGLDELETEQUERIES             GE_glDeleteQueries;
extern GE_PFGLISQUERY                   GE_glIsQuery;
extern GE_PFGLBEGINQUERY                GE_glBeginQuery;
extern GE_PFGLENDQUERY                  GE_glEndQuery;
extern GE_PFGLGETQUERYIV                GE_glGetQueryiv;
extern GE_PFGLGETQUERYOBJECTIV          GE_glGetQueryObjectiv;
extern GE_PFGLGETQUERYOBJECTUIV         GE_glGetQueryObjectuiv;
#endif

#ifndef GL_ARB_vertex_array_object
extern GE_PFGLBINDVERTEXARRAY           GE_glBindVertexArray;
extern GE_PFGLDELETEVERTEXARRAYS        GE_glDeleteVertexArrays;
extern GE_PFGLGENVERTEXARRAYS           GE_glGenVertexArrays;
extern GE_PFGLISVERTEXARRAY             GE_glIsVertexArray;
#endif


/*******************************************************
Function re-routing
********************************************************/

#ifndef GE_NO_EXTENSION_ROUTING

#ifndef GL_VERSION_1_2
#define glDrawRangeElements          GE_glDrawRangeElements
#endif

#ifndef GL_VERSION_1_3
#define glActiveTexture              GE_glActiveTexture
#define glMultiTexCoord2f            GE_glMultiTexCoord2f
#endif

#ifndef GL_VERSION_1_5
#define glGenBuffers                GE_glGenBuffers
#define glBindBuffer                GE_glBindBuffer
#define glBufferData                GE_glBufferData
#define glBufferSubData             GE_glBufferSubData
#endif

#ifndef GL_VERSION_2_0
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
#endif

#ifndef GL_EXT_framebuffer_object
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
#endif

#ifndef GL_ARB_draw_buffers
#define glDrawBuffers               GE_glDrawBuffers
#endif

#ifndef GL_ARB_occlusion_query
#define glGenQueries                GE_glGenQueries
#define glDeleteQueries             GE_glDeleteQueries
#define glIsQuery                   GE_glIsQuery
#define glBeginQuery                GE_glBeginQuery
#define glEndQuery                  GE_glEndQuery
#define glGetQueryiv                GE_glGetQueryiv
#define glGetQueryObjectiv          GE_glGetQueryObjectiv
#define glGetQueryObjectuiv         GE_glGetQueryObjectuiv
#endif

#ifndef GL_ARB_vertex_array_object
#define glBindVertexArray           GE_glBindVertexArray
#define glDeleteVertexArrays        GE_glDeleteVertexArrays
#define glGenVertexArrays           GE_glGenVertexArrays
#define glIsVertexArray             GE_glIsVertexArray
#endif

#endif//!GE_NO_EXTENSION_ROUTING


/*******************************************************
Vertical sync control
********************************************************/

typedef void (APIENTRY *GE_PFGLSWAPINTERVAL) (GLint);
extern GE_PFGLSWAPINTERVAL glSwapInterval;


/*******************************************************
Internal GL renderer parameters
********************************************************/

#define _GL_MAX_SHININESS 128


/****************************************************
Initialization
*****************************************************/

void rvgGLInit();

#endif /* __GEGLHEADERS_H */
