#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform coherent float* ptrCellStreams;
uniform coherent layout( size1x32 ) image2DArray imgCellStreams;
uniform coherent layout( size1x32 ) iimage2DArray imgCellCounters;
uniform vec2 gridOrigin;
uniform ivec2 gridSize;
uniform vec2 cellSize;

in layout( pixel_center_integer ) vec4 gl_FragCoord;
in vec2 line0;
in vec2 line1;

void addLine (vec2 l0, vec2 l1, ivec2 gridCoord);
void lineIntersectionY (vec2 l0, vec2 l1, float y, float minX, float maxX,
                        out bool found, out float x);

void main()
{
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    bool found = false, inside = false;
    float yy = 0.0, xx = 0.0;

    //Find cell origin
    vec2 cmin = vec2(
      gridOrigin.x + gridCoord.x * cellSize.x,
      gridOrigin.y + gridCoord.y * cellSize.y );

    //Transform line coords into cell space
    vec2 l0 = (line0 - cmin) / cellSize;
    vec2 l1 = (line1 - cmin) / cellSize;

    //Check if line intersects bottom edge
    lineIntersectionY( l0, l1, 1.0, 0.0, 1.0, found, xx );
    if (found)
    {
      //Increase auxiliary vertical counter in the cells to the left
      for (int x = gridCoord.x - 1; x >= 0; --x)
        imageAtomicAdd( imgCellCounters, ivec3( x, gridCoord.y, 2 ), 1 );
      inside = true;
    }

    //Skip writing into this cell if fully occluded by another object
    int cellDone = imageLoad( imgCellCounters, ivec3( gridCoord, 3 ) ).r;
    if (cellDone == 0)
    {
      //Check if line intersects right edge
      lineIntersectionY( l0.yx, l1.yx, 1.0, 0.0, 1.0, found, yy );
      if (found)
      {
        //Add line spanning from intersection point to upper-right corner
        addLine( vec2( 1.0, yy ), vec2( 1.0, -0.25 ), gridCoord );
        inside = true;
      }
      
      //Check for additional conditions if these two failed
      if (!inside)
      {
        //Check if start point inside
        if (l0.x >= 0.0 && l0.x <= 1.0 && l0.y >= 0.0 && l0.y <= 1.0)
          inside = true;
        else
        {
          //Check if end point inside
          if (l1.x >= 0.0 && l1.x <= 1.0 && l1.y >= 0.0 && l1.y <= 1.0)
            inside = true;
          else
          {
            //Check if line intersects top edge
            lineIntersectionY( l0, l1, 0.0, 0.0, 1.0, found, xx );
            if (found) inside = true;
            else
            {
              //Check if line intersects left edge
              lineIntersectionY( l0.yx, l1.yx, 0.0, 0.0, 1.0, found, yy );
              if (found) inside = true;
            }
          }
        }
      }

      if (inside)
      {
        //Add line data to stream
        addLine( l0,l1, gridCoord );
      }
    }
  }

  discard;
}

void addLine (vec2 l0, vec2 l1, ivec2 gridCoord)
{
  int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( 0, 0, 0 ), 6 );
  int prevIndex = imageAtomicExchange( imgCellCounters, ivec3( gridCoord, 1 ), streamIndex );
  
  ptrCellStreams[ streamIndex+0 ] = 1.0;
  ptrCellStreams[ streamIndex+1 ] = l0.x;
  ptrCellStreams[ streamIndex+2 ] = l0.y;
  ptrCellStreams[ streamIndex+3 ] = l1.x;
  ptrCellStreams[ streamIndex+4 ] = l1.y;
  ptrCellStreams[ streamIndex+5 ] = (float) prevIndex;
}

void lineIntersectionY (vec2 l0, vec2 l1, float y, float minX, float maxX,
                        out bool found, out float x)
{
  found = false;
  x = 0.0;

  //Linear equation (0 = a*t + b)
  float a = l1.y - l0.y;
  float b = l0.y - y;

  //Check if equation constant
  if (a != 0.0)
  {
    //Find t of intersection
    float t = -b / a;

    //Plug into linear equation to find x of intersection
    if (t >= 0.0 && t <= 1.0) {
      x = l0.x + t * (l1.x - l0.x);
      if (x >= minX && x <= maxX) found = true;
    }
  }
}
