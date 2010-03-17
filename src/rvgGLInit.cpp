#include "rvgMain.h"

#define GE_NO_EXTENSION_ROUTING
#include "rvgGLHeaders.h"

#if !defined(WIN32) && !defined(__APPLE__)
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <X11/Xos.h>
#  include <X11/Xatom.h>
#  include <X11/keysym.h>
#endif

/*
=================================================

Function pointers for undefined extensions

=================================================*/

#if !defined(WIN32) && !defined(__APPLE__)
GE_PFGLXGETPROCADDRESS           GE_glXGetProcAddress = NULL;
#endif

#ifndef GL_VERSION_1_2
GE_PFGLDRAWRANGEELEMENTS         GE_glDrawRangeElements = NULL;
#endif

#ifndef GL_VERSION_1_3
GE_PFGLACTIVETEXTURE             GE_glActiveTexture = NULL;
GE_PFGLMULTITEXCOORD2F           GE_glMultiTexCoord2f = NULL;
#endif

#ifndef GL_VERSION_1_5
GE_PFGLGENBUFFERS                GE_glGenBuffers = NULL;
GE_PFGLBINDBUFFER                GE_glBindBuffer = NULL;
GE_PFGLBUFFERDATA                GE_glBufferData = NULL;
GE_PFGLBUFFERSUBDATA             GE_glBufferSubData = NULL;
#endif

#ifndef GL_VERSION_2_0
GE_PFGLCREATEPROGRAM             GE_glCreateProgram = NULL;
GE_PFGLCREATESHADER              GE_glCreateShader = NULL;
GE_PFGLATTACHSHADER              GE_glAttachShader = NULL;
GE_PFGLSHADERSOURCE              GE_glShaderSource = NULL;
GE_PFGLCOMPILESHADER             GE_glCompileShader = NULL;
GE_PFGLLINKPROGRAM               GE_glLinkProgram = NULL;
GE_PFGLUSEPROGRAM                GE_glUseProgram = NULL;
GE_PFGLDETACHSHADER              GE_glDetachShader = NULL;
GE_PFGLDELETESHADER              GE_glDeleteShader = NULL;
GE_PFGLDELETEPROGRAMSARB         GE_glDeleteProgramsARB = NULL;
GE_PFGLGETINFOLOGARB             GE_glGetInfoLogARB = NULL;
GE_PFGLGETOBJECTPARAMETERIVARB   GE_glGetObjectParameterivARB = NULL;
GE_PFGLGETUNIFORMLOCATION        GE_glGetUniformLocation = NULL;
GE_PFGLUNIFORM1I                 GE_glUniform1i = NULL;
GE_PFGLUNIFORM2I                 GE_glUniform2i = NULL;
GE_PFGLUNIFORM3I                 GE_glUniform3i = NULL;
GE_PFGLUNIFORM4I                 GE_glUniform4i = NULL;
GE_PFGLUNIFORM1F                 GE_glUniform1f = NULL;
GE_PFGLUNIFORM2F                 GE_glUniform2f = NULL;
GE_PFGLUNIFORM3F                 GE_glUniform3f = NULL;
GE_PFGLUNIFORM4F                 GE_glUniform4f = NULL;
GE_PFGLUNIFORM1FV                GE_glUniform1fv = NULL;
GE_PFGLUNIFORM2FV                GE_glUniform2fv = NULL;
GE_PFGLUNIFORM3FV                GE_glUniform3fv = NULL;
GE_PFGLUNIFORM4FV                GE_glUniform4fv = NULL;
GE_PFGLUNIFORMMATRIX4FV          GE_glUniformMatrix4fv = NULL;
GE_PFGLBINDATTRIBLOCATION        GE_glBindAttribLocation = NULL;
GE_PFGLGETATTRIBLOCATION         GE_glGetAttribLocation = NULL;
GE_PFGLVERTEXATTRIBPOINTER       GE_glVertexAttribPointer = NULL;
GE_PFGLDISABLEVERTEXATTRIBARRAY  GE_glDisableVertexAttribArray = NULL;
GE_PFGLENABLEVERTEXATTRIBARRAY   GE_glEnableVertexAttribArray = NULL;
GE_PFGLGETSHADERIV               GE_glGetShaderiv = NULL;
GE_PFGLGETSHADERINFOLOG          GE_glGetShaderInfoLog = NULL;
GE_PFGLGETPROGRAMIV              GE_glGetProgramiv = NULL;
GE_PFGLGETPROGRAMINFOLOG         GE_glGetProgramInfoLog = NULL;
#endif

