#include "rvgMain.h"

VertexBuffer::VertexBuffer()
{
  onGpu = false;
  gpuId = 0;
}

void VertexBuffer::toGpu()
{
  if (onGpu) return;
  onGpu = true;

  glGenBuffers( 1, &gpuId );
  glBindBuffer( GL_ARRAY_BUFFER, gpuId );
  glBufferData( GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0], GL_STATIC_DRAW );
  
  checkGlError( "VertexBuffer::toGpu" );
}

void VertexBuffer::render (Shader *shader, GLenum mode)
{
  Int32 pos = shader->program->getAttribute( "in_pos" );
  Int32 tex = shader->program->getAttribute( "in_tex" );

  if (onGpu)
  {
    glBindBuffer( GL_ARRAY_BUFFER, gpuId );
    glVertexAttribPointer( pos, 3, GL_FLOAT, false, sizeof(Vertex), 0 );
    glVertexAttribPointer( tex, 2, GL_FLOAT, false, sizeof(Vertex), (void*) sizeof(Vec3) );
  }
  else
  {
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glVertexAttribPointer( pos, 3, GL_FLOAT, false, sizeof(Vertex), &verts[0] );
    glVertexAttribPointer( tex, 2, GL_FLOAT, false, sizeof(Vertex), ((Uint8*)&verts[0]) + sizeof(Vec3) );
  }

  glEnableVertexAttribArray( pos );
  glEnableVertexAttribArray( tex );
  glDrawArrays( mode, 0, verts.size() );
  glDisableVertexAttribArray( pos );
  glDisableVertexAttribArray( tex );

  checkGlError( "renderVertexBuffer" );
}
