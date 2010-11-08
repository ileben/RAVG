#include "rvgMain.h"

RendererClassic::RendererClassic()
{
  shaderQuads = new Shader(
    "shaders_classic/classic_quads.vert.c",
    "shaders_classic/classic_quads.frag.c" );

  shaderContour = new Shader(
    "shaders_classic/classic_contour.vert.c",
    "shaders_classic/classic_contour.frag.c" );

  shader = new Shader(
    "shaders_classic/classic.vert.c",
    "shaders_classic/classic.frag.c" );

  shaderQuads->load();
  shaderContour->load();
  shader->load();
}

RendererRandomAux::RendererRandomAux()
{
  shader = new Shader(
    "shaders_aux/render_aux.vert.c",
    "shaders_aux/render_aux.frag.c" );

  shader->load();
}

RendererRandomPivot::RendererRandomPivot()
{
  shader = new Shader();
  shader->addSource( ShaderType::Vertex,   "shaders_pivot/render_pivot.vert.c" );
  shader->addSource( ShaderType::Fragment, "shaders_pivot/render_pivot.lookup.c" );
  shader->addSource( ShaderType::Fragment, "shaders_pivot/render_pivot.frag.c" );
  shader->load();
}