#ifndef GL_EXT_framebuffer_object
GE_PFGLGENFRAMEBUFFERS           GE_glGenFramebuffers = NULL;
GE_PFGLBINDFRAMEBUFFER           GE_glBindFramebuffer = NULL;
GE_PFGLDELETEFRAMEBUFFERS        GE_glDeleteFramebuffers = NULL;
GE_PFGLCHECKFRAMEBUFFERSTATUS    GE_glCheckFramebufferStatus = NULL;
GE_PFGLFRAMEBUFFERTEXTURE2D      GE_glFramebufferTexture2D = NULL;
GE_PFGLFRAMEBUFFERRENDERBUFFER   GE_glFramebufferRenderbuffer = NULL;
GE_PFGLISRENDERBUFFER            GE_glIsRenderbuffer = NULL;
GE_PFGLBINDRENDERBUFFER          GE_glBindRenderbuffer = NULL;
GE_PFGLDELETERENDERBUFFERS       GE_glDeleteRenderbuffers = NULL;
GE_PFGLGENRENDERBUFFERS          GE_glGenRenderbuffers = NULL;
GE_PFGLRENDERBUFFERSTORAGE       GE_glRenderbufferStorage = NULL;
#endif

#ifndef GL_ARB_draw_buffers
GE_PFGLDRAWBUFFERS               GE_glDrawBuffers = NULL;
#endif

#ifndef GL_ARB_occlusion_query
GE_PFGLGENQUERIES                GE_glGenQueries = NULL;
GE_PFGLDELETEQUERIES             GE_glDeleteQueries = NULL;
GE_PFGLISQUERY                   GE_glIsQuery = NULL;
GE_PFGLBEGINQUERY                GE_glBeginQuery = NULL;
GE_PFGLENDQUERY                  GE_glEndQuery = NULL;
GE_PFGLGETQUERYIV                GE_glGetQueryiv = NULL;
GE_PFGLGETQUERYOBJECTIV          GE_glGetQueryObjectiv = NULL;
GE_PFGLGETQUERYOBJECTUIV         GE_glGetQueryObjectuiv = NULL;
#endif

#ifndef GL_ARB_vertex_array_object
GE_PFGLBINDVERTEXARRAY           GE_glBindVertexArray = NULL;
GE_PFGLDELETEVERTEXARRAYS        GE_glDeleteVertexArrays = NULL;
GE_PFGLGENVERTEXARRAYS           GE_glGenVertexArrays = NULL;
GE_PFGLISVERTEXARRAY             GE_glIsVertexArray = NULL;
#endif

GE_PFGLSWAPINTERVAL glSwapInterval = NULL;

int textureUnits;
bool hasMultitexture;
bool hasShaderObjects;
bool hasFramebufferObjects;
bool hasVertexBufferObjects;
bool hasVertexArrayObjects;
bool hasMultipleRenderTargets;
bool hasDepthStencilFormat;
bool hasRangeElements;
bool hasOcclusionQuery;
int maxOcclusionBits;
int maxRenderTargets;
int maxElementsIndices;
int maxElementsVertices;

/*
--------------------------------------------------
Returns address for a named procedure natively
--------------------------------------------------*/

static PFVOID getProcAddress(const char *name)
{
  #if defined(_WIN32)
  return (PFVOID) wglGetProcAddress (name);
  #elif defined(__APPLE__)
  //TODO: Mac OS glGetProcAddress implementation
  return (PFVOID) NULL;
  #else
  return GE_glXGetProcAddress ((GLubyte*) name);
  #endif
}

/*
--------------------------------------------------
Returns true if named extension is supported
--------------------------------------------------*/

static int checkExtension(const char *extensions, const char *name)
{
  int nlen = (int)strlen(name);
  int elen = (int)strlen(extensions);
  const char *e = extensions;
  assert(nlen > 0);

  while (1) {

    //Try to find sub-string
    e = strstr(e, name);
    if (e == NULL) return 0;
    //Check if last
    if (e == extensions + elen - nlen)
      return 1;
      //Check if space follows (avoid same names with a suffix)
    if (*(e + nlen) == ' ')
      return 1;

    e += nlen;
  }

  return 0;
}

/*
--------------------------------------------------------
Loads all the used OpenGL extensions. The extension
names are first searched for in the opengl extensions
string to check whether they are supported by the
hardware. The missing procedure addresses are then
queried for if not defined in the opengl headers.
--------------------------------------------------------*/

