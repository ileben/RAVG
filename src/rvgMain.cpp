#include "rvgMain.h"
#include "rvgGLHeaders.h"
#include "rvgShaders.h"
#include "rvgVectors.h"

///////////////////////////////////////////////////////////////////
// Forward declarations

class Contour;
class Object;

///////////////////////////////////////////////////////////////////
// Classes

namespace SegSpace {
  enum Enum {
    Absolute = (1 << 7),
    Relative = (1 << 7)
  };
};

namespace SegType {
  enum Enum {
    Close        = 0,
    
    MoveTo       = 1,
    LineTo       = 2,
    QuadTo       = 3,
    CubicTo      = 4,

    MoveToAbs    = (SegType::MoveTo  | SegSpace::Absolute),
    LineToAbs    = (SegType::LineTo  | SegSpace::Absolute),
    QuadToAbs    = (SegType::QuadTo  | SegSpace::Absolute),
    CubicToAbs   = (SegType::CubicTo | SegSpace::Absolute),

    MoveToRel    = (SegType::MoveTo  | SegSpace::Relative),
    LineToRel    = (SegType::LineTo  | SegSpace::Relative),
    QuadToRel    = (SegType::QuadTo  | SegSpace::Relative),
    CubicToRel   = (SegType::CubicTo | SegSpace::Relative)
  };
};

struct Line
{
  Vec2 p1;
  Vec2 p2;

  Line() {};
  Line( Float x1, Float y1, Float x2, Float y2 ) {
    p1.set( x1,y1 ); p2.set( x2,y2 );
  }
};

struct Quad
{
  Vec2 p1;
  Vec2 p2;
  Vec2 p3;

  Quad() {};
  Quad( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3 ) {
    p1.set( x1,y1 ); p2.set( x2,y2 ); p3.set( x3,y3 );
  }
};

struct Cubic
{
  Vec2 p1;
  Vec2 p2;
  Vec2 p3;
  Vec2 p4;

  Cubic() {};
  Cubic( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3, Float x4, Float y4 ) {
    p1.set( x1,y1 ); p2.set( x2,y2 ); p3.set( x3,y3 ); p4.set( x4,y4 );
  }
};

class Contour
{
public:
  friend class Object;

  std::vector< int > segments;
  std::vector< Vec2 > points;
  std::vector< Vec2 > flatPoints;

public:
};

class Object
{
  Contour *contour;
  bool penDown;
  Vec2 pen;
  Vec2 start;

public:

  Vec2 min;
  Vec2 max;

public:

  bool buffersInit;
  GLuint quadBuffer;
  GLuint abcBuffer;
  GLuint texStream;

public:

  std::vector< Contour* > contours;
  std::vector< Line > lines;
  std::vector< Cubic > cubics;
  std::vector< Quad > quads;

public:

  Object();

  void moveTo( Float x, Float y,
    SegSpace::Enum space = SegSpace::Absolute );

  void lineTo( Float x, Float y,
    SegSpace::Enum space = SegSpace::Absolute );

  void quadTo( Float x1, Float y1, Float x2, Float y2,
    SegSpace::Enum space = SegSpace::Absolute );

  void cubicTo( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3,
    SegSpace::Enum space = SegSpace::Absolute );

  void close();

  void updateBuffers();
  void updateBounds();
};

class Shader
{
  std::string vertFile;
  std::string fragFile;

public:

  GLProgram *program;
  GLShader *vertex;
  GLShader *fragment;

public:

  Shader (const std::string &vertFile, const std::string &fragFile);
  ~Shader ();
  bool load ();
  void use();
};


////////////////////////////////////////////////////////////////////
// Global vars

int resX = 400;
int resY = 400;
Shader *shader1;
Shader *shaderQuad;
Shader *shaderStream;
Object *object1;

///////////////////////////////////////////////////////////////////
// Functions

Object::Object()
{
  contour = NULL;
  penDown = false;
  pen.set( 0,0 );
  buffersInit = false;
}

