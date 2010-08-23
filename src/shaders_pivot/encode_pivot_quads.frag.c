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
flat in vec2 quad0;
flat in vec2 quad1;
flat in vec2 quad2;

int addLine (vec2 l0, vec2 l1, coherent int *ptrObjCell);
int addQuad (vec2 q0, vec2 q1, vec2 q2, coherent int *ptrObjCell);
void quadIntersectionY (vec2 q0, vec2 q1, vec2 q2, float y,
                        out bool found1, out bool found2, out float x1, out float x2);

void main()
{
  //Check grid coordinate in range
  ivec2 gridCoord = ivec2( gl_FragCoord.xy );
  if (gridCoord.x < 0 || gridCoord.x >= gridSize.x ||
      gridCoord.y < 0 || gridCoord.y >= gridSize.y)
  { discard; return; }

  bool found1=false, found2=false, inside=false;
  float yy1=0.0, yy2=0.0, xx1=0.0, xx2=0.0;

  //Get object pointer and object grid info
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

  //Transform quad coords into cell space
  vec2 q0 = (quad0 - cmin) / cellSize;
  vec2 q1 = (quad1 - cmin) / cellSize;
  vec2 q2 = (quad2 - cmin) / cellSize;

  //Check if quad intersects horizontal ray from pivot towards +x
  quadIntersectionY( q0, q1, q2, 0.5, found1, found2, xx1, xx2 );
  if ((found1 && xx1 >= 0.5) != (found2 && xx2 >= 0.5))
  {
    //Increase winding number
    atomicAdd( ptrObjCell + OBJCELL_COUNTER_WIND, 1 );
  }

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
      quadIntersectionY( q0, q1, q2, 0.0, found1, found2, xx1, xx2 );
      if ((found1 && xx1 >= 0.0 && xx1 <= 1.0) || (found2 && xx2 >= 0.0 && xx2 <= 1.0))
        inside = true;
      else
      {
        //Check if quad intersects bottom edge
        quadIntersectionY( q0, q1, q2, 1.0, found1, found2, xx1, xx2 );
        if ((found1 && xx1 >= 0.0 && xx1 <= 1.0) || (found2 && xx2 >= 0.0 && xx2 <= 1.0))
          inside = true;
        else
        {
          //Check if quad intersects left edge
          quadIntersectionY( q0.yx, q1.yx, q2.yx, 0.0, found1, found2, yy1, yy2 );
          if ((found1 && yy1 >= 0.0 && yy1 <= 1.0) || (found2 && yy2 >= 0.0 && yy2 <= 1.0))
            inside = true;
          else
          {
            //Check if quad intersects right edge
            quadIntersectionY( q0.yx, q1.yx, q2.yx, 1.0, found1, found2, yy1, yy2 );
            if ((found1 && yy1 >= 0.0 && yy1 <= 1.0) || (found2 && yy2 >= 0.0 && yy2 <= 1.0))
              inside = true;
          }
        }
      }
    }
  }

  if (inside)
  {
    //Add quad data to stream
    addQuad( q0,q1,q2, ptrObjCell );
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

int addQuad (vec2 q0, vec2 q1, vec2 q2, coherent int *ptrObjCell)
{
  //Get stream size and previous node offset
  //Store new stream size and current node offset
  int nodeOffset = atomicAdd( ptrInfo + INFO_COUNTER_STREAMLEN, 8 );
  int prevOffset = atomicExchange( ptrObjCell + OBJCELL_COUNTER_PREV, nodeOffset );
  coherent float *ptrNode = ptrStream + nodeOffset;

  //Store quad data
  ptrNode[ 0 ] = (float) NODE_TYPE_QUAD;
  ptrNode[ 1 ] = (float) prevOffset;
  ptrNode[ 2 ] = q0.x;
  ptrNode[ 3 ] = q0.y;
  ptrNode[ 4 ] = q1.x;
  ptrNode[ 5 ] = q1.y;
  ptrNode[ 6 ] = q2.x;
  ptrNode[ 7 ] = q2.y;

  return nodeOffset;
}

void quadIntersectionY (vec2 q0, vec2 q1, vec2 q2, float y,
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
      found1 = true;
    }

    if (t2 >= 0.0 && t2 <= 1.0) {
      float t = t2;
      float one_t = 1.0 - t;
      x2 = one_t*one_t * q0.x + 2*t*one_t * q1.x + t*t * q2.x;
      found2 = true;
    }
  }
}
