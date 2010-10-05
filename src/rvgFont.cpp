#include "rvgMain.h"

static bool ftInit = false;
static FT_Library ftLib;

void initFreetype()
{
  FT_Error ftErr = FT_Init_FreeType( &ftLib );
  if (ftErr)
  {
    std::cout << "Error while initializing FreeType library!" << std::endl;
    return;
  }

  ftInit = true;
}

Font::Font (const std::string &filename)
{
  if (!ftInit) initFreetype();

  FT_Error ftErr;

  //Create font face from file
  ftErr = FT_New_Face( ftLib, filename.c_str(), 0, &ftFace );
  if (ftErr)
  {
    std::cout << "Error while loading font file '" << filename << "'" << std::endl;
    return;
  }

  //Set glyph size
  FT_F26Dot6 size = 100 << 6;
  ftErr = FT_Set_Char_Size( ftFace, size, size, 72, 72 );
  if (ftErr)
  {
    std::cout << "Error while setting char size!" << std::endl;
    return;
  }
}

Font::~Font ()
{
  FT_Done_Face( ftFace );
}

float ft266ToFloat (FT_F26Dot6 f)
{
  return float(f >> 6) + float(f & 0x3F) / 64;
}

Vec2 ftVecToFloat (const FT_Vector *v)
{
  return Vec2( ft266ToFloat( v->x ), ft266ToFloat( v->y ) );
}

int ftMoveTo (const FT_Vector *v1, void *user)
{
  Font *font = (Font*) user;
  Vec2 p1 = ftVecToFloat( v1 ) + font->offset;

  font->object->moveTo( p1.x, p1.y );
  return 0;
}

int ftLineTo (const FT_Vector *v1, void *user)
{
  Font *font = (Font*) user;
  Vec2 p1 = ftVecToFloat( v1 ) + font->offset;

  font->object->lineTo( p1.x, p1.y );
  return 0;
}

int ftQuadTo (const FT_Vector *v1, const FT_Vector *v2, void *user)
{
  Font *font = (Font*) user;
  Vec2 p1 = ftVecToFloat( v1 ) + font->offset;
  Vec2 p2 = ftVecToFloat( v2 ) + font->offset;

  font->object->quadTo( p1.x, p1.y, p2.x, p2.y );
  return 0;
}

int ftCubicTo (const FT_Vector *v1, const FT_Vector *v2, const FT_Vector *v3, void *user)
{
  Font *font = (Font*) user;
  Vec2 p1 = ftVecToFloat( v1 ) + font->offset;
  Vec2 p2 = ftVecToFloat( v2 ) + font->offset;
  Vec2 p3 = ftVecToFloat( v3 ) + font->offset;

  font->object->cubicTo( p1.x, p1.y, p2.x, p2.y, p3.x, p3.y );
  return 0;
}

Object* Font::getGlyph (char code, const Vec2 &offset)
{
  FT_Error ftErr;
  FT_Glyph ftGlyph;
  FT_OutlineGlyph ftOutlineGlyph;

  //Load glyph data into font face
  FT_UInt ftGlyphIndex = FT_Get_Char_Index( ftFace, (FT_ULong)code );
  ftErr = FT_Load_Glyph( ftFace, ftGlyphIndex, FT_LOAD_DEFAULT );
  if (ftErr)
  {
    std::cout << "Error while loading glyph!" << std::endl;
    return NULL;
  }

  //Get glyph from glyph slot of font face
  ftErr = FT_Get_Glyph( ftFace->glyph, &ftGlyph );
  if (ftErr)
  {
    std::cout << "Error while getting glyph from slot!" << std::endl;
    return NULL;
  }

  //Cast glyph to outline glyph
  ftOutlineGlyph = (FT_OutlineGlyph) ftGlyph;
  if (ftGlyph->format != FT_GLYPH_FORMAT_OUTLINE)
  {
    std::cout << "Error while casting glyph to outline glyph!" << std::endl;
    return NULL;
  }

  //Construct outline
  FT_Outline_Funcs ftFuncs;
  ftFuncs.move_to = ftMoveTo;
  ftFuncs.line_to = ftLineTo;
  ftFuncs.conic_to = ftQuadTo;
  ftFuncs.cubic_to = ftCubicTo;
  ftFuncs.shift = 0;
  ftFuncs.delta = 0;
  
  Object *object = new Object;
  this->object = object;
  this->offset = offset;
  ftErr = FT_Outline_Decompose( &ftOutlineGlyph->outline, &ftFuncs, this );

  //Cleanup
  std::cout << "All good" << std::endl;
  FT_Done_Glyph( ftGlyph );

  return object;
}

Image* Font::getWord (const std::string &word)
{
  Image *img = new Image;

  Vec2 off( 0, 0 );
  const char *cword = word.c_str();
  for (int c=0; cword[c] != '\0'; ++c)
  {
    Object *o = getGlyph( cword[c], off );
    img->objects.push_back( o->cubicsToQuads() );
    delete o;

    off.x += ft266ToFloat( ftFace->glyph->metrics.horiAdvance );
  }

  return img;
}
