#ifndef IMAGEENCODERGPU_H
#define IMAGEENCODERGPU_H 1

class EncoderGpu
{
public:
  Shader *shaderInit;
  Shader *shaderInitObject;
  Shader *shaderLines;
  Shader *shaderQuads;
  Shader *shaderObject;
  Shader *shaderSort;
  Shader *shaderPivot;

protected:
  EncoderGpu() {}
};

class EncoderGpuAux : public EncoderGpu
{
public:
  EncoderGpuAux();
};

class EncoderGpuPivot : public EncoderGpu
{
public:
  EncoderGpuPivot();
};

#endif
