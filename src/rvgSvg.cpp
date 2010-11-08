#include <pugixml.hpp>
#include "rvgMain.h"
#include <stack>

class SvgStyle
{
public:

  bool hasFill;
  bool hasFillAlpha;

  Vec3 fill;
  Float fillAlpha;

  SvgStyle()
  {
    hasFill = false;
    hasFillAlpha = false;

    fillAlpha = 1.0f;
  }

  SvgStyle& operator+= (const SvgStyle &s)
  {
    if (s.hasFill)
    {
      fill = s.fill;
      hasFill = true;
    }
    if (s.hasFillAlpha)
    {
      fillAlpha = s.fillAlpha;
      hasFillAlpha = true;
    }
    return *this;
  }

  SvgStyle operator+ (const SvgStyle &s)
  {
    SvgStyle out = *this;
    out += s;
    return out;
  }
};

void skipUntil (std::istringstream &ss, const std::string &which)
{
  while (true)
  {
    if (!ss.good()) break;
    char c = ss.get();
    if (which.find( c ) != std::string::npos) break;
  }
}

void skipChars (std::istringstream &ss, const std::string &which)
{
  while (true)
  {
    if (!ss.good()) break;
    if (which.find( ss.peek() ) == std::string::npos) break;
    ss.get();
  }
}

std::string trim (const std::string &s, const std::string &which)
{
  int start = s.find_first_not_of( which );
  int end = s.find_last_not_of( which );

  if (start == std::string::npos || end == std::string::npos)
    return s;

  return s.substr( start, end - start + 1 );
}

Matrix4x4 parseMatrix (const std::string &value)
{

  std::istringstream ss( value );
  skipUntil( ss, "(" );

  float a,b,c,d,e,f;

  ss >> a;
  skipChars( ss, ", " );
  ss >> b;
  skipChars( ss, ", " );
  ss >> c;
  skipChars( ss, ", " );
  ss >> d;
  skipChars( ss, ", " );
  ss >> e;
  skipChars( ss, ", " );
  ss >> f;

  Matrix4x4 matrix;
  matrix.set(
    a, c, 0, e,
    b, d, 0, f,
    0, 0, 1, 0,
    0, 0, 0, 1 );
  return matrix;
}

Matrix4x4 parseTranslate (const std::string &value)
{
  std::istringstream ss( value );
  skipUntil( ss, "(" );

  float x=0.0f, y=0.0f;

  ss >> x;
  skipChars( ss, ", " );
  ss >> y;

  Matrix4x4 matrix;
  matrix.setTranslation( x, y, 0 );
  return matrix;
}

Matrix4x4 parseScale (const std::string &value)
{
  std::istringstream ss( value );
  skipUntil( ss, "(" );

  float x=0.0f, y=0.0f;

  ss >> x;
  skipChars( ss, ", " );
  if (!(ss >> y))
    y = x;

  Matrix4x4 matrix;
  matrix.setScale( x, y, 1 );
  return matrix;
}

Matrix4x4 parseRotate (const std::string &value)
{
  std::istringstream ss( value );
  skipUntil( ss, "(" );

  float a = 0.0f;
  ss >> a;

  Matrix4x4 matrix;
  matrix.setRotationZ( a * PI / 180.0f );

  float cx, cy;
  bool gotX = (ss >> cx) ? true : false;
  skipChars( ss, ", " );
  bool gotY = (ss >> cy) ? true : false;

  if (gotX && gotY)
  {
    Matrix4x4 mcenter, pcenter;
    mcenter.setTranslation( -cx, -cy, 0 );
    pcenter.setTranslation(  cx,  cy, 0 );

    return pcenter * matrix * mcenter;
  }
  else return matrix;
}

