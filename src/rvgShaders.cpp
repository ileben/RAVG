#include "rvgDefs.h"
#include "rvgShaders.h"
#include "rvgGLHeaders.h"

/*==============================================
 *
 * Vertex or fragment shader
 *
 *==============================================*/

GLShader::GLShader ()
{
  type = ShaderType::Any;
  handle = 0;
}

GLShader::~GLShader ()
{
  glDeleteShader( handle );
}

void GLShader::create (ShaderType::Enum type)
{
  if (handle != 0) return;
  
  switch (type)
  {
  case ShaderType::Vertex:
    handle = glCreateShader( GL_VERTEX_SHADER );
    break;
  case ShaderType::Geometry:
    handle = glCreateShader( GL_GEOMETRY_SHADER );
    break;
  case ShaderType::Fragment:
    handle = glCreateShader( GL_FRAGMENT_SHADER );
    break;
  }
  
  this->type = type;
}

bool GLShader::compile (const std::string &source)
{
  const GLchar *src = (const GLchar*) source.c_str();
  glShaderSource (handle, 1, &src, NULL);

  GLint status = 0;
  glCompileShader( handle );
  glGetShaderiv( handle, GL_COMPILE_STATUS, &status );
  return (status == GL_TRUE);
}

std::string GLShader::getInfoLog ()
{
  GLint len = 0;
  glGetShaderiv( handle, GL_INFO_LOG_LENGTH, &len);
  if (len == 0) return std::string("");

  char *clog = new char[len+1];
  glGetShaderInfoLog( handle, len+1, NULL, clog );
  clog[len] = '\0';
  
  std::string log( clog );
  delete[] clog;
  return log;
}

/*==========================================
 *
 * Shading program
 *
 *==========================================*/

GLProgram::GLProgram ()
{
}

GLProgram::~GLProgram ()
{
  glDeleteProgram( handle );
}

void GLProgram::create ()
{
  handle = glCreateProgram ();
}

void GLProgram::attach (GLShader *s)
{
  glAttachShader( handle, s->handle );
}

void GLProgram::detach (GLShader *s)
{
  glDetachShader( handle, s->handle );
}

bool GLProgram::link ()
{
  GLint status;
  glLinkProgram (handle);
  glGetProgramiv( handle, GL_LINK_STATUS, &status );
  return (status == GL_TRUE);
}

std::string GLProgram::getInfoLog ()
{
  GLint len = 0;
  glGetProgramiv( handle, GL_INFO_LOG_LENGTH, &len);
  if (len == 0) return std::string( "" );

  char *clog = new char[len+1];
  glGetProgramInfoLog( handle, len+1, NULL, clog );
  clog[len] = '\0';

  std::string log( clog );
  delete[] clog;
  return log;
}

void GLProgram::use ()
{
  glUseProgram( handle );
}

void GLProgram::UseFixed ()
{
  glUseProgram( 0 );
}

void GLProgram::bindAttribute (Uint32 index, const char *name)
{
  glBindAttribLocation( handle, index, name );
}

Int32 GLProgram::getAttribute (const char *name) const
{
  return GE_glGetAttribLocation( handle, name );
}

Int32 GLProgram::getUniform (const char *name) const
{
  return glGetUniformLocation( handle, name );
}
