#include "rvgDefs.h"

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

//GL_VERSION_1_2
PFNGLDRAWRANGEELEMENTSPROC         GE_glDrawRangeElements = NULL;
PFNGLTEXIMAGE3DPROC                GE_glTexImage3D = NULL;
PFNGLTEXSUBIMAGE3DEXTPROC          GE_glTexSubImage3D = NULL;

//GL_VERSION_1_3
PFNGLACTIVETEXTUREPROC             GE_glActiveTexture = NULL;
PFNGLMULTITEXCOORD2FPROC           GE_glMultiTexCoord2f = NULL;

//GL_VERSION_1_5
PFNGLGENBUFFERSPROC                GE_glGenBuffers = NULL;
PFNGLDELETEBUFFERSPROC             GE_glDeleteBuffers = NULL;
PFNGLBINDBUFFERPROC                GE_glBindBuffer = NULL;
PFNGLBUFFERDATAPROC                GE_glBufferData = NULL;
PFNGLBUFFERSUBDATAPROC             GE_glBufferSubData = NULL;
PFNGLMAPBUFFERPROC                 GE_glMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC               GE_glUnmapBuffer = NULL;

//GL_VERSION_2_0
PFNGLCREATEPROGRAMPROC             GE_glCreateProgram = NULL;
PFNGLCREATESHADERPROC              GE_glCreateShader = NULL;
PFNGLATTACHSHADERPROC              GE_glAttachShader = NULL;
PFNGLSHADERSOURCEPROC              GE_glShaderSource = NULL;
PFNGLCOMPILESHADERPROC             GE_glCompileShader = NULL;
PFNGLLINKPROGRAMPROC               GE_glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC                GE_glUseProgram = NULL;
PFNGLDETACHSHADERPROC              GE_glDetachShader = NULL;
PFNGLDELETESHADERPROC              GE_glDeleteShader = NULL;
PFNGLDELETEPROGRAMSARBPROC         GE_glDeleteProgramsARB = NULL;
PFNGLGETINFOLOGARBPROC             GE_glGetInfoLogARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC   GE_glGetObjectParameterivARB = NULL;
PFNGLGETUNIFORMLOCATIONPROC        GE_glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC                 GE_glUniform1i = NULL;
PFNGLUNIFORM2IPROC                 GE_glUniform2i = NULL;
PFNGLUNIFORM3IPROC                 GE_glUniform3i = NULL;
PFNGLUNIFORM4IPROC                 GE_glUniform4i = NULL;
PFNGLUNIFORM1FPROC                 GE_glUniform1f = NULL;
PFNGLUNIFORM2FPROC                 GE_glUniform2f = NULL;
PFNGLUNIFORM3FPROC                 GE_glUniform3f = NULL;
PFNGLUNIFORM4FPROC                 GE_glUniform4f = NULL;
PFNGLUNIFORM1FVPROC                GE_glUniform1fv = NULL;
PFNGLUNIFORM2FVPROC                GE_glUniform2fv = NULL;
PFNGLUNIFORM3FVPROC                GE_glUniform3fv = NULL;
PFNGLUNIFORM4FVPROC                GE_glUniform4fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC          GE_glUniformMatrix4fv = NULL;
PFNGLBINDATTRIBLOCATIONPROC        GE_glBindAttribLocation = NULL;
PFNGLGETATTRIBLOCATIONPROC         GE_glGetAttribLocation = NULL;
PFNGLVERTEXATTRIBPOINTERPROC       GE_glVertexAttribPointer = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC  GE_glDisableVertexAttribArray = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC   GE_glEnableVertexAttribArray = NULL;
PFNGLGETSHADERIVPROC               GE_glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC          GE_glGetShaderInfoLog = NULL;
PFNGLGETPROGRAMIVPROC              GE_glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC         GE_glGetProgramInfoLog = NULL;

