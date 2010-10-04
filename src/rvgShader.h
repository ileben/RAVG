#ifndef RVGSHADER_H
#define RVGSHADER_H 1

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

#endif//RVGSHADER_H