void Object::moveTo( Float x, Float y,
  SegSpace::Enum space )
{
  contour = new Contour();
  contours.push_back( contour );

  contour->segments.push_back( SegType::MoveTo | space );
  contour->points.push_back( Vec2(x,y) );
  contour->flatPoints.push_back( Vec2(x,y) );

  pen.set( x,y );
  start.set( x,y );
  penDown = true;
}

void Object::lineTo( Float x, Float y,
  SegSpace::Enum space )
{
  if (!penDown) return;

  contour->segments.push_back( SegType::LineTo | space );
  contour->points.push_back( Vec2(x,y) );
  contour->flatPoints.push_back( Vec2(x,y) );

  lines.push_back( Line( pen.x,pen.y, x, y ));
  pen.set( x,y );
}

void Object::quadTo( Float x1, Float y1, Float x2, Float y2,
  SegSpace::Enum space )
{
  if (!penDown) return;

  contour->segments.push_back( SegType::QuadTo | space );
  contour->points.push_back( Vec2(x1,y1) );
  contour->points.push_back( Vec2(x2,y2) );
  contour->flatPoints.push_back( Vec2(x2,y2) );

  quads.push_back( Quad( pen.x,pen.y, x1,y1, x2,y2 ));
  pen.set( x2,y2 );
}

void Object::cubicTo( Float x1, Float y1, Float x2, Float y2, Float x3, Float y3,
  SegSpace::Enum space )
{
  if (!penDown) return;

  contour->segments.push_back( SegType::CubicTo | space );
  contour->points.push_back( Vec2(x1,y1) );
  contour->points.push_back( Vec2(x2,y2) );
  contour->points.push_back( Vec2(x3,y3) );
  contour->flatPoints.push_back( Vec2(x3,y3) );

  cubics.push_back( Cubic( pen.x,pen.y, x1,y1, x2,y2, x3,y3 ));
  pen.set( x3,y3 );
}

void Object::close()
{
  if (!penDown) return;

  contour->segments.push_back( SegType::Close );

  lines.push_back( Line( pen.x,pen.y, start.x,start.y ));
  pen.set( start.x, start.y );
  penDown = false;
}

void Object::updateBounds()
{
  for (Uint32 c=0; c<contours.size(); ++c)
  {
    for (Uint32 p=0; p<contours[c]->points.size(); ++p)
    {
      Vec2 &point = contours[c]->points[p];
      if (c==0 && p==0)
      {
        min = max = point;
        continue;
      }

      if (point.x < min.x) min.x = point.x;
      if (point.y < min.y) min.y = point.y;

      if (point.x > max.x) max.x = point.x;
      if (point.y > max.y) max.y = point.y;
    }
  }
}