//GL_EXT_framebuffer_object
PFNGLGENFRAMEBUFFERSPROC           GE_glGenFramebuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC           GE_glBindFramebuffer = NULL;
PFNGLDELETEFRAMEBUFFERSPROC        GE_glDeleteFramebuffers = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC    GE_glCheckFramebufferStatus = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC      GE_glFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC   GE_glFramebufferRenderbuffer = NULL;
PFNGLISRENDERBUFFERPROC            GE_glIsRenderbuffer = NULL;
PFNGLBINDRENDERBUFFERPROC          GE_glBindRenderbuffer = NULL;
PFNGLDELETERENDERBUFFERSPROC       GE_glDeleteRenderbuffers = NULL;
PFNGLGENRENDERBUFFERSPROC          GE_glGenRenderbuffers = NULL;
PFNGLRENDERBUFFERSTORAGEPROC       GE_glRenderbufferStorage = NULL;

//GL_ARB_draw_buffers
PFNGLDRAWBUFFERSPROC               GE_glDrawBuffers = NULL;

//GL_ARB_occlusion_query
PFNGLGENQUERIESPROC                GE_glGenQueries = NULL;
PFNGLDELETEQUERIESPROC             GE_glDeleteQueries = NULL;
PFNGLISQUERYPROC                   GE_glIsQuery = NULL;
PFNGLBEGINQUERYPROC                GE_glBeginQuery = NULL;
PFNGLENDQUERYPROC                  GE_glEndQuery = NULL;
PFNGLGETQUERYIVPROC                GE_glGetQueryiv = NULL;
PFNGLGETQUERYOBJECTIVPROC          GE_glGetQueryObjectiv = NULL;
PFNGLGETQUERYOBJECTUIVPROC         GE_glGetQueryObjectuiv = NULL;

//GL_ARB_vertex_array_object
PFNGLBINDVERTEXARRAYPROC           GE_glBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC        GE_glDeleteVertexArrays = NULL;
PFNGLGENVERTEXARRAYSPROC           GE_glGenVertexArrays = NULL;
PFNGLISVERTEXARRAYPROC             GE_glIsVertexArray = NULL;

//GL_VERSION_3_0
PFNGLVERTEXATTRIBIPOINTERPROC      GE_glVertexAttribIPointer = NULL;

//GL_EXT_shader_image_load_store
PFNGLBINDIMAGETEXTUREEXTPROC       GE_glBindImageTexture = NULL;
PFNGLMEMORYBARRIEREXTPROC          GE_glMemoryBarrier = NULL;

//GL_NV_shader_buffer_load
PFNGLMAKEBUFFERRESIDENTNVPROC        GE_glMakeBufferResident = NULL;
PFNGLMAKEBUFFERNONRESIDENTNVPROC     GE_glMakeBufferNonResident = NULL;
PFNGLGETBUFFERPARAMETERUI64VNVPROC   GE_glGetBufferParameterui64v = NULL;
PFNGLUNIFORMUI64NVPROC               GE_glUniformui64 = NULL;

//GL_ARB_sample_shading
PFNGLMINSAMPLESHADINGARBPROC         GE_glMinSampleShading = NULL;

