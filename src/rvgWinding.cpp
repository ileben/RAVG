#include "rvgMain.h"

int lineWinding (Vec2 l0, Vec2 l1, Vec2 p0, Vec2 p1)
{
  int w = 0;

  if (l0 == l1) return 0;
  if (p0 == p1) return 0;

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
  if (ax == 0.0f) {
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

int quadWinding (Vec2 q0, Vec2 q1, Vec2 q2, Vec2 p0, Vec2 p1)
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
  if (ax == 0.0f) {
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

void lineIntersectionY (vec2 l0, vec2 l1, float y, bool &found, float &x)
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
      found = true;
    }
  }
}

void quadIntersectionY (vec2 q0, vec2 q1, vec2 q2, float y,
                        bool &found1, bool &found2, float &x1, float &x2)
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
      float one_t = 1.0f - t;
      x1 = one_t*one_t * q0.x + 2*t*one_t * q1.x + t*t * q2.x;
      found1 = true;
    }

    if (t2 >= 0.0 && t2 <= 1.0) {
      float t = t2;
      float one_t = 1.0f - t;
      x2 = one_t*one_t * q0.x + 2*t*one_t * q1.x + t*t * q2.x;
      found2 = true;
    }
  }
}

bool pointInRect (const Vec2 &min, const Vec2 &max, const Vec2 &p)
{
  return (p.x >= min.x && p.x <= max.x &&
          p.y >= min.y && p.y <= max.y);
}

bool lineInCell (const Vec2 &min, const Vec2 &max, const Line &line)
{
  Vec2 points[2] = { line.p0, line.p1 };
  for (int p=0; p<2; ++p)
    if (pointInRect( min, max, points[p] ))
      return true;
  return false;
}

bool quadInCell (const Vec2 &min, const Vec2 &max, const Quad &quad)
{
  Vec2 points[3] = { quad.p0, quad.p1, quad.p2 };
  for (int p=0; p<3; ++p)
    if (pointInRect( min, max, points[p] ))
      return true;
  return false;
}
