#include <pugixml.hpp>
#include "rvgMain.h"

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
    //Check if path found
    if (strcmp( it->name(), "path" ) == 0)
    {
      //Get path description attribute
      pugi::xml_attribute ad = it->attribute( "d" );
      std::string d = ad.value();
      std::istringstream dss( d );

      //Parse path into object
      Object *obj = new Object();
      std::string cmd = "";
      while (dss.good())
      {
        //Try to parse new command or keep the old one
        std::string newCmd;
        if (dss >> newCmd)
          cmd = newCmd;

        //Parse command parameters
        if (cmd == "M" || cmd == "m")
        {
          SegSpace::Enum space = (cmd == "M")
            ? SegSpace::Absolute : SegSpace::Relative;

          Vec2 p1;
          dss >> p1.x >> p1.y;
          obj->moveTo( p1.x, p1.y, space );
        }
        else if (cmd == "L" || cmd == "l")
        {
          SegSpace::Enum space = (cmd == "L")
            ? SegSpace::Absolute : SegSpace::Relative;

          Vec2 p1;
          dss >> p1.x >> p1.y;
          obj->lineTo( p1.x, p1.y, space );
        }
        else if (cmd == "Q" || cmd == "q")
        {
          SegSpace::Enum space = (cmd == "Q")
            ? SegSpace::Absolute : SegSpace::Relative;

          Vec2 p1, p2;
          dss >> p1.x >> p1.y >> p2.x >> p2.y;
          obj->quadTo( p1.x, p1.y, p2.x, p2.y, space );
        }
        else if (cmd == "C" || cmd == "c")
        {
          SegSpace::Enum space = (cmd == "C")
            ? SegSpace::Absolute : SegSpace::Relative;

          Vec2 p1, p2, p3;
          dss >> p1.x >> p1.y >> p2.x >> p2.y >> p3.x >> p3.y;
          obj->cubicTo( p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, space );
        }
        else if (cmd == "Z" || cmd == "z")
        {
          obj->close();
        }
        else break;
      }
      
      image->addObject( obj );
    }
  }

  return image;
}