void Object::updateBuffers()
{
  if (!buffersInit)
  {
    glGenBuffers( 1, &quadBuffer );
    glGenBuffers( 1, &abcBuffer );
    glGenTextures( 1, &texStream );
    buffersInit = true;
  }

  //////////////////////////////////////
  //Quadratic control points

  glBindBuffer( GL_ARRAY_BUFFER, quadBuffer );
  glBufferData( GL_ARRAY_BUFFER, quads.size() * sizeof( Quad ), &quads[0], GL_STATIC_DRAW );

  //////////////////////////////////////
  //Quadratic interpolation ABC values

  Vec3 *abcData = new Vec3[ quads.size() * 3 * 2 ];
  for (Uint32 q=0; q<quads.size(); ++q)
  {
    //Implicit-equation-space coordinates
    float u1 = 0.0f, u2 = 0.5f, u3 = 1.0f;
    float v1 = 0.0f, v2 = 0.0f, v3 = 1.0f;

    float P,Q,R,S,T,U,det,idet;
    Vec3 ABCu, ABCv;

    Quad &quad = quads[q];
    P = quad.p1.x - quad.p2.x;
    Q = quad.p1.y - quad.p2.y;
    R = quad.p1.x - quad.p3.x;
    S = quad.p1.y - quad.p3.y;
    det = P*S - R*Q;
    idet = 1.0f / det;

    T = u1 - u2;
    U = u1 - u3;
    ABCu.x = (T*S - U*Q) * idet;
    ABCu.y = (P*U - R*T) * idet;
    ABCu.z = u1 - ABCu.x * quad.p1.x - ABCu.y * quad.p1.y;

    T = v1 - v2;
    U = v1 - v3;
    ABCv.x = (T*S - U*Q) * idet;
    ABCv.y = (P*U - R*T) * idet;
    ABCv.z = v1 - ABCv.x * quad.p1.x - ABCv.y * quad.p1.y;

    abcData[ q*6+0 ] = ABCu;
    abcData[ q*6+1 ] = ABCv;
    abcData[ q*6+2 ] = ABCu;
    abcData[ q*6+3 ] = ABCv;
    abcData[ q*6+4 ] = ABCu;
    abcData[ q*6+5 ] = ABCv;
  }
  
  glBindBuffer( GL_ARRAY_BUFFER, abcBuffer );
  glBufferData( GL_ARRAY_BUFFER, quads.size() * 3 * 2 * sizeof( Vec3 ), abcData, GL_STATIC_DRAW );
  delete abcData;

  //////////////////////////////////////
  //Encode contour into stream

  std::vector< Float > stream;
  stream.push_back( (Float) lines.size() );
  stream.push_back( (Float) quads.size() );
  //stream.push_back( 0.0f );
  //stream.push_back( 0.0f );

  for (Uint32 l=0; l<lines.size(); ++l)
  {
    Line &line = lines[l];
    stream.push_back( line.p1.x );
    stream.push_back( line.p1.y );
    stream.push_back( line.p2.x );
    stream.push_back( line.p2.y );
  }

  for (Uint32 q=0; q<quads.size(); ++q)
  {
    Quad &quad = quads[q];
    stream.push_back( quad.p1.x );
    stream.push_back( quad.p1.y );
    stream.push_back( quad.p2.x );
    stream.push_back( quad.p2.y );
    stream.push_back( quad.p3.x );
    stream.push_back( quad.p3.y );
  }

  Uint32 pad = 4 - (stream.size() % 4);
  for (Uint32 p=0; p<pad; ++p)
    stream.push_back( 0.0f );
  
  glBindTexture( GL_TEXTURE_1D, texStream );
  glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA32F, stream.size() / 4, 0, GL_RGBA, GL_FLOAT, &stream[0] );
  glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}


Shader::Shader (const std::string &vertFile, const std::string &fragFile)
{
  this->vertFile = vertFile;
  this->fragFile = fragFile;

  vertex = NULL;
  fragment = NULL;
  program = NULL;
}

Shader::~Shader ()
{
  if (vertex) delete vertex;
  if (fragment) delete fragment;
  if (program) delete program;
}

std::string readFile (const std::string &filename)
{
  std::ifstream f;
  f.open( filename.c_str(), std::ios::binary );
  if (!f.is_open()) {
    std::cout << "Failed opening file!" << std::endl;
    return std::string( "" );
  }

  long begin = f.tellg();
  f.seekg( 0, std::ios::end );
  long end = f.tellg();
  f.seekg( 0, std::ios::beg );
  long size = end - begin;

  if (size == 0) {
    std::cout << "Zero file size!"  << std::endl;
    return std::string( "" );
  }

  char *cstr = new char[ size+1 ];
  f.read( cstr, size );
  f.close();
  cstr[ size ] = '\0';

  std::string str( cstr );
  delete[] cstr;

  return str;
}

