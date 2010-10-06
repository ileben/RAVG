#include "rvgMain.h"

////////////////////////////////////////////////////////////////////
// Global vars

int resX = 500;
int resY = 600;
int gridResX = 60;
int gridResY = 60;


namespace Proc {
  enum Enum {
    Cpu   = 0,
    Gpu   = 1,
    Count = 2
  };
};

namespace Rep {
  enum Enum {
    Aux   = 0,
    Pivot = 1,
    Count = 2
  };
};

namespace View {
  enum Enum {
    Classic         = 0,
    RandomDirect    = 1,
    RandomCylinder  = 2,
    Count           = 3
  };
};

namespace Source {
  enum Enum {
    Tiger = 0,
    Text  = 1,
    Count = 2
  };
};

int proc = Proc::Gpu;
int rep = Rep::Pivot;
int view = View::RandomDirect;
int source = Source::Tiger;

bool encode = true;
bool draw = true;
bool drawGrid = false;

MatrixStack matModelView;
MatrixStack matProjection;
MatrixStack matTexture;

Shader *shaderGrid;

Shader *shaderRenderAux;
Shader *shaderRenderPivot;

Shader *shaderClassicQuads;
Shader *shaderClassicContour;
Shader *shaderClassic;

ImageEncoder *imageEncoderAux;
ImageEncoder *imageEncoderPivot;
ImageEncoderGpu *imageEncoderGpuAux;
ImageEncoderGpu *imageEncoderGpuPivot;

Image *imageTiger;
Image *imageText;

int mouseButton = 0;
Vec2 mouseDown;

Float angleX = 0.0f;
Float angleY = 0.0f;
Float zoomZ = 6.0f;

Float panX = 0.0f;
Float panY = 0.0f;
Float zoomS = 1.0f;

///////////////////////////////////////////////////////////////////
// Functions

void checkGlError (const std::string &text)
{
  static GLenum err = GL_NO_ERROR;
  GLenum newErr = glGetError();
  if (newErr != GL_NO_ERROR && newErr != err) {
    const GLubyte *errString = gluErrorString( newErr );
    std::cout << "GLERROR at '" << text << "': 0x" << std::hex  << int(newErr) << std::dec << " " << errString << std::endl;
  }
  err = newErr;
  glGetError();
}

void renderFullScreenQuad (Shader *shader)
{
  Float coords[] = {
    -1.0f, -1.0f,
    +1.0f, -1.0f,
    +1.0f, +1.0f,
    -1.0f, +1.0f
  };

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, 2 * sizeof( Float ), coords );

  glEnableVertexAttribArray( pos );
  glDrawArrays( GL_QUADS, 0, 4 );
  glDisableVertexAttribArray( pos );
}

void renderQuad (Shader *shader, Vec2 min, Vec2 max)
{
  Float coords[] = {
    min.x, min.y,
    max.x, min.y,
    max.x, max.y,
    min.x, max.y
  };

  Int32 pos = shader->program->getAttribute( "in_pos" );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glVertexAttribPointer( pos, 2, GL_FLOAT, false, 2 * sizeof( Float ), coords );

  glEnableVertexAttribArray( pos );
  glDrawArrays( GL_QUADS, 0, 4 );
  glDisableVertexAttribArray( pos );
}

void renderImageRandom (Shader *shader, Image *image, VertexBuffer *buf, GLenum mode)
{
  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );
  
  //glEnable( GL_BLEND );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  shader->use();

  Int32 uModelview = shader->program->getUniform( "modelview" );
  glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

  Int32 uProjection = shader->program->getUniform( "projection" );
  glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

  Int32 uMatTexture = shader->program->getUniform( "matTexture" );
  glUniformMatrix4fv( uMatTexture, 1, false, (GLfloat*) matTexture.top().m );

  Int32 uPtrGrid = shader->program->getUniform( "ptrGrid" );
  glUniformui64( uPtrGrid, image->ptrGpuGrid );

  Int32 uPtrStream = shader->program->getUniform( "ptrStream" );
  glUniformui64( uPtrStream, image->ptrGpuStream );

  Int32 uPtrObjects = shader->program->getUniform( "ptrObjects" );
  glUniformui64( uPtrObjects, image->ptrObjInfos );

  Int32 uCellSize = shader->program->getUniform( "cellSize" );
  glUniform2f( uCellSize, image->cellSize.x, image->cellSize.y );

  Int32 uGridSize = shader->program->getUniform( "gridSize" );
  glUniform2i( uGridSize, image->gridSize.x, image->gridSize.y );

  Int32 uGridOrigin = shader->program->getUniform( "gridOrigin" );
  glUniform2f( uGridOrigin, image->min.x, image->min.y );
  
  buf->render( shader, mode );

  //glDisable( GL_BLEND );
}

