#include "rvgMain.h"

GpuBuffer::GpuBuffer (BufferType type)
{
  this->id = 0;
  this->idValid = false;

  this->addr = 0;
  this->addrValid = false;

  this->type = type;
  this->bufSize = 0;
  this->dataSize = 0;
}

GpuBuffer::~GpuBuffer ()
{
  if (idValid)
    glDeleteBuffers( 1, &id );
}

Uint64 GpuBuffer::capacity ()
{
  return bufSize;
}

/*
Ensures the buffer has at least newBufSize memory available.
Buffer contents are invalidated. */

bool GpuBuffer::reserve (Uint64 newBufSize)
{
  //Check invalid size
  if (newBufSize == 0)
    return true;

  //Generate buffer first time
  if (!idValid)
  {
    glGenBuffers( 1, &id );
    idValid = true;
  }

  //Check if resize is required at all
  if (newBufSize > bufSize)
  {
    //Unload buffer if it was made resident
    if (addrValid)
    {
      glBindBuffer( GL_ARRAY_BUFFER, id );
      glMakeBufferNonResident( GL_ARRAY_BUFFER );
      addrValid = false;
    }

    //Setup buffer size
    switch (type) {
    case GpuBuffer::Static:
      glBindBuffer( GL_ARRAY_BUFFER, id );
      glBufferData( GL_ARRAY_BUFFER, (GLsizeiptr) newBufSize, NULL, GL_STATIC_DRAW );
      break;
    case GpuBuffer::Dynamic:
      glBindBuffer( GL_ARRAY_BUFFER, id );
      glBufferData( GL_ARRAY_BUFFER, (GLsizeiptr) newBufSize, NULL, GL_DYNAMIC_DRAW );
      break;
    }

    bufSize = newBufSize;
  }

  dataSize = 0;
  return true;
}

/*
Appends data to the end of the buffer. Returns false if not enough
memory available. */

bool GpuBuffer::append (Uint64 newDataSize, void *data)
{
  //Check invalid size
  if (newDataSize == 0)
    return true;

  //Check if memory available
  if (dataSize + newDataSize > bufSize)
    return false;

  //Upload at end of existing data
  glBindBuffer( GL_ARRAY_BUFFER, id );
  glBufferSubData( GL_ARRAY_BUFFER, (GLsizeiptr) dataSize, (GLsizeiptr) newDataSize, data );

  dataSize += newDataSize;
  return true;
}

/*
Uploads the data into the buffer, allocating memory if required.
Previous buffer contents are overwritten. Same as calling
reserve() followed by append(). */

bool GpuBuffer::upload (Uint64 newDataSize, void *data)
{
  //Check invalid size
  if (newDataSize == 0)
    return true;

  //Make sure memory is available
  if (! reserve( newDataSize ))
    return false;

  //Upload at start of buffer
  glBindBuffer( GL_ARRAY_BUFFER, id );
  glBufferSubData( GL_ARRAY_BUFFER, 0, (GLsizeiptr) newDataSize, data );

  dataSize = newDataSize;
  return true;
}

/*
Invalidates buffer contents */

void GpuBuffer::clear()
{
  dataSize = 0;
}

GLuint GpuBuffer::getId ()
{
  return id;
}

GLuint64 GpuBuffer::getAddress ()
{
  //Make buffer resident and query address
  if (!addrValid)
  {
    glBindBuffer( GL_ARRAY_BUFFER, id );
    glMakeBufferResident( GL_ARRAY_BUFFER, GL_READ_WRITE );
    glGetBufferParameterui64v( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &addr );
    addrValid = true;
  }

  return addr;
}
