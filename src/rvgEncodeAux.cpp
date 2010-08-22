#if (0)
#include "rvgMain.h"

ivec4 Image::imageLoad (int* counters, const IVec3 &coord)
{
  int counterIndex = coord.z * gridSize.x * gridSize.y + coord.y * gridSize.x + coord.x;
  return ivec4( counters[ counterIndex ] );
}

void Image::imageStore (int* counters, const IVec3 &coord, const ivec4 &value)
{
  int counterIndex = coord.z * gridSize.x * gridSize.y + coord.y * gridSize.x + coord.x;
  counters[ counterIndex ] = value.x;
}

int Image::imageAtomicAdd (int* counters, const IVec3 &coord, int value)
{
  int counterIndex = coord.z * gridSize.x * gridSize.y + coord.y * gridSize.x + coord.x;
  int oldValue = counters[ counterIndex ];
  counters[ counterIndex ] += value;
  return oldValue;
}

int Image::imageAtomicExchange (int* counters, const IVec3 &coord, int value)
{
  int counterIndex = coord.z * gridSize.x * gridSize.y + coord.y * gridSize.x + coord.x;
  int oldValue = counters[ counterIndex ];
  counters[ counterIndex ] = value;
  return oldValue;
}

void Image::addLine (const Vec2 &l0, const Vec2 &l1, const IVec2 &gridCoord)
{
  int streamIndex = imageAtomicAdd( cpuCounters, ivec3( 0,0, COUNTER_MASTER ), 6 );
  int prevIndex = imageAtomicExchange( cpuCounters, ivec3( gridCoord, COUNTER_PREV ), streamIndex );

  cpuStream[ streamIndex+0 ] = (float) StreamSegType::Line;
  cpuStream[ streamIndex+1 ] = l0.x;
  cpuStream[ streamIndex+2 ] = l0.y;
  cpuStream[ streamIndex+3 ] = l1.x;
  cpuStream[ streamIndex+4 ] = l1.y;
  cpuStream[ streamIndex+5 ] = (float) prevIndex;
}

void Image::addQuad (const Vec2 &q0, const Vec2 &q1, const Vec2 &q2, const IVec2 &gridCoord)
{
  int streamIndex = imageAtomicAdd( cpuCounters, ivec3( 0,0, COUNTER_MASTER ), 8 );
  int prevIndex = imageAtomicExchange( cpuCounters, ivec3( gridCoord, COUNTER_PREV ), streamIndex );

  cpuStream[ streamIndex+0 ] = (float) StreamSegType::Quad;
  cpuStream[ streamIndex+1 ] = q0.x;
  cpuStream[ streamIndex+2 ] = q0.y;
  cpuStream[ streamIndex+3 ] = q1.x;
  cpuStream[ streamIndex+4 ] = q1.y;
  cpuStream[ streamIndex+5 ] = q2.x;
  cpuStream[ streamIndex+6 ] = q2.y;
  cpuStream[ streamIndex+7 ] = (float) prevIndex;
}

void Image::addObject (const Vec4 &color, const IVec2 &gridCoord)
{
  int streamIndex = imageAtomicAdd( cpuCounters, ivec3( 0,0, COUNTER_MASTER ), 6 );
  int prevIndex = imageAtomicExchange( cpuCounters, ivec3( gridCoord, COUNTER_PREV ), streamIndex );

  cpuStream[ streamIndex+0 ] = (float) StreamSegType::Object;
  cpuStream[ streamIndex+1 ] = color.x;
  cpuStream[ streamIndex+2 ] = color.y;
  cpuStream[ streamIndex+3 ] = color.z;
  cpuStream[ streamIndex+4 ] = color.w;
  cpuStream[ streamIndex+5 ] = (float) prevIndex;
}

