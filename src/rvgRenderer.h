#ifndef RVGRENDERER_H
#define RVGRENDERER_H 1

class RendererClassic
{
public:
  Shader *shaderQuads;
  Shader *shaderContour;
  Shader *shader;

  RendererClassic ();
};

class RendererRandom
{
public:
  Shader *shader;

protected:
  RendererRandom () {}
};

class RendererRandomAux : public RendererRandom
{
public:
  RendererRandomAux ();
};

class RendererRandomPivot : public RendererRandom
{
public:
  RendererRandomPivot ();
};

class RendererRandomPivotLight : public RendererRandom
{
public:
  RendererRandomPivotLight ();
};

#endif//RVGRENDERER_H
