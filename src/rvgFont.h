#ifndef RVGFONT_H
#define RVGFONT_H 1

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_IMAGE_H
#include FT_OUTLINE_H

class Font
{
  FT_Face ftFace;

public:
  Object *object;
  Vec2 offset;

  Font (const std::string &filename);
  ~Font();

  Object* getGlyph (char code, const Vec2 &offset);
  Image* getWord (const std::string &word);
};

#endif//RVGFONT_H