bool Shader::load()
{
  if (vertex) delete vertex;
  if (fragment) delete fragment;
  if (program) delete program;

  program = new GLProgram();
  vertex = new GLShader();
  fragment = new GLShader();

  //Vertex
  std::cout << "Loading shader " << vertFile << "..." << std::endl;
  std::string vertString = readFile( vertFile );

  vertex->create( ShaderType::Vertex );
  if (vertex->compile( vertString ))
    std::cout << "Vertex compiled successfuly" << std::endl;
  else std::cout << "Vertex compilation failed" << std::endl;

  //std::string vertLog = vertex->getInfoLog();
  //if (vertLog.length() > 0)
    //std::cout << vertLog << std::endl;

  //Fragment
  std::cout << "Loading shader " << fragFile << "..." << std::endl;
  std::string fragString = readFile( fragFile );

  fragment->create( ShaderType::Fragment );
  if (fragment->compile( fragString ))
    std::cout << "Fragment compiled successfuly" << std::endl;
  else std::cout << "Fragment compilation failed" << std::endl;
  
  //std::string fragLog = fragment->getInfoLog();
  //if (fragLog.length() > 0)
    //std::cout << fragLog << std::endl;

  //Program
  program->create();
  program->attach( vertex );
  program->attach( fragment );
  
  if (program->link())
    std::cout << "Program linked successfully" << std::endl;
  else std::cout << "Program link failed" << std::endl;

  std::string linkLog = program->getInfoLog();
  if (linkLog.length() > 0)
    std::cout << linkLog << std::endl;

  //std::cout << program->getInfoLog() << std::endl;

  return true;
}

void Shader::use()
{
  if (program) program->use();
}

void renderQuads (Object *o, Shader *shader)
{
  Int32 pos = shader->program->getAttribute( "in_pos" );
  glEnableVertexAttribArray( pos );
  glBindBuffer( GL_ARRAY_BUFFER, o->quadBuffer );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

  Int32 abcU = shader->program->getAttribute( "in_abcU" );
  Int32 abcV = shader->program->getAttribute( "in_abcV" );
  glEnableVertexAttribArray( abcU );
  glEnableVertexAttribArray( abcV );
  glBindBuffer( GL_ARRAY_BUFFER, o->abcBuffer );
  glVertexAttribPointer( abcU, 3, GL_FLOAT, false, 2 * sizeof( Vec3 ), 0 );
  glVertexAttribPointer( abcV, 3, GL_FLOAT, false, 2 * sizeof( Vec3 ), (GLvoid*) sizeof( Vec3 ) );
  
  glDrawArrays( GL_TRIANGLES, 0, o->quads.size() * 3 );
  glDisableVertexAttribArray( pos );
}

void renderObject (Object *o)
{
  glColor3f( 0,0,0 );

  Shader *shader = shaderStream;
  shader->use();

  Int32 sampStream = shader->program->getUniform( "sampStream" );
  glUniform1i( sampStream, 0 );

  glEnable( GL_TEXTURE_1D );
  glBindTexture( GL_TEXTURE_1D, o->texStream );

  
  Float verts[] = {
    o->min.x, o->min.y,
    o->max.x, o->min.y,
    o->max.x, o->max.y,
    o->min.x, o->max.y
  };

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, 2 * sizeof( Float ), verts );
  
  glEnableVertexAttribArray( pos );
  glDrawArrays( GL_QUADS, 0, 4 );
  glDisableVertexAttribArray( pos );
  
  /*
  glBegin( GL_QUADS );
  glVertex2f( o->min.x, o->min.y );
  glVertex2f( o->max.x, o->min.y );
  glVertex2f( o->max.x, o->max.y );
  glVertex2f( o->min.x, o->max.y );
  glEnd();
*/
  //-----------------------------------------------

  /*
  //Vertex data needs to come from a buffer for gl_VertexID to be defined in shaders
  Shader *shader = shaderQuad;
  shader->use();

  //glEnable( GL_MULTISAMPLE );
  //glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
  
  renderQuads( o, shader );

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glEnableVertexAttribArray( pos );
  glBindBuffer( GL_ARRAY_BUFFER, o->quadBuffer );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );
  glDrawArrays( GL_TRIANGLES, 0, o->quads.size() * 3 );
  glDisableVertexAttribArray( pos );
  */

  /*
  //-----------------------------------------------

  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );

  glEnable( GL_STENCIL_TEST );
  glStencilFunc( GL_ALWAYS, 0, 1 );
  glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
  glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

  renderQuads( o, shader );

  glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );

  //-----------------------------------------------

  shader = shader1;
  shader->use();

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glEnableVertexAttribArray( pos );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  for (Uint32 c=0; c<o->contours.size(); ++c)
  {
    Contour *contour = o->contours[c];
    glVertexAttribPointer( pos, 2, GL_FLOAT, false, sizeof( Vec2 ), &contour->flatPoints[0] );
    glDrawArrays( GL_TRIANGLE_FAN, 0, contour->flatPoints.size() * 2 );
  }

  glDisableVertexAttribArray( pos );

  //-----------------------------------------------

  glStencilFunc( GL_EQUAL, 1, 1 );
  glStencilOp( GL_ZERO, GL_ZERO, GL_ZERO );
  glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

  glBegin( GL_QUADS );
  glVertex2f( o->min.x, o->min.y );
  glVertex2f( o->max.x, o->min.y );
  glVertex2f( o->max.x, o->max.y );
  glVertex2f( o->min.x, o->max.y );
  glEnd(); */
}

