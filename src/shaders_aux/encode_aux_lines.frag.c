#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform int* ptrObjects;
uniform coherent int* ptrInfo;
uniform coherent int* ptrGrid;
uniform coherent float* ptrStream;

uniform vec2 gridOrigin;
uniform ivec2 gridSize;
uniform vec2 cellSize;
uniform int objectId;

in layout( pixel_center_integer ) vec4 gl_FragCoord;
flat in vec2 line0;
flat in vec2 line1;

int addLine (vec2 l0, vec2 l1, coherent int *ptrObjCell);
void lineIntersectionY (vec2 l0, vec2 l1, float y, float minX, float maxX,
                        out bool found, out float x);

void main()
{
  //Check grid coordinate in range
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { discard; return; }

  bool found = false, inside = false;
  float yy = 0.0, xx = 0.0;

  //Get object pointer and grid info
  int *ptrObj = ptrObjects + objectId * 5;
  ivec2 objGridOrigin = ivec2( ptrObj[0], ptrObj[1] );
  ivec2 objGridSize   = ivec2( ptrObj[2], ptrObj[3] );
  int   objGridOffset = ptrObj[4];
  
  //Get object grid pointer
  coherent int *ptrObjGrid = ptrGrid + objGridOffset;

  //Check object grid coordinate in range
  ivec2 objGridCoord = gridCoord - objGridOrigin;
  if (objGridCoord.x < 0 || objGridCoord.x >= objGridSize.x ||
      objGridCoord.y < 0 || objGridCoord.y >= objGridSize.y)
  { discard; return; }

  //Get object grid cell pointer
  coherent int* ptrObjCell = ptrObjGrid
    + (objGridCoord.y * objGridSize.x + objGridCoord.x) * NUM_OBJCELL_COUNTERS;

  //Find cell origin
  vec2 cmin = gridOrigin + gridCoord * cellSize;

  //Transform line coords into cell space
  vec2 l0 = (line0 - cmin) / cellSize;
  vec2 l1 = (line1 - cmin) / cellSize;

  //Check if line intersects bottom edge
  lineIntersectionY( l0, l1, 1.0, 0.0, 1.0, found, xx );
  if (found)
  {
    //Walk all the cells to the left
    for (int x = (int)objGridCoord.x - 1; x >= 0; --x)
    {
      //Get object grid cell pointer
      coherent int *ptrObjCellAux = ptrObjGrid
        + (objGridCoord.y * objGridSize.x + x) * NUM_OBJCELL_COUNTERS;

      //Increase auxiliary vertical counter
      atomicAdd( ptrObjCellAux + OBJCELL_COUNTER_AUX, 1 );
    }
    inside = true;
  }

  //Check if line intersects right edge
  lineIntersectionY( l0.yx, l1.yx, 1.0, 0.0, 1.0, found, yy );
  if (found)
  {
    //Add line spanning from intersection point to upper-right corner
    addLine( vec2( 1.0, yy ), vec2( 1.0, -0.25 ), ptrObjCell );
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
    addLine( l0,l1, ptrObjCell );
  }

  discard;
}

int addLine (vec2 l0, vec2 l1, coherent int *ptrObjCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 6 );
  int prevOffset = atomicExchange( ptrObjCell + OBJCELL_COUNTER_PREV, nodeOffset );
  coherent float *ptrNode = ptrStream + nodeOffset;

  //Store line data
  ptrNode[ 0 ] = (float) NODE_TYPE_LINE;
  ptrNode[ 1 ] = (float) prevOffset;
  ptrNode[ 2 ] = l0.x;
  ptrNode[ 3 ] = l0.y;
  ptrNode[ 4 ] = l1.x;
  ptrNode[ 5 ] = l1.y;

  return nodeOffset;
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
