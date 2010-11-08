#include "rvgDefs.h"
#include "rvgShaders.h"
#include "rvgGLHeaders.h"

/*
==============================================

Vertex or fragment shader

==============================================*/

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

/*
==========================================

Shading program

==========================================*/

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

/*
==========================================

High-level Shader

==========================================*/
/*
std::vector< Shader::Def > Shader::defs;

void Shader::Define (const std::string &key, const std::string &value)
{
  Def def;
  def.key = key;
  def.value = value;
  defs.push_back( def );
}

void Shader::Define (const std::string &key, int value)
{
  std::ostringstream s;
  if (s << value)
  {
    Def def;
    def.key = key;
    def.value = s.str();
    defs.push_back( def );
  }
}

std::string Shader::ApplyDefs (const std::string &source)
{
  std::string str = source;
  
  for (Uint32 d=0; d<defs.size(); ++d)
  {
    std::size_t x = 0;
    while ((x = str.find( defs[d].key )) != std::string::npos)
      str = str.replace( x, defs[d].key.length(), defs[d].value );
  }

  return str;
}

Shader::Shader (const std::string &vertFile, const std::string &fragFile)
{
  this->vertFile = vertFile;
  this->fragFile = fragFile;

  vertex = NULL;
  geometry = NULL;
  fragment = NULL;
  program = NULL;
}

Shader::Shader (const std::string &vertFile, const std::string &geomFile, const std::string &fragFile)
{
  this->vertFile = vertFile;
  this->geomFile = geomFile;
  this->fragFile = fragFile;

  vertex = NULL;
  geometry = NULL;
  fragment = NULL;
  program = NULL;
}

Shader::~Shader ()
{
  if (vertex) delete vertex;
  if (geometry) delete geometry;
  if (fragment) delete fragment;
  if (program) delete program;
}

std::string readFile (const std::string &filename)
{
  std::ifstream f;
  f.open( filename.c_str(), std::ios::binary );
  if (!f.is_open()) {
    std::cout << "Shader: Failed opening file!" << std::endl;
    return std::string( "" );
  }

  long begin = f.tellg();
  f.seekg( 0, std::ios::end );
  long end = f.tellg();
  f.seekg( 0, std::ios::beg );
  long size = end - begin;

  if (size == 0) {
    std::cout << "Shader: Zero file size!"  << std::endl;
    return std::string( "" );
  }

  char *cstr = new char[ size+1 ];
  f.read( cstr, size );
  f.close();
  cstr[ size ] = '\0';

  std::string str( cstr );
  delete[] cstr;

  return str;
}

bool Shader::load()
{
  if (vertex) delete vertex;
  if (geometry) delete geometry;
  if (fragment) delete fragment;
  if (program) delete program;

  program = new GLProgram();
  program->create();

  //Vertex
  if (vertFile != "")
  {
    vertex = new GLShader();
    std::cout << "Loading shader " << vertFile << "..." << std::endl;
    std::string vertString = readFile( vertFile );
    vertString = Shader::ApplyDefs( vertString );

    vertex->create( ShaderType::Vertex );
    if (vertex->compile( vertString ))
    {
      //std::cout << "Vertex compiled successfuly" << std::endl;
      program->attach( vertex );
    }
    else std::cout << "Vertex compilation FAILED" << std::endl;

    std::string vertLog = vertex->getInfoLog();
    if (vertLog.length() > 0)
      std::cout << vertLog << std::endl;
  }

  //Geometry
  if (geomFile != "")
  {
    geometry = new GLShader();
    std::cout << "Loading shader " << geomFile << "..." << std::endl;
    std::string geomString = readFile( geomFile );
    geomString = Shader::ApplyDefs( geomString );

    geometry->create( ShaderType::Geometry );
    if (geometry->compile( geomString ))
    {
      //std::cout << "Geometry compiled successfuly" << std::endl;
      program->attach( geometry );
    }
    else std::cout << "Geometry compilation FAILED" << std::endl;

    std::string geomLog = geometry->getInfoLog();
    if (geomLog.length() > 0)
      std::cout << geomLog << std::endl;
  }

  //Fragment
  if (fragFile != "")
  {
    fragment = new GLShader();
    std::cout << "Loading shader " << fragFile << "..." << std::endl;
    std::string fragString = readFile( fragFile );
    fragString = Shader::ApplyDefs( fragString );

    fragment->create( ShaderType::Fragment );
    if (fragment->compile( fragString ))
    {
      //std::cout << "Fragment compiled successfuly" << std::endl;
      program->attach( fragment );
    }
    else std::cout << "Fragment compilation FAILED" << std::endl;
    
    std::string fragLog = fragment->getInfoLog();
    if (fragLog.length() > 0)
      std::cout << fragLog << std::endl;
  }

  //Link
  if (program->link())
    ; //std::cout << "Program linked successfully" << std::endl;
  else std::cout << "Program link FAILED" << std::endl;

  std::string linkLog = program->getInfoLog();
  if (linkLog.length() > 0)
    std::cout << linkLog << std::endl;

  return true;
}

void Shader::use()
{
  if (program) program->use();
}
*/

