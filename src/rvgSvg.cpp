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
  std::string sStyle( aStyle.value() );

  //Tokenize parameters
  std::string param;
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

  //Push top node on stack
  std::stack< pugi::xml_node > nodeStack;
  nodeStack.push( svg );

  //Push default style on stack
  std::stack< SvgStyle > styleStack;
  styleStack.push( SvgStyle() );

  //Temp list for reversing
  std::vector< pugi::xml_node > tempList;

  while (!nodeStack.empty())
  {
    //Pop node from the stack
    pugi::xml_node node = nodeStack.top();
    nodeStack.pop();

    //Pop style from the stack
    SvgStyle style = styleStack.top();
    styleStack.pop();

    //Get node ID
    std::string id = node.attribute( "id" ).value();

    //Check node type by name
    std::string name = node.name();
    if (name == "g" || name == "svg")
    {
      //Parse style and combine with current one
      SvgStyle groupStyle = style + parseStyle( node );

      //Insert all the children in temp list
      for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
        tempList.push_back( *it);

      //Push all the children on the stack in reverse order
      while (!tempList.empty()) {
        nodeStack.push( tempList.back() );
        styleStack.push( groupStyle );
        tempList.pop_back();
      }
    }
    else if (name == "path")
    {
      //Parse style and combine with current one
      SvgStyle pathStyle = style + parseStyle( node );
      if (!pathStyle.hasFill) continue;

      //Parse path and apply style to object
      Object *obj = parsePath( node );
      applyStyle( pathStyle, obj );

      //Add to image
      Object *oo = obj->cubicsToQuads();
      image->addObject( oo );
      delete obj;
    }
  }
  
  return image;
}
