#ifndef RVGGPUBUFFER_H
#define RVGGPUBUFFER_H 1

class GpuBuffer
{
public:
  enum BufferType
  {
    Static,
    Dynamic
  };

private:
  BufferType type;

  GLuint id;
  bool idValid;

  GLuint64 addr;
  bool addrValid;

  Uint64 bufSize;
  Uint64 dataSize;

public:
  GpuBuffer (BufferType type);
  ~GpuBuffer ();

  Uint64 capacity();
  bool reserve (Uint64 newBufSize);
  bool append (Uint64 newDataSize, void *data);
  bool upload (Uint64 newDataSize, void *data);
  void clear();

  GLuint getId ();
  GLuint64 getAddress ();
};

class StaticGpuBuffer : public GpuBuffer {
public: StaticGpuBuffer() : GpuBuffer( GpuBuffer::Static ) {}
};

class DynamicGpuBuffer : public GpuBuffer {
public: DynamicGpuBuffer() : GpuBuffer( GpuBuffer::Dynamic ) {}
};

#endif//RVGGPUBUFFER_H
