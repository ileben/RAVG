#include "rvgMain.h"

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
    std::cout << "Failed opening file!" << std::endl;
    return std::string( "" );
  }

  long begin = f.tellg();
  f.seekg( 0, std::ios::end );
  long end = f.tellg();
  f.seekg( 0, std::ios::beg );
  long size = end - begin;

  if (size == 0) {
    std::cout << "Zero file size!"  << std::endl;
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