void renderImageClassic (Image *image)
{
  glEnable( GL_MULTISAMPLE );
  glEnable( GL_SAMPLE_SHADING );
  glMinSampleShading( 1.0f );

  glDisable( GL_DEPTH_TEST );
  glEnable( GL_STENCIL_TEST );

  for (Uint32 o=0; o<image->objects.size(); ++o)
  {
    Object *obj = image->objects[o];

    ///////////////////////////////////////////////
    //Render into stencil

    glStencilFunc( GL_ALWAYS, 0, 1 );
    glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

    ///////////////////////////////////////////////
    //Render quads
    {
      Shader *shader = shaderClassicQuads;
      shader->use();
      
      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );
      
      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glEnableVertexAttribArray( aPos );
      glBindBuffer( GL_ARRAY_BUFFER, obj->bufQuads );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );
      
      glDrawArrays( GL_TRIANGLES, 0, obj->quads.size() * 3 );
      glDisableVertexAttribArray( aPos );
    }
    
    ///////////////////////////////////////////////
    //Render contours
    {
      Shader *shader = shaderClassicContour;
      shader->use();
      
      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

      for (Uint32 c=0; c<obj->contours.size(); ++c)
      {
        Contour *cnt = obj->contours[c];

        Int32 aPos = shader->program->getAttribute( "in_pos" );
        glEnableVertexAttribArray( aPos );
        glBindBuffer( GL_ARRAY_BUFFER, cnt->bufFlatPoints );
        glVertexAttribPointer( aPos, 2, GL_FLOAT, false, sizeof( Vec2 ), 0 );

        glDrawArrays( GL_TRIANGLE_FAN, 0, cnt->flatPoints.size() );
        glDisableVertexAttribArray( aPos );
      }
    }

    ///////////////////////////////////////////////
    //Render through stencil

    glStencilFunc( GL_EQUAL, 1, 1 );
    glStencilOp( GL_ZERO, GL_ZERO, GL_ZERO );
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

    ///////////////////////////////////////////////
    //Render boundbox quad
    {
      Shader *shader = shaderClassic;
      shader->use();

      Int32 uModelview = shader->program->getUniform( "modelview" );
      glUniformMatrix4fv( uModelview, 1, false, (GLfloat*) matModelView.top().m );

      Int32 uProjection = shader->program->getUniform( "projection" );
      glUniformMatrix4fv( uProjection, 1, false, (GLfloat*) matProjection.top().m );

      Int32 uColor = shader->program->getUniform( "color" );
      glUniform4fv( uColor, 1, (GLfloat*) &obj->color );

      Float coords[] = {
        obj->min.x, obj->min.y,
        obj->max.x, obj->min.y,
        obj->max.x, obj->max.y,
        obj->min.x, obj->max.y
      };

      Int32 aPos = shader->program->getAttribute( "in_pos" );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
      glVertexAttribPointer( aPos, 2, GL_FLOAT, false, 2 * sizeof( Float ), coords );

      glEnableVertexAttribArray( aPos );
      glDrawArrays( GL_QUADS, 0, 4 );
      glDisableVertexAttribArray( aPos );
    }
  }

  glDisable( GL_STENCIL_TEST );
}

