#ifndef __GESHADERS_H
#define __GESHADERS_H

/*================================
 * Forward declarations
 *================================*/

class GLProgram;

/*================================
 * Vertex or fragment shader
 *================================*/

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

/*==============================
 * Shading program
 *==============================*/

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

#endif /* __GESHADERS_H */