//Vertical sync
PFNWGLSWAPINTERVALEXTPROC          GE_wglSwapInterval = NULL;

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
  if (!extensions) return true;

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
    (PFNGLXGETPROCADDRESSPROC) dlsym (dlhandle, "glXGetProcAddress");
  
  if (GE_glXGetProcAddress == NULL)
  {
    GE_glXGetProcAddress =
      (PFNGLXGETPROCADDRESSPROC) dlsym (dlhandle, "glXGetProcAddressARB");
    
    ASSERT (GE_glXGetProcAddress != NULL);
  }
  
  #endif
  
  
  /*
  Obtain supported extension string
  *****************************************/
  
  const char *ext = (const char*) glGetString (GL_EXTENSIONS);
  /*if (!ext) {
    std::cout << "Failed obtaining extensions string!" << std::endl;
    std::getchar();
    exit( 0 );
  }*/
  
  /*
  Check 1.2 features
  ******************************************/

  GE_glTexImage3D = (PFNGLTEXIMAGE3DPROC)
    getProcAddress( "glTexImage3D" );

  GE_glTexSubImage3D = (PFNGLTEXSUBIMAGE3DEXTPROC)
    getProcAddress( "glTexSubImage3D" );

  /*
  Check elements range
  *****************************************/
  
  if (checkExtension(ext, "GL_EXT_draw_range_elements")) {
    hasRangeElements = true;
    
    GE_glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)
      getProcAddress ("glDrawRangeElementsEXT");
    
    if (GE_glDrawRangeElements==NULL)
      hasRangeElements = false;
    
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxElementsIndices);
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxElementsVertices);
    
  }else{ hasRangeElements = false; }
  
  /*
  Check multitexturing
  *****************************************/
  
  if (checkExtension(ext, "GL_ARB_multitexture")) {
    hasMultitexture = true;

    GE_glActiveTexture = (PFNGLACTIVETEXTUREPROC)
      getProcAddress ("glActiveTextureARB");
    GE_glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)
      getProcAddress ("glMultiTexCoord2fARB");
    
    if (GE_glActiveTexture==NULL || GE_glMultiTexCoord2f==NULL)
      hasMultitexture = false;

  }else{ hasMultitexture = false; }

  /*
  Check vertex buffer objects
  *****************************************/

  if (checkExtension(ext, "GL_ARB_vertex_buffer_object")) {
    hasVertexBufferObjects = true;

    GE_glGenBuffers = (PFNGLGENBUFFERSPROC)
      getProcAddress("glGenBuffersARB");
    GE_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
      getProcAddress("glDeleteBuffersARB");
    GE_glBindBuffer = (PFNGLBINDBUFFERPROC)
      getProcAddress("glBindBufferARB");
    GE_glBufferData = (PFNGLBUFFERDATAPROC)
      getProcAddress("glBufferDataARB");
    GE_glBufferSubData = (PFNGLBUFFERSUBDATAPROC)
      getProcAddress("glBufferSubDataARB");
    GE_glMapBuffer = (PFNGLMAPBUFFERPROC)
      getProcAddress( "glMapBufferARB" );
    GE_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)
      getProcAddress( "glUnmapBufferARB" );

    if (GE_glGenBuffers==NULL || GE_glBindBuffer==NULL ||
        GE_glBufferData==NULL || GE_glBufferSubData==NULL)
      hasVertexBufferObjects = false;
  
  }else{ hasVertexBufferObjects = false; }
  
  /*
  Check shader objects
  *****************************************/
  
  if (checkExtension(ext, "GL_ARB_shader_objects") &&
      checkExtension(ext, "GL_ARB_vertex_program") &&
      checkExtension(ext, "GL_ARB_fragment_program")) {
    hasShaderObjects = true;
    
    GE_glCreateProgram = (PFNGLCREATEPROGRAMPROC)
      getProcAddress ("glCreateProgramObjectARB");
    GE_glCreateShader = (PFNGLCREATESHADERPROC)
      getProcAddress ("glCreateShaderObjectARB");
    GE_glAttachShader = (PFNGLATTACHSHADERPROC)
      getProcAddress ("glAttachObjectARB");
    GE_glShaderSource = (PFNGLSHADERSOURCEPROC)
      getProcAddress ("glShaderSourceARB");
    GE_glCompileShader = (PFNGLCOMPILESHADERPROC)
      getProcAddress ("glCompileShaderARB");
    GE_glLinkProgram = (PFNGLLINKPROGRAMPROC)
      getProcAddress ("glLinkProgramARB");
    GE_glUseProgram = (PFNGLUSEPROGRAMPROC)
      getProcAddress ("glUseProgramObjectARB");
    GE_glDetachShader = (PFNGLDETACHSHADERPROC)
      getProcAddress ("glDetachObjectARB");
    GE_glDeleteShader = (PFNGLDELETESHADERPROC)
      getProcAddress ("glDeleteObjectARB");
    GE_glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)
      getProcAddress ("glDeleteProgramsARB");
    GE_glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)
      getProcAddress ("glGetInfoLogARB");
    GE_glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)
      getProcAddress ("glGetObjectParameterivARB");
    GE_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)
      getProcAddress ("glGetUniformLocationARB");
    GE_glUniform1i = (PFNGLUNIFORM1IPROC)
      getProcAddress ("glUniform1iARB");
    GE_glUniform2i = (PFNGLUNIFORM2IPROC)
      getProcAddress ("glUniform2iARB");
    GE_glUniform3i = (PFNGLUNIFORM3IPROC)
      getProcAddress ("glUniform3iARB");
    GE_glUniform4i = (PFNGLUNIFORM4IPROC)
      getProcAddress ("glUniform4iARB");
    GE_glUniform1f = (PFNGLUNIFORM1FPROC)
      getProcAddress ("glUniform1fARB");
    GE_glUniform2f = (PFNGLUNIFORM2FPROC)
      getProcAddress ("glUniform2fARB");
    GE_glUniform3f = (PFNGLUNIFORM3FPROC)
      getProcAddress ("glUniform3fARB");
    GE_glUniform4f = (PFNGLUNIFORM4FPROC)
      getProcAddress ("glUniform4fARB");
    GE_glUniform1fv = (PFNGLUNIFORM1FVPROC)
      getProcAddress ("glUniform1fvARB");
    GE_glUniform2fv = (PFNGLUNIFORM2FVPROC)
      getProcAddress ("glUniform2fvARB");
    GE_glUniform3fv = (PFNGLUNIFORM3FVPROC)
      getProcAddress ("glUniform3fvARB");
    GE_glUniform4fv = (PFNGLUNIFORM4FVPROC)
      getProcAddress ("glUniform4fvARB");
    GE_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)
      getProcAddress ("glUniformMatrix4fvARB");
    GE_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)
      getProcAddress ("glBindAttribLocationARB");
    GE_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)
      getProcAddress ("glGetAttribLocationARB");
    GE_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)
      getProcAddress ("glVertexAttribPointerARB");
    GE_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)
      getProcAddress ("glDisableVertexAttribArrayARB");
    GE_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)
      getProcAddress ("glEnableVertexAttribArrayARB");
    GE_glGetShaderiv = (PFNGLGETSHADERIVPROC)
      getProcAddress ("glGetShaderiv");
    GE_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)
      getProcAddress ("glGetShaderInfoLog");
    GE_glGetProgramiv = (PFNGLGETPROGRAMIVPROC)
      getProcAddress ("glGetProgramiv");
    GE_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)
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
    
  }else{ hasShaderObjects = false; }

  /*
  Check framebuffer objects
  *****************************************/

  if (checkExtension(ext, "GL_EXT_framebuffer_object")) {
    hasFramebufferObjects = true;
    
    GE_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)
      getProcAddress ("glGenFramebuffersEXT");
    GE_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)
      getProcAddress ("glBindFramebufferEXT");
    GE_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)
      getProcAddress ("glDeleteFramebuffersEXT");
    GE_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)
      getProcAddress ("glCheckFramebufferStatusEXT");
    GE_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)
      getProcAddress ("glFramebufferTexture2DEXT");
    GE_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)
      getProcAddress ("glFramebufferRenderbufferEXT");
    GE_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)
      getProcAddress ("glIsRenderbufferEXT");
    GE_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)
      getProcAddress ("glBindRenderbufferEXT");
    GE_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)
      getProcAddress ("glDeleteRenderbuffersEXT");
    GE_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)
      getProcAddress ("glGenRenderbuffersEXT");
    GE_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)
      getProcAddress ("glRenderbufferStorageEXT");

    if (GE_glGenFramebuffers==NULL || GE_glBindFramebuffer==NULL ||
        GE_glBindFramebuffer==NULL || GE_glDeleteFramebuffers==NULL ||
        GE_glCheckFramebufferStatus==NULL || GE_glFramebufferTexture2D==NULL ||
        GE_glFramebufferRenderbuffer==NULL || GE_glIsRenderbuffer==NULL ||
        GE_glBindRenderbuffer==NULL || GE_glDeleteRenderbuffers==NULL ||
        GE_glGenRenderbuffers==NULL || GE_glRenderbufferStorage==NULL)
      hasFramebufferObjects = false;

  }else{ hasFramebufferObjects = false; }

  /*
  Check multiple render targets
  *****************************************/

  if (checkExtension( ext, "GL_ARB_draw_buffers" )) {
    hasMultipleRenderTargets = true;
    glGetIntegerv( GL_MAX_DRAW_BUFFERS, &maxRenderTargets );

    GE_glDrawBuffers = (PFNGLDRAWBUFFERSPROC)
      getProcAddress( "glDrawBuffersARB" );

    if (GE_glDrawBuffers==NULL)
      hasMultipleRenderTargets = false;

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

    GE_glGenQueries = (PFNGLGENQUERIESPROC)
      getProcAddress( "glGenQueriesARB" );
    GE_glDeleteQueries = (PFNGLDELETEQUERIESPROC)
      getProcAddress( "glDeleteQueriesARB" );
    GE_glIsQuery = (PFNGLISQUERYPROC)
      getProcAddress( "glIsQueryARB" );
    GE_glBeginQuery = (PFNGLBEGINQUERYPROC)
      getProcAddress( "glBeginQueryARB" );
    GE_glEndQuery = (PFNGLENDQUERYPROC)
      getProcAddress( "glEndQueryARB" );
    GE_glGetQueryiv = (PFNGLGETQUERYIVPROC)
      getProcAddress( "glGetQueryivARB" );
    GE_glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)
      getProcAddress( "glGetQueryObjectivARB" );
    GE_glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)
      getProcAddress( "glGetQueryObjectuivARB" );

    if (GE_glGenQueries==NULL || GE_glDeleteQueries==NULL ||
        GE_glIsQuery==NULL || GE_glBeginQuery==NULL ||
        GE_glEndQuery==NULL || GE_glGetQueryiv==NULL ||
        GE_glGetQueryObjectiv==NULL || GE_glGetQueryObjectuiv==NULL)
      hasOcclusionQuery = false;

    maxOcclusionBits = 0;
    if (hasOcclusionQuery)
      GE_glGetQueryiv( GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &maxOcclusionBits );

  }else{ hasOcclusionQuery = false; }

  /*
  Check for vertex array object
  *****************************************/

  if (checkExtension( ext, "GL_ARB_vertex_array_object" )) {
    hasVertexArrayObjects = true;

    GE_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)
      getProcAddress( "glBindVertexArray" );
    GE_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)
      getProcAddress( "glDeleteVertexArrays" );
    GE_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)
      getProcAddress( "glGenVertexArrays" );
    GE_glIsVertexArray = (PFNGLISVERTEXARRAYPROC)
      getProcAddress( "glIsVertexArray" );

    if (GE_glBindVertexArray==NULL || GE_glIsVertexArray==NULL ||
        GE_glGenVertexArrays==NULL || GE_glDeleteVertexArrays==NULL)
      hasVertexArrayObjects = false;

  }else{ hasVertexArrayObjects = false; }

  /*
  Check 3.0 features
  *****************************************/

  //if (checkExtension( ext, "GL_EXT_glVertexAttribIPointer")) {

    GE_glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)
      getProcAddress( "glVertexAttribIPointerEXT" );
  //}

  /*
  GL_EXT_shader_image_load_store
  ******************************************/
  if (checkExtension( ext, "GL_EXT_shader_image_load_store" )) {

    GE_glBindImageTexture = (PFNGLBINDIMAGETEXTUREEXTPROC)
      getProcAddress( "glBindImageTextureEXT" );
    GE_glMemoryBarrier = (PFNGLMEMORYBARRIEREXTPROC)
      getProcAddress( "glMemoryBarrierEXT" );
  }

  /*
  GL_NV_shader_buffer_load
  *******************************************/
  if (checkExtension( ext, "GL_NV_shader_buffer_load" )) {

    GE_glMakeBufferResident = (PFNGLMAKEBUFFERRESIDENTNVPROC)
      getProcAddress( "glMakeBufferResidentNV" );
    GE_glMakeBufferNonResident = (PFNGLMAKEBUFFERNONRESIDENTNVPROC)
      getProcAddress( "glMakeBufferNonResidentNV" );
    GE_glGetBufferParameterui64v = (PFNGLGETBUFFERPARAMETERUI64VNVPROC)
      getProcAddress( "glGetBufferParameterui64vNV" );
    GE_glUniformui64 = (PFNGLUNIFORMUI64NVPROC)
      getProcAddress( "glUniformui64NV" );
  }

  if (checkExtension( ext, "GL_NV_shader_buffer_store" )) {
    bool zomg = true;
  }

  //GL_ARB_sample_shading
  if (checkExtension( ext, "GL_ARB_sample_shading" )) {
    
    GE_glMinSampleShading = (PFNGLMINSAMPLESHADINGARBPROC)
      getProcAddress( "glMinSampleShadingARB" );
  }

  /*
  Check vertical sync control
  *****************************************/
  
  #if defined(WIN32)
  GE_wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)
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
