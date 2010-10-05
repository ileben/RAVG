#ifndef IMAGEENCODERGPU_H
#define IMAGEENCODERGPU_H 1

class ImageEncoderGpu
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
  ImageEncoderGpu() {}
};

class ImageEncoderGpuAux : public ImageEncoderGpu
{
public:
  ImageEncoderGpuAux();
};

class ImageEncoderGpuPivot : public ImageEncoderGpu
{
public:
  ImageEncoderGpuPivot();
};

#endif