void rvgGLInit()
{
  /*
  Check for glXGetProcAddress
  *****************************************/
  
  #if !defined(WIN32) && !defined(__APPLE__)
  
  void *dlhandle = dlopen (NULL, RTLD_LAZY);
  ASSERT (dlhandle != NULL);
  
  GE_glXGetProcAddress =
    (GE_PFGLXGETPROCADDRESS) dlsym (dlhandle, "glXGetProcAddress");
  
  if (GE_glXGetProcAddress == NULL)
  {
    GE_glXGetProcAddress =
      (GE_PFGLXGETPROCADDRESS) dlsym (dlhandle, "glXGetProcAddressARB");
    
    ASSERT (GE_glXGetProcAddress != NULL);
  }
  
  #endif
  
  
  /*
  Obtain supported extension string
  *****************************************/
  
  const char *ext = (const char*) glGetString (GL_EXTENSIONS);
  
  /*
  Check elements range
  *****************************************/
  
  if (checkExtension(ext, "GL_EXT_draw_range_elements")) {
    hasRangeElements = true;
    
    #ifndef GL_VERSION_1_2
    GE_glDrawRangeElements = (GE_PFGLDRAWRANGEELEMENTS)
      getProcAddress ("glDrawRangeElementsEXT");
    
    if (GE_glDrawRangeElements==NULL)
      hasRangeElements = false;
    #endif
    
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxElementsIndices);
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxElementsVertices);
    
  }else{ hasRangeElements = false; }
  
  /*
  Check multitexturing
  *****************************************/
  
  if (checkExtension(ext, "GL_ARB_multitexture")) {
    hasMultitexture = true;

    #ifndef GL_VERSION_1_3
    GE_glActiveTexture = (GE_PFGLACTIVETEXTURE)
      getProcAddress ("glActiveTextureARB");
    GE_glMultiTexCoord2f = (GE_PFGLMULTITEXCOORD2F)
      getProcAddress ("glMultiTexCoord2fARB");
    
    if (GE_glActiveTexture==NULL || GE_glMultiTexCoord2f==NULL)
      hasMultitexture = false;
    #endif

  }else{ hasMultitexture = false; }

  /*
  Check vertex buffer objects
  *****************************************/

  if (checkExtension(ext, "GL_ARB_vertex_buffer_object")) {
    hasVertexBufferObjects = true;

    #ifndef GL_VERSION_1_5
    GE_glGenBuffers = (GE_PFGLGENBUFFERS)
      getProcAddress("glGenBuffersARB");
    GE_glBindBuffer = (GE_PFGLBINDBUFFER)
      getProcAddress("glBindBufferARB");
    GE_glBufferData = (GE_PFGLBUFFERDATA)
      getProcAddress("glBufferDataARB");
    GE_glBufferSubData = (GE_PFGLBUFFERSUBDATA)
      getProcAddress("glBufferSubDataARB");

    if (GE_glGenBuffers==NULL || GE_glBindBuffer==NULL ||
        GE_glBufferData==NULL || GE_glBufferSubData==NULL)
      hasVertexBufferObjects = false;
    #endif
  
  }else{ hasVertexBufferObjects = false; }
  
  /*
  Check shader objects
  *****************************************/
  
  if (checkExtension(ext, "GL_ARB_shader_objects") &&
      checkExtension(ext, "GL_ARB_vertex_program") &&
      checkExtension(ext, "GL_ARB_fragment_program")) {
    hasShaderObjects = true;
    
    #ifndef GL_VERSION_2_0
    GE_glCreateProgram = (GE_PFGLCREATEPROGRAM)
      getProcAddress ("glCreateProgramObjectARB");
    GE_glCreateShader = (GE_PFGLCREATESHADER)
      getProcAddress ("glCreateShaderObjectARB");
    GE_glAttachShader = (GE_PFGLATTACHSHADER)
      getProcAddress ("glAttachObjectARB");
    GE_glShaderSource = (GE_PFGLSHADERSOURCE)
      getProcAddress ("glShaderSourceARB");
    GE_glCompileShader = (GE_PFGLCOMPILESHADER)
      getProcAddress ("glCompileShaderARB");
    GE_glLinkProgram = (GE_PFGLLINKPROGRAM)
      getProcAddress ("glLinkProgramARB");
    GE_glUseProgram = (GE_PFGLUSEPROGRAM)
      getProcAddress ("glUseProgramObjectARB");
    GE_glDetachShader = (GE_PFGLDETACHSHADER)
      getProcAddress ("glDetachObjectARB");
    GE_glDeleteShader = (GE_PFGLDELETESHADER)
      getProcAddress ("glDeleteObjectARB");
    GE_glDeleteProgramsARB = (GE_PFGLDELETEPROGRAMSARB)
      getProcAddress ("glDeleteProgramsARB");
    GE_glGetInfoLogARB = (GE_PFGLGETINFOLOGARB)
      getProcAddress ("glGetInfoLogARB");
    GE_glGetObjectParameterivARB = (GE_PFGLGETOBJECTPARAMETERIVARB)
      getProcAddress ("glGetObjectParameterivARB");
    GE_glGetUniformLocation = (GE_PFGLGETUNIFORMLOCATION)
      getProcAddress ("glGetUniformLocationARB");
    GE_glUniform1i = (GE_PFGLUNIFORM1I)
      getProcAddress ("glUniform1iARB");
    GE_glUniform2i = (GE_PFGLUNIFORM2I)
      getProcAddress ("glUniform2iARB");
    GE_glUniform3i = (GE_PFGLUNIFORM3I)
      getProcAddress ("glUniform3iARB");
    GE_glUniform4i = (GE_PFGLUNIFORM4I)
      getProcAddress ("glUniform4iARB");
    GE_glUniform1f = (GE_PFGLUNIFORM1F)
      getProcAddress ("glUniform1fARB");
    GE_glUniform2f = (GE_PFGLUNIFORM2F)
      getProcAddress ("glUniform2fARB");
    GE_glUniform3f = (GE_PFGLUNIFORM3F)
      getProcAddress ("glUniform3fARB");
    GE_glUniform4f = (GE_PFGLUNIFORM4F)
      getProcAddress ("glUniform4fARB");
    GE_glUniform1fv = (GE_PFGLUNIFORM1FV)
      getProcAddress ("glUniform1fvARB");
    GE_glUniform2fv = (GE_PFGLUNIFORM2FV)
      getProcAddress ("glUniform2fvARB");
    GE_glUniform3fv = (GE_PFGLUNIFORM3FV)
      getProcAddress ("glUniform3fvARB");
    GE_glUniform4fv = (GE_PFGLUNIFORM4FV)
      getProcAddress ("glUniform4fvARB");
    GE_glUniformMatrix4fv = (GE_PFGLUNIFORMMATRIX4FV)
      getProcAddress ("glUniformMatrix4fvARB");
    GE_glBindAttribLocation = (GE_PFGLBINDATTRIBLOCATION)
      getProcAddress ("glBindAttribLocationARB");
    GE_glGetAttribLocation = (GE_PFGLGETATTRIBLOCATION)
      getProcAddress ("glGetAttribLocationARB");
    GE_glVertexAttribPointer = (GE_PFGLVERTEXATTRIBPOINTER)
      getProcAddress ("glVertexAttribPointerARB");
    GE_glDisableVertexAttribArray = (GE_PFGLDISABLEVERTEXATTRIBARRAY)
      getProcAddress ("glDisableVertexAttribArrayARB");
    GE_glEnableVertexAttribArray = (GE_PFGLENABLEVERTEXATTRIBARRAY)
      getProcAddress ("glEnableVertexAttribArrayARB");
    GE_glGetShaderiv = (GE_PFGLGETSHADERIV)
      getProcAddress ("glGetShaderiv");
    GE_glGetShaderInfoLog = (GE_PFGLGETSHADERINFOLOG)
      getProcAddress ("glGetShaderInfoLog");
    GE_glGetProgramiv = (GE_PFGLGETPROGRAMIV)
      getProcAddress ("glGetProgramiv");
    GE_glGetProgramInfoLog = (GE_PFGLGETPROGRAMINFOLOG)
      getProcAddress ("glGetProgramInfoLog");
      
    
    if (GE_glCreateProgram==NULL || GE_glCreateShader==NULL ||
        GE_glCreateShader==NULL || GE_glAttachShader==NULL ||
        GE_glShaderSource==NULL || GE_glCompileShader==NULL ||
        GE_glLinkProgram==NULL || GE_glUseProgram==NULL ||
        GE_glUseProgram==NULL || GE_glDetachShader==NULL ||
        GE_glDeleteShader==NULL || GE_glDeleteProgramsARB==NULL ||
        GE_glGetInfoLogARB==NULL || GE_glGetObjectParameterivARB==NULL ||
        GE_glGetUniformLocation==NULL ||
        GE_glUniform1i==NULL || GE_glUniform2i==NULL ||
        GE_glUniform3i==NULL || GE_glUniform4i==NULL ||
        GE_glUniform1f==NULL || GE_glUniform2f==NULL ||
        GE_glUniform3f==NULL || GE_glUniform4f==NULL ||
        GE_glUniform1fv==NULL || GE_glUniform2fv==NULL ||
        GE_glUniform3fv==NULL || GE_glUniform4fv==NULL ||
        GE_glUniformMatrix4fv==NULL || GE_glBindAttribLocation==NULL ||
        GE_glGetAttribLocation==NULL || GE_glVertexAttribPointer==NULL ||
        GE_glDisableVertexAttribArray==NULL || GE_glEnableVertexAttribArray==NULL)
      hasShaderObjects = false;
    #endif
    
  }else{ hasShaderObjects = false; }

  /*
  Check framebuffer objects
  *****************************************/

  if (checkExtension(ext, "GL_EXT_framebuffer_object")) {
    hasFramebufferObjects = true;
    
    #ifndef GL_EXT_framebuffer_object
    GE_glGenFramebuffers = (GE_PFGLGENFRAMEBUFFERS)
      getProcAddress ("glGenFramebuffersEXT");
    GE_glBindFramebuffer = (GE_PFGLBINDFRAMEBUFFER)
      getProcAddress ("glBindFramebufferEXT");
    GE_glDeleteFramebuffers = (GE_PFGLDELETEFRAMEBUFFERS)
      getProcAddress ("glDeleteFramebuffersEXT");
    GE_glCheckFramebufferStatus = (GE_PFGLCHECKFRAMEBUFFERSTATUS)
      getProcAddress ("glCheckFramebufferStatusEXT");
    GE_glFramebufferTexture2D = (GE_PFGLFRAMEBUFFERTEXTURE2D)
      getProcAddress ("glFramebufferTexture2DEXT");
    GE_glFramebufferRenderbuffer = (GE_PFGLFRAMEBUFFERRENDERBUFFER)
      getProcAddress ("glFramebufferRenderbufferEXT");
    GE_glIsRenderbuffer = (GE_PFGLISRENDERBUFFER)
      getProcAddress ("glIsRenderbufferEXT");
    GE_glBindRenderbuffer = (GE_PFGLBINDRENDERBUFFER)
      getProcAddress ("glBindRenderbufferEXT");
    GE_glDeleteRenderbuffers = (GE_PFGLDELETERENDERBUFFERS)
      getProcAddress ("glDeleteRenderbuffersEXT");
    GE_glGenRenderbuffers = (GE_PFGLGENRENDERBUFFERS)
      getProcAddress ("glGenRenderbuffersEXT");
    GE_glRenderbufferStorage = (GE_PFGLRENDERBUFFERSTORAGE)
      getProcAddress ("glRenderbufferStorageEXT");

    if (GE_glGenFramebuffers==NULL || GE_glBindFramebuffer==NULL ||
        GE_glBindFramebuffer==NULL || GE_glDeleteFramebuffers==NULL ||
        GE_glCheckFramebufferStatus==NULL || GE_glFramebufferTexture2D==NULL ||
        GE_glFramebufferRenderbuffer==NULL || GE_glIsRenderbuffer==NULL ||
        GE_glBindRenderbuffer==NULL || GE_glDeleteRenderbuffers==NULL ||
        GE_glGenRenderbuffers==NULL || GE_glRenderbufferStorage==NULL)
      hasFramebufferObjects = false;
    #endif

  }else{ hasFramebufferObjects = false; }

  /*
  Check multiple render targets
  *****************************************/

  if (checkExtension( ext, "GL_ARB_draw_buffers" )) {
    hasMultipleRenderTargets = true;
    glGetIntegerv( GL_MAX_DRAW_BUFFERS, &maxRenderTargets );

    #ifndef GL_ARB_draw_buffers
    GE_glDrawBuffers = (GE_PFGLDRAWBUFFERS)
      getProcAddress( "glDrawBuffersARB" );

    if (GE_glDrawBuffers==NULL)
      hasMultipleRenderTargets = false;
    #endif

  }else{ hasMultipleRenderTargets = false; }

  /*
  Check for packed depth-stencil format
  *****************************************/

  if (checkExtension( ext, "GL_EXT_packed_depth_stencil" )) {
    hasDepthStencilFormat = true;
  }else{ hasDepthStencilFormat = false; }

  /*
  Check for occlusion query
  *****************************************/

  if (checkExtension( ext, "GL_ARB_occlusion_query" )) {
    hasOcclusionQuery = true;

    #ifndef GL_ARB_occlusion_query
    GE_glGenQueries = (GE_PFGLGENQUERIES)
      getProcAddress( "glGenQueriesARB" );
    GE_glDeleteQueries = (GE_PFGLDELETEQUERIES)
      getProcAddress( "glDeleteQueriesARB" );
    GE_glIsQuery = (GE_PFGLISQUERY)
      getProcAddress( "glIsQueryARB" );
    GE_glBeginQuery = (GE_PFGLBEGINQUERY)
      getProcAddress( "glBeginQueryARB" );
    GE_glEndQuery = (GE_PFGLENDQUERY)
      getProcAddress( "glEndQueryARB" );
    GE_glGetQueryiv = (GE_PFGLGETQUERYIV)
      getProcAddress( "glGetQueryivARB" );
    GE_glGetQueryObjectiv = (GE_PFGLGETQUERYOBJECTIV)
      getProcAddress( "glGetQueryObjectivARB" );
    GE_glGetQueryObjectuiv = (GE_PFGLGETQUERYOBJECTUIV)
      getProcAddress( "glGetQueryObjectuivARB" );

    if (GE_glGenQueries==NULL || GE_glDeleteQueries==NULL ||
        GE_glIsQuery==NULL || GE_glBeginQuery==NULL ||
        GE_glEndQuery==NULL || GE_glGetQueryiv==NULL ||
        GE_glGetQueryObjectiv==NULL || GE_glGetQueryObjectuiv==NULL)
      hasOcclusionQuery = false;
    #endif

    maxOcclusionBits = 0;
    if (hasOcclusionQuery)
      GE_glGetQueryiv( GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &maxOcclusionBits );

  }else{ hasOcclusionQuery = false; }

  /*
  Check for vertex array object
  *****************************************/

  if (checkExtension( ext, "GL_ARB_vertex_array_object" )) {
    hasVertexArrayObjects = true;

    #ifndef GL_ARB_vertex_array_object
    GE_glBindVertexArray = (GE_PFGLBINDVERTEXARRAY)
      getProcAddress( "glBindVertexArray" );
    GE_glDeleteVertexArrays = (GE_PFGLDELETEVERTEXARRAYS)
      getProcAddress( "glDeleteVertexArrays" );
    GE_glGenVertexArrays = (GE_PFGLGENVERTEXARRAYS)
      getProcAddress( "glGenVertexArrays" );
    GE_glIsVertexArray = (GE_PFGLISVERTEXARRAY)
      getProcAddress( "glIsVertexArray" );

    if (GE_glBindVertexArray==NULL || GE_glIsVertexArray==NULL ||
        GE_glGenVertexArrays==NULL || GE_glDeleteVertexArrays==NULL)
      hasVertexArrayObjects = false;
    #endif

  }else{ hasVertexArrayObjects = false; }

  /*
  Check vertical sync control
  *****************************************/
  
  #if defined(WIN32)
  glSwapInterval = (GE_PFGLSWAPINTERVAL)
    getProcAddress("wglSwapIntervalEXT");
  #endif
  
  /*
  Report
  *****************************************/
  printf( "hasMultitexture: %s\n", (hasMultitexture ? "true" : "false" ));
  printf( "hasShaderObjects: %s\n", (hasShaderObjects ? "true" : "false" ));
  printf( "hasFramebufferObjects: %s\n", (hasFramebufferObjects ? "true" : "false" ));
  printf( "hasVertexBufferObjects: %s\n", (hasVertexBufferObjects ? "true" : "false" ));
  printf( "hasMultipleRenderTargets: %s\n", (hasMultipleRenderTargets ? "true" : "false" ));
  printf( "hasDepthStencilFormat: %s\n", (hasDepthStencilFormat ? "true" : "false" ));
  printf( "hasRangeElements: %s\n", (hasRangeElements ? "true" : "false" ));
  printf( "hasOcclusionQuery: %s\n", (hasOcclusionQuery ? "true" : "false" ));
  printf( "maxRenderTargets: %d\n", maxRenderTargets );
  printf( "maxElementsVertices: %d\n", maxElementsVertices );
  printf( "maxElementsIndices: %d\n", maxElementsIndices );
  printf( "maxOcclusionBits: %d\n", maxOcclusionBits);
}
