#ifndef __GESHADERS_H
#define __GESHADERS_H

/*
================================
Forward declarations
================================*/

class GLProgram;

/*
================================
Shader
================================*/

namespace ShaderType {
  enum Enum
  {
    Vertex     =  0,
    Geometry   =  1,
    Fragment   =  2,
    Any        =  3
  };};

class GLShader
{
  friend class GLProgram;
  
protected:
  Uint32 handle;
  ShaderType::Enum type;
  
public:
  GLShader ();
  virtual ~GLShader ();
  void create (ShaderType::Enum type);
  bool compile (const std::string &source);
  std::string getInfoLog ();
};

/*
==============================
Shading program
==============================*/

class GLProgram
{
protected:
  Uint32 handle;
 
public:
  GLProgram ();
  virtual ~GLProgram ();
  
  void create ();
  void attach (GLShader *s);
  void detach (GLShader *s);

  bool link ();
  std::string getInfoLog ();
  void use ();
  static void UseFixed ();
  void bindAttribute (Uint32 index, const char *name);
  Int32 getUniform (const char *name) const;
  Int32 getAttribute (const char *name) const;
};

/*
==============================
High-level Shader
==============================*/
/*
class Shader
{
  //Static /////////////
private:

  struct Def  { std::string key; std::string value; };
  static std::vector< Def > defs;
  static std::string ApplyDefs (const std::string &source);

public:

  static void Define( const std::string &key, const std::string &value );
  static void Define( const std::string &key, int value );

  //Non-static /////////
private:

  std::string vertFile;
  std::string geomFile;
  std::string fragFile;

public:

  GLProgram *program;
  GLShader *vertex;
  GLShader *geometry;
  GLShader *fragment;

public:

  Shader (const std::string &vertFile, const std::string &fragFile);
  Shader (const std::string &vertFile, const std::string &geomFile, const std::string &fragFile);
  ~Shader ();
  bool load ();
  void use();
};
*/

class Shader
{
  struct Source
  {
    ShaderType::Enum type;
    std::string file;
  };

  //Static /////////////
private:

  struct Def  { std::string key; std::string value; };
  static std::vector< Def > defs;
  static std::string ApplyDefs (const std::string &source);

public:

  static void Define( const std::string &key, const std::string &value );
  static void Define( const std::string &key, int value );

  //Non-static /////////
private:

  std::vector< Source > sources;
  std::vector< GLShader* > shaders;

  void freeShaders();

public:

  GLProgram *program;

public:

  Shader ();
  Shader (const std::string &vertFile, const std::string &fragFile);
  Shader (const std::string &vertFile, const std::string &geomFile, const std::string &fragFile);
  ~Shader ();

  void addSource (ShaderType::Enum type, const std::string &file);
  bool load ();
  void use();
};

#endif//__GESHADERS_H
