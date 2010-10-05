#include "rvgMain.h"

ImageEncoderGpuAux::ImageEncoderGpuAux()
{
  shaderInit = new Shader(
    "shaders_aux/encode_aux_init.vert.c",
    "shaders_aux/encode_aux_init.frag.c" );

  shaderInitObject = new Shader(
    "shaders_aux/encode_aux_initobject.vert.c",
    "shaders_aux/encode_aux_initobject.geom.c",
    "shaders_aux/encode_aux_initobject.frag.c" );

  shaderLines = new Shader(
    "shaders_aux/encode_aux_lines.vert.c",
    "shaders_aux/encode_aux_lines.geom.c",
    "shaders_aux/encode_aux_lines.frag.c" );

  shaderQuads = new Shader(
    "shaders_aux/encode_aux_quads.vert.c",
    "shaders_aux/encode_aux_quads.geom.c",
    "shaders_aux/encode_aux_quads.frag.c" );

  shaderObject = new Shader(
    "shaders_aux/encode_aux_object.vert.c",
    "shaders_aux/encode_aux_object.frag.c" );

  shaderSort = new Shader(
    "shaders_aux/encode_aux_sort.vert.c",
    "shaders_aux/encode_aux_sort.frag.c" );

  shaderInit->load();
  shaderInitObject->load();
  shaderLines->load();
  shaderQuads->load();
  shaderObject->load();
  shaderSort->load();
}

ImageEncoderGpuPivot::ImageEncoderGpuPivot()
{
  shaderInit = new Shader(
    "shaders_pivot/encode_pivot_init.vert.c",
    "shaders_pivot/encode_pivot_init.frag.c" );

  shaderInitObject = new Shader(
    "shaders_pivot/encode_pivot_initobject.vert.c",
    "shaders_pivot/encode_pivot_initobject.geom.c",
    "shaders_pivot/encode_pivot_initobject.frag.c" );

  shaderLines = new Shader(
    "shaders_pivot/encode_pivot_lines.vert.c",
    "shaders_pivot/encode_pivot_lines.geom.c",
    "shaders_pivot/encode_pivot_lines.frag.c" );

  shaderQuads = new Shader(
    "shaders_pivot/encode_pivot_quads.vert.c",
    "shaders_pivot/encode_pivot_quads.geom.c",
    "shaders_pivot/encode_pivot_quads.frag.c" );

  shaderObject = new Shader(
    "shaders_pivot/encode_pivot_object.vert.c",
    "shaders_pivot/encode_pivot_object.frag.c" );

  shaderSort = new Shader(
    "shaders_pivot/encode_pivot_sort.vert.c",
    "shaders_pivot/encode_pivot_sort.frag.c" );

  shaderInit->load();
  shaderInitObject->load();
  shaderLines->load();
  shaderQuads->load();
  shaderObject->load();
  shaderSort->load();
}
