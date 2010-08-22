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
in vec2 quad0;
in vec2 quad1;
in vec2 quad2;

void addLine (vec2 l0, vec2 l1, ivec2 gridCoord);
void addQuad (vec2 q0, vec2 q1, vec2 q2, ivec2 gridCoord);
void quadIntersectionY (vec2 q0, vec2 q1, vec2 q2, float y, float minX, float maxX,
                        out bool found1, out bool found2, out float x1, out float x2);

void main()
{
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    bool found1=false, found2=false, inside=false;
    float yy1=0.0, yy2=0.0, xx1=0.0, xx2=0.0;

    //Find cell origin
    vec2 cmin = vec2(
      gridOrigin.x + gridCoord.x * cellSize.x,
      gridOrigin.y + gridCoord.y * cellSize.y );

    //Transform quad coords into cell space
    vec2 q0 = (quad0 - cmin) / cellSize;
    vec2 q1 = (quad1 - cmin) / cellSize;
    vec2 q2 = (quad2 - cmin) / cellSize;

    //Check if quad intersects bottom edge
    quadIntersectionY( q0, q1, q2, 1.0, 0.0, 1.0, found1, found2, xx1, xx2 );
    if (found1 != found2)
    {
      //Increase auxiliary vertical counter in the cells to the left
      for (int x = gridCoord.x - 1; x >= 0; --x)
        imageAtomicAdd( imgCellCounters, ivec3( x, gridCoord.y, 2 ), 1 );
    }
    if (found1 || found2) inside = true;

    //Skip writing into this cell if fully occluded by another object
    int cellDone = imageLoad( imgCellCounters, ivec3( gridCoord, 3 ) ).r;
    if (cellDone == 0)
    {
      //Check if quad intersects right edge
      quadIntersectionY( q0.yx, q1.yx, q2.yx, 1.0, 0.0, 1.0, found1, found2, yy1, yy2 );
      if (found1)
      {
        //Add line spanning from intersection point to upper-right corner
        addLine( vec2( 1.0, yy1 ), vec2( 1.0, -0.25 ), gridCoord );
        inside = true;
      }
      if (found2)
      {
        //Add line spanning from intersection point to upper-right corner
        addLine( vec2( 1.0, yy2 ), vec2( 1.0, -0.25 ), gridCoord );
        inside = true;
      }

      //Check for additional conditions if these two failed
      if (!inside)
      {
        //Check if start point inside
        if (q0.x >= 0.0 && q0.x <= 1.0 && q0.y >= 0.0 && q0.y <= 1.0)
          inside = true;
        else
        {
          //Check if end point inside
          if (q2.x >= 0.0 && q2.x <= 1.0 && q2.y >= 0.0 && q2.y <= 1.0)
            inside = true;
          else
          {
            //Check if quad intersects top edge
            quadIntersectionY( q0, q1, q2, 0.0, 0.0, 1.0, found1, found2, xx1, xx2 );
            if (found1 || found2) inside = true;
            else
            {
              //Check if quad intersects left edge
              quadIntersectionY( q0.yx, q1.yx, q2.yx, 0.0, 0.0, 1.0, found1, found2, yy1, yy2 );
              if (found1 || found2) inside = true;
            }
          }
        }
      }

      if (inside)
      {
        //Add quad data to stream
        addQuad( q0,q1,q2, gridCoord );
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


void addQuad (vec2 q0, vec2 q1, vec2 q2, ivec2 gridCoord)
{
  int streamIndex = imageAtomicAdd( imgCellCounters, ivec3( 0, 0, 0 ), 8 );
  int prevIndex = imageAtomicExchange( imgCellCounters, ivec3( gridCoord, 1 ), streamIndex );
  
  ptrCellStreams[ streamIndex+0 ] = 2.0;
  ptrCellStreams[ streamIndex+1 ] = q0.x;
  ptrCellStreams[ streamIndex+2 ] = q0.y;
  ptrCellStreams[ streamIndex+3 ] = q1.x;
  ptrCellStreams[ streamIndex+4 ] = q1.y;
  ptrCellStreams[ streamIndex+5 ] = q2.x;
  ptrCellStreams[ streamIndex+6 ] = q2.y;
  ptrCellStreams[ streamIndex+7 ] = (float) prevIndex;
}

void quadIntersectionY (vec2 q0, vec2 q1, vec2 q2, float y, float minX, float maxX,
                        out bool found1, out bool found2, out float x1, out float x2)
{
  found1 = false;
  found2 = false;
  x1 = 0.0;
  x2 = 0.0;

  //Quadratic equation (0 = a*t*t + b*t + c)
  float a = (q0.y - 2*q1.y + q2.y);
  float b = (-2*q0.y + 2*q1.y);
  float c = q0.y - y;

  //Discriminant
  float d = b*b - 4*a*c;

  //Find roots
  if (d > 0.0) {

    float t1,t2;

    //Find t of intersection
    if (a == 0.0)
    {
      //Equation is linear
      t1 = (y - q0.y) / (q2.y - q0.y);
      t2 = -1.0;
    }
    else
    {
      //Equation is quadratic
      float sd = sqrt( d );
      t1 = (-b - sd) / (2*a);
      t2 = (-b + sd) / (2*a);
    }
    
    //Plug into bezier equation to find x of intersection
    if (t1 >= 0.0 && t1 <= 1.0) {
      float t = t1;
      float one_t = 1.0 - t;
      x1 = one_t*one_t * q0.x + 2*t*one_t * q1.x + t*t * q2.x;
      if (x1 >= minX && x1 <= maxX) found1 = true;
    }

    if (t2 >= 0.0 && t2 <= 1.0) {
      float t = t2;
      float one_t = 1.0 - t;
      x2 = one_t*one_t * q0.x + 2*t*one_t * q1.x + t*t * q2.x;
      if (x2 >= minX && x2 <= maxX) found2 = true;
    }
  }
}
