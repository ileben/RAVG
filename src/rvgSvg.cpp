#include <pugixml.hpp>
#include "rvgMain.h"

void parseFill (const std::string &value, Object *obj)
{
  //Only accept hex color fill for now
  if (value.at(0) != '#') return;

  //Break values apart
  std::istringstream ssr( value.substr( 1,2 ) );
  std::istringstream ssg( value.substr( 3,2 ) );
  std::istringstream ssb( value.substr( 5,2 ) );

  //Parse hex values
  int r=0, g=0, b=0;
  ssr >> std::hex >> r;
  ssg >> std::hex >> g;
  ssb >> std::hex >> b;

  //Assign to object
  obj->setColor( (float) r/255, (float) g/255, (float) b/255 );
}

void parseStyle (pugi::xml_node node, Object *obj)
{
  //Get style attribute
  pugi::xml_attribute aStyle = node.attribute( "style" );
  std::string style( aStyle.value() );

  //Tokenize parameters
  std::string param;
  std::istringstream ssStyle( style );
  while (getline( ssStyle, param, ';'))
  {
    //Parse name and value of parameter
    std::string name, value;
    std::istringstream ssParam( param );
    getline( ssParam, name, ':' );
    getline( ssParam, value );

    //Process parameter
    if (name == "fill")
      parseFill( value, obj );
  }
}

void parsePath (pugi::xml_node node, Object *obj)
{
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
      ss.ignore( 256,',' );
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
      ss.ignore( 256, ',' );
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
      ss.ignore( 256, ',' );
      ss >> p1.y;
      
      ss >> p2.x;
      ss.ignore( 256, ',' );
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
      ss.ignore( 256, ',' );
      ss >> p1.y;
      
      ss >> p2.x;
      ss.ignore( 256, ',' );
      ss >> p2.y;
      
      ss >> p3.x;
      ss.ignore( 256, ',' );
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

  Image *image = new Image();

  //Iterate all the children of the top node
  for (pugi::xml_node_iterator it = svg.begin(); it != svg.end(); ++it)
  {
    //Check if path node found
    if (strcmp( it->name(), "path" ) == 0)
    {
      //Parse path
      Object *obj = new Object();
      parsePath( *it, obj );
      parseStyle( *it, obj );
      
      //Add to image
      Object *oo = obj->cubicsToQuads();
      image->addObject( oo );
      delete obj;
    }
  }

  return image;
}