void renderImageClassic1to1 (Image *image, float offX=0.0f, float offY=0.0f, bool invert=false)
{
  //Render using current pan and zoom

  glViewport( 0,0, resX, resY );
  glDisable( GL_DEPTH_TEST );
  
  matModelView.push();
  matModelView.translate( offX, offY, 0 );
  matModelView.translate( panX, panY, 0 );
  matModelView.scale( zoomS, zoomS, zoomS );
  if (invert) matModelView.scale( 1.0f, -1.0f, 1.0f );

  renderImageClassic( image );

  matModelView.pop();
}

void renderImageRandom1to1 (Image *image, float offX=0.0f, float offY=0.0f, bool invert=false)
{
  //Setup quad buffer

  Vec2 coords[] = {
    Vec2( 0, 0 ),
    Vec2( 1, 0 ),
    Vec2( 1, 1 ),
    Vec2( 0, 1 )
  };

  static bool bufInit = false;
  static VertexBuffer buf;
  if (!bufInit)
  {
    bufInit = true;
    for (int v=0; v<4; ++v)
    {
      Vertex vert;
      vert.coord = coords[v].xy( 0.0f );
      vert.texcoord = coords[v];
      buf.verts.push_back( vert );
    }
    buf.toGpu();
  }

  //Render using current pan and zoom

  glViewport( 0,0, resX, resY );
  glDisable( GL_DEPTH_TEST );

  matModelView.push();
  matModelView.translate( offX, offY, 0 );
  matModelView.translate( panX, panY, 0 );
  matModelView.scale( zoomS, zoomS, zoomS );
  if (invert) matModelView.scale( 1.0f, -1.0f, 1.0f );

  Vec2 sz = image->max - image->min;
  matModelView.translate( image->min.x, image->min.y, 0.0f );
  matModelView.scale( sz.x, sz.y, 1.0f );

  matTexture.push();
  matTexture.identity();
  matTexture.translate( image->min.x, image->min.y, 0.0f );
  matTexture.scale( sz.x, sz.y, 1.0f );

  switch (rep) {
  case Rep::Aux:   renderImageRandom( shaderRenderAux, image, &buf, GL_QUADS );  break;
  case Rep::Pivot: renderImageRandom( shaderRenderPivot, image, &buf, GL_QUADS ); break;
  }
  
  matTexture.pop();
  matModelView.pop();
}

void renderImageRandomCylinder (Image *image, bool invert)
{
  //Setup cylinder buffer

  static bool bufInit = false;
  static VertexBuffer buf;
  if (!bufInit)
  {
    bufInit = true;
    int numSteps = 60;
    float aStep = 2 * PI / numSteps;
    float tStep = 1.0f / numSteps;

    for (int s=0; s<=numSteps; ++s)
    {
      float a = s * aStep;
      float t = s * tStep;
      float cosa = COS( a );
      float sina = SIN( a );
      
      Vertex vertTop;
      Vertex vertBtm;

      vertTop.coord.set( -sina, +1.0f, cosa );
      vertBtm.coord.set( -sina, -1.0f, cosa );

      vertTop.texcoord.set( t, 1.0f );
      vertBtm.texcoord.set( t, 0.0f );
      
      buf.verts.push_back( vertTop );
      buf.verts.push_back( vertBtm );
    }

    buf.toGpu();
  }

  //Render using current rotation and zoom

  glViewport( 0,0, resX, resY );
  glEnable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );

  matProjection.push();
  matModelView.push();
  matTexture.push();

  Matrix4x4 mproj;
  mproj.setPerspectiveFovLH( PI/4, (float)resX/resY, 0.01f, 1000.0f );
  matProjection.load( mproj );

  matModelView.identity();
  matModelView.translate( 0.0f, 0.0f, zoomZ );
  matModelView.rotate( 1.0f, 0.0f, 0.0f, angleY );
  matModelView.rotate( 0.0f, 1.0f, 0.0f, angleX );
  matModelView.scale( 1.0f, 2.0f, 1.0f );

  Vec2 sz = image->max - image->min;
  matTexture.identity();
  matTexture.translate( image->min.x, image->min.y, 0.0f );
  matTexture.scale( sz.x, sz.y, 1.0f );

  if (invert) {
    matTexture.translate( 0.0f, 1.0f, 0.0f );
    matTexture.scale( 1.0f, -1.0, 1.0f );
  }

  switch (rep) {
  case Rep::Aux:   renderImageRandom( shaderRenderAux, image, &buf, GL_TRIANGLE_STRIP );  break;
  case Rep::Pivot: renderImageRandom( shaderRenderPivot, image, &buf, GL_TRIANGLE_STRIP ); break;
  }

  matProjection.pop();
  matModelView.pop();
  matTexture.pop();
}

