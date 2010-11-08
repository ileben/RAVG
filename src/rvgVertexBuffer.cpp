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
  Int32 aPos = shader->program->getAttribute( "in_pos" );
  Int32 aTex = shader->program->getAttribute( "in_tex" );
  Int32 aNormal = shader->program->getAttribute( "in_normal" );

  if (onGpu)
  {
    glBindBuffer( GL_ARRAY_BUFFER, gpuId );
    glVertexAttribPointer( aPos, 3, GL_FLOAT, false, sizeof(Vertex), 0 );
    glVertexAttribPointer( aTex, 2, GL_FLOAT, false, sizeof(Vertex), (void*) sizeof(Vec3) );
    if (aNormal != -1)
      glVertexAttribPointer( aNormal, 3, GL_FLOAT, false, sizeof(Vertex), (void*) (sizeof(Vec3) + sizeof(Vec2)) );
  }
  else
  {
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glVertexAttribPointer( aPos, 3, GL_FLOAT, false, sizeof(Vertex), &verts[0] );
    glVertexAttribPointer( aTex, 2, GL_FLOAT, false, sizeof(Vertex), ((Uint8*)&verts[0]) + sizeof(Vec3) );
    if (aNormal != -1)
      glVertexAttribPointer( aNormal, 3, GL_FLOAT, false, sizeof(Vertex), ((Uint8*)&verts[0]) + sizeof(Vec3) + sizeof(Vec2) );
  }

  glEnableVertexAttribArray( aPos );
  glEnableVertexAttribArray( aTex );
  if (aNormal != -1) glEnableVertexAttribArray( aNormal );

  glDrawArrays( mode, 0, verts.size() );
  
  glDisableVertexAttribArray( aPos );
  glDisableVertexAttribArray( aTex );
  if (aNormal != -1) glEnableVertexAttribArray( aNormal );

  checkGlError( "renderVertexBuffer" );
}