void display ()
{
  glClearColor( 1,1,1,1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

  renderObject( object1 );

  //Check fps
  static int fps = 0;
  static int lastUpdate = glutGet( GLUT_ELAPSED_TIME );
  fps++;

  int now = glutGet( GLUT_ELAPSED_TIME );
  if (now - lastUpdate > 1000) {
    std::cout << "Fps: " << fps << std::endl;
    lastUpdate = now;
    fps = 0;
  }

  //Check for errors
  static GLenum err = GL_NO_ERROR;
  GLenum newErr = glGetError();
  if (newErr != GL_NO_ERROR && newErr != err)
    std::cout << "GLERROR: 0x" << std::hex  << int(newErr) << std::dec << std::endl;
  err = newErr;
  glGetError();

  glutSwapBuffers();
}

void reshape (int w, int h)
{
  std::cout << "reshape (" << w << "," << h << ")" << std::endl;

  resX = w;
  resY = h;

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  //gluOrtho2D( 0, w, 0, h );
  gluOrtho2D( 0,400,0,400 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glViewport( 0,0,w,h );
}

void animate ()
{
  glutPostRedisplay();
}

void specialKey (int key, int x, int y)
{
  if (key == GLUT_KEY_F7)
  {
    std::cout << "Compiling..." << std::endl;
    shader1->load();
    shaderQuad->load();
    shaderStream->load();
  }
}

void rvgGlutInit (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitContextVersion( 3, 0 );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_STENCIL );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  glutCreateWindow( "Random-access Vector Graphics" );

  //glutGameModeString( "1280x1024:32@60" );
  //glutEnterGameMode();
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  //glutKeyboardFunc( keyDown );
  //glutKeyboardUpFunc( keyUp );
  glutSpecialFunc( specialKey );
  //glutMouseFunc( mouseClick );
  //glutMotionFunc( mouseMove );
  //glutPassiveMotionFunc( mouseMove );
  glutIdleFunc( animate );
}

void calcSamples()
{
  float off[4] = { -0.3f, -0.1f, +0.1f, +0.3f };
  float a = PI / 6;

  std::ofstream file( "samples.txt", std::ios::out | std::ios::binary );
  if (!file.is_open())
    return;

  int s=0;
  for (int x=0; x<4; ++x) {
    for (int y=0; y<4; ++y) {

      float newx = COS(a) * off[x] - SIN(a) * off[y];
      float newy = SIN(a) * off[x] + COS(a) * off[y];
      file << "samples[" << s << "] = vec2( " << newx << ", " << newy << " );\r\n";
      s++;
    }
  }

  file.close();
}

int main (int argc, char **argv)
{
  rvgGlutInit( argc, argv );
  rvgGLInit();
  glSwapInterval( 0 );

  shader1 = new Shader( "shader.vert.c", "shader.frag.c" );
  shader1->load();

  shaderQuad = new Shader( "curvequad.vert.c", "curvequad.frag.c" );
  shaderQuad->load();

  shaderStream = new Shader( "stream.vert.c", "stream.frag.c" );
  shaderStream->load();

  object1 = new Object();
  object1->moveTo( 100,100 );
  object1->quadTo( 150,140, 200,100 );
  object1->lineTo( 220,200 );
  object1->quadTo( 170,220, 120,200 );
  object1->close();
  object1->updateBuffers();
  object1->updateBounds();

  glutMainLoop();
}