void display ()
{
  glClearColor( 1,1,1,1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  Image *image;
  float offX=0.0f, offY=0.0f;
  bool invert=false;

  switch (source) {
  case Source::Tiger:
    image = imageTiger;
    offX = 180.0f;
    offY = 400.0f;
    invert = true;
    break;
  
  case Source::Text:
    image = imageText;
    break;
  }
  
  if (view == View::Classic)
  {
    //Render image classic
    if (draw) renderImageClassic1to1( image, offX, offY, invert );
  }
  else
  {
    //Encode image
    static bool first = true;
    static int prevRep = rep;
    static int prevProc = proc;
    static int prevSource = source;
    if (first || encode || rep != prevRep || proc != prevProc || source != prevSource)
    {
      first = false;
      prevRep = rep;
      prevProc = proc;
      prevSource = source;
      switch (proc)
      {
      case Proc::Cpu:

        switch (rep) {
        case Rep::Aux:   image->encodeCpu( imageEncoderAux ); break;
        case Rep::Pivot: image->encodeCpu( imageEncoderPivot ); break;
        } break;

      case Proc::Gpu:

        switch (rep) {
        case Rep::Aux:   image->encodeGpu( imageEncoderGpuAux ); break;
        case Rep::Pivot: image->encodeGpu( imageEncoderGpuPivot ); break;
        } break;
      }
    }

    //Render image random-access
    if (draw)
    {
      switch (view) {
      case View::RandomDirect:   renderImageRandom1to1( image, offX, offY, invert ); break;
      case View::RandomCylinder: renderImageRandomCylinder( image, invert ); break;
      }
    }
  }

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
  checkGlError( "display end" );
  glutSwapBuffers();
}

void reshape (int w, int h)
{
  std::cout << "reshape (" << w << "," << h << ")" << std::endl;

  resX = w;
  resY = h;

  Matrix4x4 m;
  m.setOrthoLH( 0, (Float)w, 0, (Float)h, -1.0f, 1.0f );
  matProjection.clear();
  matProjection.load( m );
  matModelView.clear();

  glViewport( 0,0, resX, resY );
}

void animate ()
{
  glutPostRedisplay();
}

void reportState ()
{
  std::cout << std::endl;

  switch (proc) {
  case Proc::Cpu: std::cout << "(F1) Using CPU encoding" << std::endl; break;
  case Proc::Gpu: std::cout << "(F1) Using GPU encoding" << std::endl; break;
  }

  switch (rep) {
  case Rep::Aux:   std::cout << "(F2) Using AUX representation" << std::endl; break;
  case Rep::Pivot: std::cout << "(F2) Using PIVOT representation" << std::endl; break;
  }

  std::cout << (encode ? "(F3) Encoding ON" : "(F3) Encoding OFF" ) << std::endl;
  std::cout << (draw ? "(F4) Drawing ON" : "(F4) Drawing OFF" ) << std::endl;

  switch (view) {
  case View::Classic:        std::cout << "(F5) View Classic 1:1" << std::endl; break;
  case View::RandomDirect:   std::cout << "(F5) View Random 1:1" << std::endl; break;
  case View::RandomCylinder: std::cout << "(F5) View Random Cylinder" << std::endl; break;
  }

  switch (source) {
  case Source::Tiger: std::cout << "(F6) Image Tiger" << std::endl; break;
  case Source::Text:  std::cout << "(F6) Image Text" << std::endl; break;
  }

  ivec2 screenSize = ivec2( (imageTiger->max - imageTiger->min) * zoomS );
  std::cout << "Image screen size: " << screenSize.x << "x" << screenSize.y << "px" << std::endl;

  std::cout << std::endl;
}

void specialKey (int key, int x, int y)
{
  if (key == GLUT_KEY_F7)
  {
    std::cout << "Compiling..." << std::endl;
    //shader1->load();
    //shaderQuad->load();
    //shaderStream->load();
  }
  else if (key == GLUT_KEY_F1)
  {
    proc = (proc + 1) % Proc::Count;
    reportState();
  }
  else if (key == GLUT_KEY_F2)
  {
    rep = (rep + 1) % Rep::Count;
    reportState();
  }
  else if (key == GLUT_KEY_F3)
  {
    encode = !encode;
    reportState();
  }
  else if (key == GLUT_KEY_F4)
  {
    draw = !draw;
    reportState();
  }/*
  else if (key == GLUT_KEY_F4)
  {
    drawGrid = !drawGrid;
    std::cout << (drawGrid ? "Grid ON" : "Grid OFF" ) << std::endl;
  }*/
  else if (key == GLUT_KEY_F5)
  {
    view = (view + 1) % View::Count;
    reportState();
  }
  else if (key == GLUT_KEY_F6)
  {
    source = (source + 1) % Source::Count;
    reportState();
  }
}

void mouseClick (int button, int state, int x, int y)
{
  mouseButton = button;
  mouseDown.set( (Float)x, (Float)y );
}

void mouseMove (int x, int y)
{
  Vec2 mouse( (Float)x, (Float)y );
  Vec2 mouseDiff = mouse - mouseDown;
  mouseDown = mouse;

  if (view == View::RandomCylinder)
  {
    if (mouseButton == GLUT_LEFT_BUTTON)
    {
      angleX += -mouseDiff.x * 0.5f * (PI / 180);
      angleY += -mouseDiff.y * 0.5f * (PI / 180);
    }
    else if (mouseButton == GLUT_RIGHT_BUTTON)
    {
      zoomZ *= 1.0f + -mouseDiff.y / 100.0f;
    }
  }
  else
  {
    if (mouseButton == GLUT_LEFT_BUTTON)
    {
      panX += mouseDiff.x;
      panY += -mouseDiff.y;
    }
    else if (mouseButton == GLUT_RIGHT_BUTTON)
    {
      zoomS *= 1.0f + mouseDiff.y / 100.0f;
    }
  }
}

void rvgGlutInit (int argc, char **argv)
{
  glutInit( &argc, argv );
  glutInitContextVersion( 3, 0 );
  glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_STENCIL | GLUT_DEPTH );
  //glutInitDisplayString( "rgba alpha double stencil depth samples=32" );

  glutInitWindowPosition( 100,100 );
  glutInitWindowSize( resX,resY );
  int ret = glutCreateWindow( "Random-access Vector Graphics" );

  //glutGameModeString( "1280x1024:32@60" );
  //glutEnterGameMode();
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  //glutKeyboardFunc( keyDown );
  //glutKeyboardUpFunc( keyUp );
  glutSpecialFunc( specialKey );
  glutMouseFunc( mouseClick );
  glutMotionFunc( mouseMove );
  //glutPassiveMotionFunc( mouseMove );
  glutIdleFunc( animate );

  int numSamples = 0;
  glGetIntegerv( GL_SAMPLES, &numSamples );
  std::cout << "Number of FSAA samples: " << numSamples << std::endl;
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

void processCommands (Object *o, int i)
{
  int numCommands = commandCounts[ i ];
  const VGfloat *data = dataArrays[ i ];
  const VGubyte *commands = commandArrays[ i ];
 
  int d = 0;
  for (int c=0; c<numCommands; c++)
  {
    VGubyte cmd = commands[ c ];
    switch (cmd)
    {
    case VG_MOVE_TO_ABS:
      {
      float x = data[ d++ ];
      float y = data[ d++ ];
      o->moveTo( x, y );
      break;
      }
    case VG_LINE_TO_ABS:
      {
      float x0 = data[ d++ ];
      float y0 = data[ d++ ];
      o->lineTo( x0, y0 );
      break;
      }
    case VG_CUBIC_TO_ABS:
      {
      float x0 = data[ d++ ];
      float y0 = data[ d++ ];
      float x1 = data[ d++ ];
      float y1 = data[ d++ ];
      float x2 = data[ d++ ];
      float y2 = data[ d++ ];
      o->cubicTo( x0,y0, x1,y1, x2,y2 );
      break;
      }
    case VG_CLOSE_PATH:
      o->close();
      break;
    }
  }

  //if (commands[ numCommands-1 ] != VG_CLOSE_PATH)
    //o->close();
}

std::string bytesToString (Uint64 bytes)
{
  double f = (double)bytes;
  int u = 0;
  std::string units[] = { "b", "KB", "MB", "GB", "TB" };

  while (f > 1024.0)
  {
    f /= 1024;
    u++;
  }

  std::ostringstream out;
  out.precision( 2 );
  out << std::fixed << f << units[ u ];
  return out.str();
}

void measureData (ImageEncoder *encoder)
{
  Uint32 cpuTotalStreamLen;
  encoder->getTotalStreamInfo( cpuTotalStreamLen );
  
  Uint32 cpuMaxCellLen = 0;
  Uint32 cpuMaxCellObjects = 0;
  Uint32 cpuMaxCellSegments = 0;

  for (int x=0; x < gridResX; ++x) {
    for (int y=0; y < gridResY; ++y) {

      Uint32 cellLen = 0, cellObjects = 0, cellSegments = 0;
      encoder->getCellStreamInfo( x, y, cellLen, cellObjects, cellSegments );

      if (cellLen > cpuMaxCellLen) cpuMaxCellLen = cellLen;
      if (cellObjects > cpuMaxCellObjects) cpuMaxCellObjects = cellObjects;
      if (cellSegments > cpuMaxCellSegments) cpuMaxCellSegments = cellSegments;
    }}

  std::cout << "Total stream bytes: " << bytesToString( cpuTotalStreamLen * 4 ) << std::endl;
  std::cout << "Max cell bytes: " << bytesToString( cpuMaxCellLen * 4 ) << std::endl;
  std::cout << "Max cell words: " << cpuMaxCellLen << std::endl;
  std::cout << "Max cell objects: " << cpuMaxCellObjects << std::endl;
  std::cout << "Max cell segments: " << cpuMaxCellSegments << std::endl;
}

const std::string loremIpsum =
"Lorem ipsum dolor sit amet, consectetur adipisicing elit,\n"
"sed do eiusmod tempor incididunt ut labore et dolore magna\n"
"aliqua. Ut enim ad minim veniam, quis nostrud exercitation\n"
"ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
"Duis aute irure dolor in reprehenderit in voluptate velit\n"
"esse cillum dolore eu fugiat nulla pariatur. Excepteur sint\n"
"occaecat cupidatat non proident, sunt in culpa qui officia\n"
"deserunt mollit anim id est laborum.";

int main (int argc, char **argv)
{
  rvgGlutInit( argc, argv );
  rvgGLInit();
  wglSwapInterval( 0 );

  Shader::Define( "INFO_COUNTER_STREAMLEN", INFO_COUNTER_STREAMLEN );
  Shader::Define( "NUM_INFO_COUNTERS",      NUM_INFO_COUNTERS );

  Shader::Define( "OBJCELL_COUNTER_PREV",   OBJCELL_COUNTER_PREV );
  Shader::Define( "OBJCELL_COUNTER_AUX",    OBJCELL_COUNTER_AUX );
  Shader::Define( "OBJCELL_COUNTER_WIND",   OBJCELL_COUNTER_WIND );
  Shader::Define( "NUM_OBJCELL_COUNTERS",   NUM_OBJCELL_COUNTERS );

  Shader::Define( "CELL_COUNTER_PREV",      CELL_COUNTER_PREV );
  Shader::Define( "NUM_CELL_COUNTERS",      NUM_CELL_COUNTERS );

  Shader::Define( "NODE_TYPE_LINE",         NODE_TYPE_LINE );
  Shader::Define( "NODE_TYPE_QUAD",         NODE_TYPE_QUAD );
  Shader::Define( "NODE_TYPE_OBJECT",       NODE_TYPE_OBJECT );

  Shader::Define( "NODE_SIZE_OBJINFO",      NODE_SIZE_OBJINFO );

  imageEncoderGpuAux = new ImageEncoderGpuAux;
  imageEncoderGpuPivot = new ImageEncoderGpuPivot;
  imageEncoderAux = new ImageEncoderAux;
  imageEncoderPivot = new ImageEncoderPivot;

  shaderRenderAux = new Shader(
    "shaders_aux/render_aux.vert.c",
    "shaders_aux/render_aux.frag.c" );
  shaderRenderAux->load();
  
  shaderRenderPivot = new Shader(
    "shaders_pivot/render_pivot.vert.c",
    "shaders_pivot/render_pivot.frag.c" );
  shaderRenderPivot->load();

  shaderClassicQuads = new Shader(
    "shaders_classic/classic_quads.vert.c",
    "shaders_classic/classic_quads.frag.c" );
  shaderClassicQuads->load();

  shaderClassicContour = new Shader(
    "shaders_classic/classic_contour.vert.c",
    "shaders_classic/classic_contour.frag.c" );
  shaderClassicContour->load();

  shaderClassic = new Shader(
    "shaders_classic/classic.vert.c",
    "shaders_classic/classic.frag.c" );
  shaderClassic->load();

  shaderGrid = new Shader(
    "shaders/grid.vert.c",
    "shaders/grid.frag.c" );
  shaderGrid->load();


  ///////////////////////////////////////////////////////
  // Debug object

  /*
  //Object *object1 = new Object();
  object1->moveTo( 100,100 );
  object1->quadTo( 150,140, 200,100 );
  object1->lineTo( 150,150 );
  object1->quadTo( 130,200, 110,150 );
  object1->close();
  */
  /*
  object1->moveTo( 140,100 );
  //object1->cubicTo( 100,200, 200,200, 200,100 );
  object1->cubicTo( 200,200, 100,200, 160,100 );
  //object1->cubicTo( 200,0, 100,0, 100,100 );
  object1->close();
  */

  //object1->cubicsToQuads();
  //image->objects.push_back( object1 );

  ///////////////////////////////////////////////////////
  // Tiger

  imageTiger = new Image();
  
  GLint param;
  glGetIntegerv( GL_MAX_TEXTURE_SIZE, &param);

  const int VG_FILL_PATH  (1 << 1);
  for (int i=0; i<pathCount; ++i)
  //for (int i=100; i<102; ++i)
  //for (int i=12; i<13; ++i)
  {
    const float *style = styleArrays[i];
    if (! (((int)style[9]) & VG_FILL_PATH)) continue;

    Object *objCubic = new Object();
    processCommands( objCubic, i );

    Object *obj = objCubic->cubicsToQuads();
    delete objCubic;
    
    obj->color.set( style[4], style[5], style[6], style[7] );
    //if (obj->color == Vec4( 1,1,1,1 ))
      //obj->color = Vec4( 0,0,0,1 );

    imageTiger->objects.push_back( obj );
  }

  imageTiger->updateBounds( gridResX, gridResY );
  imageTiger->updateBuffers();
  imageTiger->encodeCpu( imageEncoderPivot );
  measureData( imageEncoderPivot );

  ///////////////////////////////////////////////////////
  // Text

  Font *f = new Font( "Timeless.ttf" );
  //imageText = f->getWord( "Hello\nWorld" );
  //imageText->updateBounds( 50, 10 );
  imageText = f->getWord( loremIpsum+"\n\n"+loremIpsum+"\n\n"+loremIpsum );
  imageText->updateBounds( 200, 200 );
  imageText->updateBuffers();
  //imageText->encodeGpu( imageEncoderGpuPivot );
  
  ///////////////////////////////////////////////////////
  // Main loop
  
  reportState();
  glutMainLoop();
}
