#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable

uniform int* ptrGrid;
uniform float *ptrStream;
uniform vec2 cellSize;
uniform ivec2 gridSize;
uniform vec2 gridOrigin;

smooth in vec2 f_tex;

out vec4 out_color;

ivec2 gridCoord;

int lineWinding (vec2 l0, vec2 l1, vec2 p0, vec2 p1);
int quadWinding (vec2 q0, vec2 q1, vec2 q2, vec2 p0, vec2 p1);

void main (void)
{
  int objIndex = 0;
  int segIndex = 0;
  int safetyCounter = 0;
  vec3 finalColor = vec3( 1,1,1 );

  //Find grid coordinate
  gridCoord = ivec2(floor( (f_tex - gridOrigin) / cellSize ));

  //Check if coordinate in range
  if (gridCoord.x >= 0 && gridCoord.x < gridSize.x &&
      gridCoord.y >= 0 && gridCoord.y < gridSize.y)
  {
    //Find cell origin
    vec2 cmin = gridOrigin + gridCoord * cellSize;

    //Loop until end of object list
    objIndex = ptrGrid[ (gridCoord.y * gridSize.x + gridCoord.x) * NUM_CELL_COUNTERS ];
    while (objIndex != -1)
    {
      //Sanity check
      //if (++safetyCounter >= 10000) { finalColor = vec3(1,0,0); break; }
      //if (objIndex < -1 || objIndex > 500000)
        //break;

      //Get object type and link to previous object
      float *ptrObj = ptrStream + objIndex;
      int objType = (int) ptrObj[0];
      objIndex = (int) ptrObj[1];

      //Check object type
      if (objType == NODE_TYPE_OBJECT)
      {
        //Get color of the object
        int objId = (int) ptrObj[2];
        int objWinding = (int) ptrObj[3];
        vec4 objColor = vec4( ptrObj[4], ptrObj[5], ptrObj[6], ptrObj[7] );

        //Loop until end of segment list
        segIndex = (int) ptrObj[8];
        while (segIndex != -1)
        {
          //Sanity check
          //if (++safetyCounter >= 10000) { finalColor = vec3(1,0,0); break; }
          //if (segIndex < -1 || segIndex > 500000)
            //break;

          //Get segment type and link to previous segment
          float *ptrSeg = ptrStream + segIndex;
          int segType = (int) ptrSeg[0];
          segIndex = (int) ptrSeg[1];

          //Check segment type
          if (segType == NODE_TYPE_LINE)
          {
            vec2 l0 = vec2( ptrSeg[2], ptrSeg[3] );
            vec2 l1 = vec2( ptrSeg[4], ptrSeg[5] );
            
            //Update winding number
            bool found; float xx;
            vec2 pp = clamp( (f_tex - cmin) / cellSize, vec2(0), vec2(1) );
            objWinding += lineWinding( l0,l1, vec2(0.5,0.5), pp );
          }
          else if (segType == NODE_TYPE_QUAD)
          {
            vec2 q0 = vec2( ptrSeg[2], ptrSeg[3] );
            vec2 q1 = vec2( ptrSeg[4], ptrSeg[5] );
            vec2 q2 = vec2( ptrSeg[6], ptrSeg[7] );

            //Update winding number
            bool found1, found2; float xx1,xx2;
            vec2 pp = clamp( (f_tex - cmin) / cellSize, vec2(0), vec2(1) );
            objWinding += quadWinding( q0,q1,q2, vec2(0.5,0.5), pp );
          }
          else break;
        }

        //Merge object color
        if (objWinding % 2 == 1)
          finalColor = objColor.rgb;
      }
      else break;
    }
  }

  out_color = vec4( finalColor, 1 );
}

int lineWinding (vec2 l0, vec2 l1, vec2 p0, vec2 p1)
{
  int w = 0;

  //Line equation
  float AX = (l1.x - l0.x);
  float BX = l0.x;

  float AY = (l1.y - l0.y);
  float BY = l0.y;

  //Line from pivot to fragment point
  float ax = (p1.x - p0.x);
  float bx = p0.x;

  float ay = (p1.y - p0.y);
  float by = p0.y;

  //Check for vertical line
  if (ax == 0.0) {
    float tmp;
    tmp = ax; ax = ay; ay = tmp;
    tmp = bx; bx = by; by = tmp;
    tmp = AX; AX = AY; AY = tmp;
    tmp = BX; BX = BY; BY = tmp;
  }

  //Intersection equation
  float r = ay / ax;
  float a = (AY - r * AX);
  float b = (BY - r * BX) - (by - r * bx);

  //Find root
  float t = -b/a;
  if (t >= 0.0 && t <= 1.0) {
    float k = t * AX/ax + BX/ax - bx/ax;
    if (k >= 0.0 && k <= 1.0) w++;
  }
  
  return w;
}

int quadWinding (vec2 q0, vec2 q1, vec2 q2, vec2 p0, vec2 p1)
{
  int w = 0;

  //Quadratic equation
  float AX = (q0.x - 2*q1.x + q2.x);
  float BX = (-2*q0.x + 2*q1.x);
  float CX = q0.x;

  float AY = (q0.y - 2*q1.y + q2.y);
  float BY = (-2*q0.y + 2*q1.y);
  float CY = q0.y;

  //Line from pivot to fragment point
  float ax = (p1.x - p0.x);
  float bx = p0.x;

  float ay = (p1.y - p0.y);
  float by = p0.y;

  //Check for vertical line
  if (ax == 0.0) {
    float tmp;
    tmp = ax; ax = ay; ay = tmp;
    tmp = bx; bx = by; by = tmp;
    tmp = AX; AX = AY; AY = tmp;
    tmp = BX; BX = BY; BY = tmp;
    tmp = CX; CX = CY; CY = tmp;
  }

  //Intersection equation
  float r = ay / ax;
  float a = (AY - r * AX);
  float b = (BY - r * BX);
  float c = (CY - r * CX) - (by - r * bx);

  //Discriminant
  float d = b*b - 4*a*c;

  //Find roots
  if (d > 0.0) {

    float sd = sqrt( d );
    float t1 = (-b - sd) / (2*a);
    float t2 = (-b + sd) / (2*a);
    
    if (t1 >= 0.0 && t1 <= 1.0) {
      float k1 = t1*t1 * AX/ax + t1 * BX/ax + CX/ax - bx/ax;
      if (k1 >= 0.0 && k1 <= 1.0) w++;
    }

    if (t2 >= 0.0 && t2 <= 1.0) {
      float k2 = t2*t2 * AX/ax + t2 * BX/ax + CX/ax - bx/ax;
      if (k2 >= 0.0 && k2 <= 1.0) w++;
    }
  }

  return w;
}