std::vector< Shader::Def > Shader::defs;

void Shader::Define (const std::string &key, const std::string &value)
{
  Def def;
  def.key = key;
  def.value = value;
  defs.push_back( def );
}

void Shader::Define (const std::string &key, int value)
{
  std::ostringstream s;
  if (s << value)
  {
    Def def;
    def.key = key;
    def.value = s.str();
    defs.push_back( def );
  }
}

std::string Shader::ApplyDefs (const std::string &source)
{
  std::string str = source;
  
  for (Uint32 d=0; d<defs.size(); ++d)
  {
    std::size_t x = 0;
    while ((x = str.find( defs[d].key )) != std::string::npos)
      str = str.replace( x, defs[d].key.length(), defs[d].value );
  }

  return str;
}

Shader::Shader()
{
  program = NULL;
}

Shader::Shader (const std::string &vertFile, const std::string &fragFile)
{
  addSource( ShaderType::Vertex, vertFile );
  addSource( ShaderType::Fragment, fragFile );
  program = NULL;
}

Shader::Shader (const std::string &vertFile, const std::string &geomFile, const std::string &fragFile)
{
  addSource( ShaderType::Vertex, vertFile );
  addSource( ShaderType::Geometry, geomFile );
  addSource( ShaderType::Fragment, fragFile );
  program = NULL;
}

void Shader::addSource (ShaderType::Enum type, const std::string &file)
{
  Source s;
  s.type = type;
  s.file = file;
  sources.push_back( s );
}

Shader::~Shader ()
{
  freeShaders();
}

void Shader::freeShaders()
{
  for (Uint s=0; s<shaders.size(); ++s)
    delete shaders[s];

  if (program)
    delete program;
}

std::string readFile (const std::string &filename)
{
  std::ifstream f;
  f.open( filename.c_str(), std::ios::binary );
  
  if (!f.is_open()) {
    throw std::string( "Shader '" ) + filename + "': Failed opening file!";
  }

  long begin = f.tellg();
  f.seekg( 0, std::ios::end );
  long end = f.tellg();
  f.seekg( 0, std::ios::beg );
  long size = end - begin;

  if (size == 0) {
    throw std::string( "Shader '" ) + filename + "': Zero file size!";
  }

  char *cstr = new char[ size+1 ];
  f.read( cstr, size );
  f.close();
  cstr[ size ] = '\0';

  std::string str( cstr );
  delete[] cstr;

  return str;
}

bool Shader::load()
{
  //Cleanup
  freeShaders();

  //Program
  program = new GLProgram();
  program->create();

  //Shaders
  for (Uint s=0; s<sources.size(); ++s)
  {
    //Load source
    std::cout << "Loading shader " << sources[s].file << "..." << std::endl;
    std::string source;
    try
    {
      source = readFile( sources[s].file );
      source = Shader::ApplyDefs( source );
    }
    catch (std::string e)
    {
      std::cout << e << std::endl;
      continue;
    }

    //Compile shader
    GLShader *shader = new GLShader();
    shader->create( sources[s].type );
    shaders.push_back( shader );
    if (shader->compile( source ))
    {
      //std::cout << "Shader compiled successfuly" << std::endl;
      program->attach( shader );
    }
    else std::cout << "Shader compilation FAILED" << std::endl;

    //Report log
    std::string shaderLog = shader->getInfoLog();
    if (shaderLog.length() > 0)
      std::cout << shaderLog << std::endl;
  }

  //Link program
  if (program->link())
    ; //std::cout << "Program linked successfully" << std::endl;
  else std::cout << "Program link FAILED" << std::endl;

  //Report log
  std::string linkLog = program->getInfoLog();
  if (linkLog.length() > 0)
    std::cout << linkLog << std::endl;

  return true;
}

void Shader::use()
{
  if (program) program->use();
}