Matrix4x4 parseTransform (pugi::xml_node node)
{
  //Get transform attribute
  pugi::xml_attribute aTransform = node.attribute( "transform" );
  if (!aTransform) return Matrix4x4();

  //Parse transformation according to type
  std::string sTransform( aTransform.value() );
  
  if (sTransform.find( "matrix" ) != std::string::npos)
    return parseMatrix( sTransform );

  else if (sTransform.find( "translate" ) != std::string::npos)
    return parseTranslate( sTransform );

  else if (sTransform.find( "scale" ) != std::string::npos)
    return parseScale( sTransform );

  else if (sTransform.find( "rotate" ) != std::string::npos)
    return parseRotate( sTransform );

  return Matrix4x4();
}

Vec3 parseFill (const std::string &value)
{
  //Only accept hex color fill for now
  if (value.at(0) != '#') return Vec3(0,0,0);

  //Break values apart
  std::istringstream ssr( value.substr( 1,2 ) );
  std::istringstream ssg( value.substr( 3,2 ) );
  std::istringstream ssb( value.substr( 5,2 ) );

  //Parse hex values
  int r=0, g=0, b=0;
  ssr >> std::hex >> r;
  ssg >> std::hex >> g;
  ssb >> std::hex >> b;

  //Transform to [0,1] range
  return Vec3( (float) r/255, (float) g/255, (float) b/255 );
}

SvgStyle parseStyle (pugi::xml_node node)
{
  SvgStyle style;

  //Get style attribute
  pugi::xml_attribute aStyle = node.attribute( "style" );
  if (!aStyle) return style;

  //Tokenize parameters
  std::string param;
  std::string sStyle( aStyle.value() );
  std::istringstream ssStyle( sStyle );
  while (getline( ssStyle, param, ';'))
  {
    //Parse name and value of parameter
    std::string name, value;
    std::istringstream ssParam( param );
    getline( ssParam, name, ':' );
    getline( ssParam, value );

    //Trim whitespace
    name = trim( name, " " );
    value = trim( value, " " );

    //Process parameter
    if (name == "fill" && value != "none")
    {
      style.fill = parseFill( value );
      style.hasFill = true;
    }
  }

  return style;
}

Object* parsePath (pugi::xml_node node)
{
  Object *obj = new Object();

  //Get data attribute
  pugi::xml_attribute aData = node.attribute( "d" );
  std::string data( aData.value() );
  std::istringstream ss( data );

  //Parse path into object
  char cmd = '\0';
  while (ss.good())
  {
    //Parse new command character
    char newCmd = '\0';
    if (!(ss >> newCmd))
      break;

    //Accept new command if alphabetical, otherwise return to stream
    if (isalpha( newCmd ))
      cmd = newCmd;
    else ss.putback( newCmd );

    //Check command type
    if (cmd == 'M' || cmd == 'm')
    {
      //Find space
      SegSpace::Enum space = (cmd == 'M')
        ? SegSpace::Absolute : SegSpace::Relative;

      //Command is implicitly changed to line-to
      cmd = (cmd == 'M') ? 'L' : 'l';

      //Parse coordinates
      Vec2 p1;

      ss >> p1.x;
      skipChars( ss, ", " );
      ss >> p1.y;

      //Add to object
      obj->moveTo( p1.x, p1.y, space );
    }
    else if (cmd == 'L' || cmd == 'l')
    {
      //Find space
      SegSpace::Enum space = (cmd == 'L')
        ? SegSpace::Absolute : SegSpace::Relative;

      //Parse coordinates
      Vec2 p1;

      ss >> p1.x;
      skipChars( ss, ", " );
      ss >> p1.y;

      //Add to object
      obj->lineTo( p1.x, p1.y, space );
    }
    else if (cmd == 'Q' || cmd == 'q')
    {
      //Find space
      SegSpace::Enum space = (cmd == 'Q')
        ? SegSpace::Absolute : SegSpace::Relative;

      //Parse coordinates
      Vec2 p1, p2;
      
      ss >> p1.x;
      skipChars( ss, ", " );
      ss >> p1.y;
      
      ss >> p2.x;
      skipChars( ss, ", " );
      ss >> p2.y;

      //Add to object
      obj->quadTo( p1.x, p1.y, p2.x, p2.y, space );
    }
    else if (cmd == 'C' || cmd == 'c')
    {
      //Find space
      SegSpace::Enum space = (cmd == 'C')
        ? SegSpace::Absolute : SegSpace::Relative;

      //Parse coordinates
      Vec2 p1, p2, p3;

      ss >> p1.x;
      skipChars( ss, ", " );
      ss >> p1.y;
      
      ss >> p2.x;
      skipChars( ss, ", " );
      ss >> p2.y;
      
      ss >> p3.x;
      skipChars( ss, ", " );
      ss >> p3.y;

      //Add to object
      obj->cubicTo( p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, space );
    }
    else if (cmd == 'H' || cmd == 'h')
    {
      //Find space
      SegSpace::Enum space = (cmd == 'H')
        ? SegSpace::Absolute : SegSpace::Relative;

      //Parse coordinates
      float x1 = 0;
      ss >> x1;

      //Add to object
      obj->horizTo( x1, space );
    }
    else if (cmd == 'V' || cmd == 'v')
    {
      //Find space
      SegSpace::Enum space = (cmd == 'V')
        ? SegSpace::Absolute : SegSpace::Relative;

      //Parse coordinates
      float y1 = 0;
      ss >> y1;

      //Add to object
      obj->vertTo( y1, space );
    }
    else if (cmd == 'Z' || cmd == 'z')
    {
      //Add to object
      obj->close();
    }
    else break;
  }

  return obj;
}