void Image::encodeLine (const vec2 &line0,
                        const vec2 &line1,
                        const ivec2 &gridCoord,
                        float *stream,
                        int *counters)
{
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    bool found = false, inside = false;
    float yy = 0.0f, xx = 0.0f;

    //Find cell origin
    Vec2 cmin = Vec2(
      gridOrigin.x + gridCoord.x * cellSize.x,
      gridOrigin.y + gridCoord.y * cellSize.y );

    //Transform line coords into cell space
    Vec2 l0 = (line0 - cmin) / cellSize;
    Vec2 l1 = (line1 - cmin) / cellSize;

    //Check if line intersects bottom edge
    lineIntersectionY( l0, l1, 1.0f, 0.0f, 1.0f, found, xx );
    if (found)
    {
      //Increase auxiliary vertical counter in the cells to the left
      for (int x = (int)gridCoord.x - 1; x >= 0; --x)
        imageAtomicAdd( counters, ivec3( x, gridCoord.y, COUNTER_AUX), 1 );
      inside = true;
    }

    //Skip writing into this cell if fully occluded by another object
    int cellDone = imageLoad( counters, ivec3( gridCoord, COUNTER_OCCLUSION ) ).x;
    if (cellDone == 0)
    {
      //Check if line intersects right edge
      lineIntersectionY( l0.yx(), l1.yx(), 1.0f, 0.0f, 1.0f, found, yy );
      if (found)
      {
        //Add line spanning from intersection point to upper-right corner
        addLine( Vec2( 1.0f, yy ), Vec2( 1.0f, -0.25f ), gridCoord );
        inside = true;
      }
      
      //Check for additional conditions if these two failed
      if (!inside)
      {
        //Check if start point inside
        if (l0.x >= 0.0f && l0.x <= 1.0f && l0.y >= 0.0f && l0.y <= 1.0f)
          inside = true;
        else
        {
          //Check if end point inside
          if (l1.x >= 0.0f && l1.x <= 1.0f && l1.y >= 0.0f && l1.y <= 1.0f)
            inside = true;
          else
          {
            //Check if line intersects top edge
            lineIntersectionY( l0, l1, 0.0f, 0.0f, 1.0f, found, xx );
            if (found) inside = true;
            else
            {
              //Check if line intersects left edge
              lineIntersectionY( l0.yx(), l1.yx(), 0.0f, 0.0f, 1.0f, found, yy );
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
}

void Image::encodeQuad (const vec2 &quad0,
                        const vec2 &quad1,
                        const vec2 &quad2,
                        const ivec2 &gridCoord,
                        float *stream,
                        int *counters)
{
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
        imageAtomicAdd( counters, ivec3( x, gridCoord.y, COUNTER_AUX ), 1 );
    }
    if (found1 || found2) inside = true;

    //Skip writing into this cell if fully occluded by another object
    int cellDone = imageLoad( counters, ivec3( gridCoord, COUNTER_OCCLUSION ) ).x;
    if (cellDone == 0)
    {
      //Check if quad intersects right edge
      quadIntersectionY( q0.yx(), q1.yx(), q2.yx(), 1.0, 0.0, 1.0, found1, found2, yy1, yy2 );
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
              quadIntersectionY( q0.yx(), q1.yx(), q2.yx(), 0.0, 0.0, 1.0, found1, found2, yy1, yy2 );
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
}

void Image::encodeObject (const vec4 &color,
                          const ivec2 &gridCoord,
                          float *stream,
                          int *counters)
{
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    //Get and reset auxiliary vertical counter
    int aux = imageAtomicExchange( counters, ivec3( gridCoord, COUNTER_AUX ), 0 );

    //Skip writing into this cell if fully occluded by another object
    int cellDone = imageLoad( counters, ivec3( gridCoord, COUNTER_OCCLUSION ) ).x;
    if (cellDone == 0)
    {
      bool anySegments = false;

      //Get index of the previous node
      int prevIndex = imageLoad( counters, ivec3( gridCoord, COUNTER_PREV ) ).x;
      if (prevIndex >= 0)
      {
        //Check if previous node other than an object start
        int prevType = (int) stream[ prevIndex ];
        if (prevType != 3) anySegments = true;
      }

      //Check if auxiliary counter parity is odd
      if (aux % 2 == 1)
      {
        //Add auxiliary vertical segment
        addLine( vec2( 1.0, 1.25f ), vec2( 1.0, -0.25f ), gridCoord );

        //If no other segments, mark the cell fully occluded
        if (!anySegments) imageStore( counters, ivec3( gridCoord, 3 ), ivec4( 1 ) );
        anySegments = true;
      }
      
      //Add object data if any of its segments in this cell
      if (anySegments) addObject( color, gridCoord );
    }
  }
}

void Image::updateStream ()
{
  //Reset buffers and stream
  if (cpuCounters) delete[] cpuCounters;
  cpuCounters = new int[ gridSize.x * gridSize.y * COUNTER_LEN ];

  if (cpuStream) delete[] cpuStream;
  cpuStream = new Float[ MAX_COMBINED_STREAM_LEN ];

  int maxCellLen = 0;

  //Init auxiliary vertical counters
  for (int x=0; x < gridSize.x; ++x) {
    for (int y=0; y < gridSize.y; ++y) {

      imageStore( cpuCounters, ivec3( x,y, COUNTER_MASTER ),    ivec4( 0 ));
      imageStore( cpuCounters, ivec3( x,y, COUNTER_PREV ),      ivec4( -1 ));
      imageStore( cpuCounters, ivec3( x,y, COUNTER_AUX ),       ivec4( 0 ));
      imageStore( cpuCounters, ivec3( x,y, COUNTER_OCCLUSION ), ivec4( 0 ));
    }
  }

  //Walk the list of objects
  for (int o=(int)objects.size()-1; o>=0; --o)
  {
    Object *obj = objects[o];

    //Walk the list of line segments
    for (Uint32 l=0; l<obj->lines.size(); ++l) {

      //Find line bounds
      Line &line = obj->lines[l];
      Vec2 lmin = Vec::Min( line.p0, line.p1 );
      Vec2 lmax = Vec::Max( line.p0, line.p1 );

      //Transform line bounds into grid space
      lmin = Vec::Floor( (lmin - min) / cellSize );
      lmax = Vec::Ceil( (lmax - min) / cellSize );

      //Walk the list of cells within line bounds
      for (int x=(int)lmin.x; x<(int)lmax.x; ++x) {
        for (int y=(int)lmin.y; y<(int)lmax.y; ++y) {

          encodeLine( line.p0, line.p1, ivec2(x,y), cpuStream, cpuCounters );

        }//y
      }//x
    }//lines


    //Walk the list of quad segments
    for (Uint32 q=0; q < obj->quads.size(); ++q) {

      //Find quad bounds
      Quad &quad= obj->quads[q];
      Vec2 qmin = Vec::Min( quad.p0, Vec::Min( quad.p1, quad.p2 ) );
      Vec2 qmax = Vec::Max( quad.p0, Vec::Max( quad.p1, quad.p2 ) );

      //Transform quad bounds into grid space
      qmin = Vec::Floor( (qmin - min) / cellSize );
      qmax = Vec::Ceil( (qmax - min) / cellSize );

      //Walk the list of cells within quad bounds
      for (int x=(int)qmin.x; x<(int)qmax.x; ++x) {
        for (int y=(int)qmin.y; y<(int)qmax.y; ++y) {

          encodeQuad( quad.p0, quad.p1, quad.p2, ivec2(x,y), cpuStream, cpuCounters );

        }//y
      }//x
    }//quads


    //Transform object bounds into grid space
    Vec2 omin = Vec::Floor( (obj->min - min) / cellSize );
    Vec2 omax = Vec::Ceil( (obj->max - min) / cellSize );

    //Walk the list of cells within object bounds
    for (int x=(int)omin.x; x<(int)omax.x; ++x) {
      for (int y=(int)omin.y; y<(int)omax.y; ++y) {

        encodeObject( obj->color, ivec2(x,y), cpuStream, cpuCounters );

      }//y
    }//x
  }//objects

  cpuStreamLen = imageLoad( cpuCounters, ivec3(0,0,0) ).x;
}

#endif