void applyStyle (const SvgStyle &s, Object *obj)
{
  obj->setColor( s.fill.x, s.fill.y, s.fill.z, s.fillAlpha );
}

struct ParseNode
{
  pugi::xml_node node;
  SvgStyle style;
  Matrix4x4 transform;
  std::string id;
};

Image* loadSvg (const std::string &filename)
{
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file( filename.c_str() );

  if (!result)
  {
    std::cout << "Failed loading file '" << filename << "'" << std::endl;
    std::cout << "Error: " << result.description() << std::endl;
    return NULL;
  }

  //Get top-level SVG node
  pugi::xml_node svg = doc.child( "svg" );
  if (!svg)
  {
    std::cout << "Invalid SVG file '" << filename << "'" << std::endl;
    return NULL;
  }

  //Create new image object
  Image *image = new Image();

  //Create node for top level
  ParseNode top;
  top.node = svg;

  //Push top node on stack
  std::stack< ParseNode > stack;
  stack.push( top );

  //Temp list for reversing
  std::vector< pugi::xml_node > temp;


  while (!stack.empty())
  {
    //Pop node from the stack
    ParseNode top = stack.top();
    stack.pop();

    //Get node ID
    std::string id = top.node.attribute( "id" ).value();

    //Check node type by name
    std::string name = top.node.name();
    if (name == "g" || name == "svg")
    {
      //Parse style / transform and combine with current one
      SvgStyle groupStyle = top.style + parseStyle( top.node );
      Matrix4x4 groupTransform = top.transform * parseTransform( top.node );
      std::string groupId = top.id + id;

      //Insert all the children in temp list
      for (pugi::xml_node_iterator it = top.node.begin(); it != top.node.end(); ++it)
        temp.push_back( *it);

      //Push all the children on the stack in reverse order
      while (!temp.empty()) {
        ParseNode child;
        child.node = temp.back();
        child.style = groupStyle;
        child.transform = groupTransform;
        child.id = groupId;
        stack.push( child );
        temp.pop_back();
      }
    }
    else if (name == "path")
    {
      //Parse style / transform and combine with current one
      SvgStyle pathStyle = top.style + parseStyle( top.node );
      Matrix4x4 pathTransform = top.transform * parseTransform( top.node );
      std::string pathId = top.id + id;
      if (!pathStyle.hasFill) continue;

      //Parse path and apply style to object
      Object *obj = parsePath( top.node );
      obj->setId( pathId );
      obj->setTransform( pathTransform );
      applyStyle( pathStyle, obj );

      //Add to image
      Object *oo = obj->cubicsToQuads();
      image->addObject( oo );
      delete obj;
    }
  }
  
  return image;
}
