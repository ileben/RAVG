#include "rvgMain.h"
#include "rvgBenchmark.h"

//Matrices

MatrixStack matModelView;
MatrixStack matProjection;
MatrixStack matTexture;

//Encoders and renderers

EncoderCpu *encoderCpuAux;
EncoderCpu *encoderCpuPivot;
EncoderGpu *encoderGpuAux;
EncoderGpu *encoderGpuPivot;

RendererRandom *rendererRandomAux;
RendererRandom *rendererRandomPivot;
RendererClassic *rendererClassic;

Shader *shaderGrid;

//Data

int gridResX = 60;
int gridResY = 60;

Image *imageTiger;
Image *imageText;

//User interface

int resX = 500;
int resY = 600;

int mouseButton = 0;
Vec2 mouseDown;

Float angleX = 0.0f;
Float angleY = 0.0f;
Float zoomZ = 6.0f;

Float panX = 0.0f;
Float panY = 0.0f;
Float zoomS = 1.0f;

//Options

Options options;
OptionsCount optionsCount;

//Results

Results results;

//Tests

std::vector< Test* > tests;

///////////////////////////////////////////////////////////////////
// Forward declaration

void reportState ();

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

  image->renderClassic( rendererClassic );

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

  Vec2 sz = image->getSize();
  matModelView.translate( image->getMin().x, image->getMin().y, 0.0f );
  matModelView.scale( sz.x, sz.y, 1.0f );

  matTexture.push();
  matTexture.identity();
  matTexture.translate( image->getMin().x, image->getMin().y, 0.0f );
  matTexture.scale( sz.x, sz.y, 1.0f );

  switch (options[ Opt::Rep ]) {
  case Rep::Aux:   image->renderRandom( rendererRandomAux, &buf, GL_QUADS );  break;
  case Rep::Pivot: image->renderRandom( rendererRandomPivot, &buf, GL_QUADS ); break;
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

  Vec2 sz = image->getSize();
  matTexture.identity();
  matTexture.translate( image->getMin().x, image->getMin().y, 0.0f );
  matTexture.scale( sz.x, sz.y, 1.0f );

  if (invert) {
    matTexture.translate( 0.0f, 1.0f, 0.0f );
    matTexture.scale( 1.0f, -1.0, 1.0f );
  }

  switch (options[ Opt::Rep ]) {
  case Rep::Aux:   image->renderRandom( rendererRandomAux, &buf, GL_TRIANGLE_STRIP );  break;
  case Rep::Pivot: image->renderRandom( rendererRandomPivot, &buf, GL_TRIANGLE_STRIP ); break;
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

  switch (options[ Opt::Source ]) {
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
  
  if (options[ Opt::Render ] == Render::Classic)
  {
    //Render image classic
    if (options[ Opt::Draw ])
      renderImageClassic1to1( image, offX, offY, invert );
  }
  else if (options[ Opt::Render ] == Render::Random)
  {
    //Encode image
    static bool first = true;
    static Options prevOptions = options;
    if (first || (prevOptions != options) || options[ Opt::Encode ])
    {
      prevOptions = options;
      first = false;

      switch (options[ Opt::Proc ]) {
      case Proc::Cpu:

        switch (options[ Opt::Rep ]) {
        case Rep::Aux:   image->encodeCpu( encoderCpuAux ); break;
        case Rep::Pivot: image->encodeCpu( encoderCpuPivot ); break;
        } break;

      case Proc::Gpu:

        switch (options[ Opt::Rep ]) {
        case Rep::Aux:   image->encodeGpu( encoderGpuAux ); break;
        case Rep::Pivot: image->encodeGpu( encoderGpuPivot ); break;
        } break;
      }
    }

    //Render image random-access
    if (options[ Opt::Draw ]) {
      switch (options[ Opt::View ]) {
      case View::Flat:     renderImageRandom1to1( image, offX, offY, invert ); break;
      case View::Cylinder: renderImageRandomCylinder( image, invert ); break;
      }
    }
  }

  //Check fps
  static int fps = 0;
  static int lastUpdate = glutGet( GLUT_ELAPSED_TIME );
  fps++;

  int now = glutGet( GLUT_ELAPSED_TIME );
  if (now - lastUpdate > 1000) {
    results[ Res::Fps ] = fps;
    std::cout << "Fps: " << fps << std::endl;
    lastUpdate = now;
    fps = 0;
    /*
    //Submit results
    tests[0]->results( results );
    if (tests[0]->done()) {
      std::cout << "DONE" << std::endl;
      tests[0]->reset();
    }
    options = tests[0]->next();
    reportState(); */
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

  std::cout << (options[ Opt::Draw ] ? "(F1) Drawing ON" : "(F1) Drawing OFF" ) << std::endl;
  std::cout << (options[ Opt::Encode ] ? "(F2) Encoding ON" : "(F2) Encoding OFF" ) << std::endl;

  switch (options[ Opt::Proc ]) {
  case Proc::Cpu: std::cout << "(F3) Encoding with CPU" << std::endl; break;
  case Proc::Gpu: std::cout << "(F3) Encoding with GPU" << std::endl; break;
  }

  switch (options[ Opt::Rep ]) {
  case Rep::Aux:   std::cout << "(F4) Representation AUX" << std::endl; break;
  case Rep::Pivot: std::cout << "(F4) Representation PIVOT" << std::endl; break;
  }

  switch (options[ Opt::Render ]) {
  case Render::Classic:  std::cout << "(F5) Renderer Classic" << std::endl; break;
  case Render::Random:   std::cout << "(F5) Renderer Random-Access" << std::endl; break;
  }

  switch (options[ Opt::View ]) {
  case View::Flat:     std::cout << "(F6) View Flat" << std::endl; break;
  case View::Cylinder: std::cout << "(F6) View Cylinder" << std::endl; break;
  }

  switch (options[ Opt::Source ]) {
  case Source::Tiger: std::cout << "(F7) Image Tiger" << std::endl; break;
  case Source::Text:  std::cout << "(F7) Image Text" << std::endl; break;
  }

  ivec2 screenSize = ivec2( imageTiger->getSize() * zoomS );
  std::cout << "Image screen size: " << screenSize.x << "x" << screenSize.y << "px" << std::endl;

  std::cout << std::endl;
}

void specialKey (int key, int x, int y)
{
  /*
  if (key == GLUT_KEY_F7)
  {
    std::cout << "Compiling..." << std::endl;
    //shader1->load();
    //shaderQuad->load();
    //shaderStream->load();
  }*/

  if (key >= GLUT_KEY_F1 && key < GLUT_KEY_F1 + Opt::Count)
  {
    int opt = key - GLUT_KEY_F1;
    options[ opt ] = (options[ opt ] + 1) % optionsCount[ opt ];
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

  if (options[ Opt::View ] == View::Cylinder)
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
  else if (options[ Opt::View ] == View::Flat)
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

void measureData (EncoderCpu *encoder)
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

  encoderGpuAux = new EncoderGpuAux;
  encoderGpuPivot = new EncoderGpuPivot;
  encoderCpuAux = new EncoderCpuAux;
  encoderCpuPivot = new EncoderCpuPivot;

  rendererRandomAux = new RendererRandomAux;
  rendererRandomPivot = new RendererRandomPivot;
  rendererClassic = new RendererClassic;

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
    
    obj->setColor( style[4], style[5], style[6], style[7] );
    //if (obj->color == Vec4( 1,1,1,1 ))
      //obj->color = Vec4( 0,0,0,1 );

    imageTiger->addObject( obj );
  }

  imageTiger->setGridResolution( gridResX, gridResY );
  imageTiger->encodeCpu( encoderCpuPivot );
  measureData( encoderCpuPivot );

  ///////////////////////////////////////////////////////
  // Text

  Font *f = new Font( "Timeless.ttf" );
  //imageText = f->getWord( "Hello\nWorld" );
  //imageText->setGridResolution( 50, 10 );
  imageText = f->getWord( loremIpsum+"\n\n"+loremIpsum+"\n\n"+loremIpsum );
  imageText->setGridResolution( 200, 200 );

  ///////////////////////////////////////////////////////
  // Tests

  Options optEncodeTiger = 
    OptVal( Opt::Draw,   Draw::False ) +
    OptVal( Opt::Encode, Encode::True ) +
    OptVal( Opt::Source, Source::Tiger ) +
    OptVal( Opt::Render, Render::Random );

  Options optCpuAuxE = optEncodeTiger +
    OptVal( Opt::Proc, Proc::Cpu ) +
    OptVal( Opt::Rep,  Rep::Aux );

  Options optCpuPivotE = optEncodeTiger +
    OptVal( Opt::Proc, Proc::Cpu ) +
    OptVal( Opt::Rep,  Rep::Pivot );

  Options optGpuAuxE = optEncodeTiger +
    OptVal( Opt::Proc, Proc::Gpu) +
    OptVal( Opt::Rep,  Rep::Aux );

  Options optGpuPivotE = optEncodeTiger +
    OptVal( Opt::Proc, Proc::Gpu ) +
    OptVal( Opt::Rep,  Rep::Pivot );

  Test *testETime = new Test( Opt::Grid, Res::Fps, 4, 1 );
  //testETime->addArguments( 20, 40, 60, 80, 100, -1 );
  testETime->addArguments( 20, 40, -1 );
  testETime->addEnvironment( "Cpu Auxiliary", optCpuAuxE );
  testETime->addEnvironment( "Cpu Pivot",     optCpuPivotE );
  testETime->addEnvironment( "Gpu Auxiliary", optGpuAuxE );
  testETime->addEnvironment( "Gpu Pivot",     optGpuPivotE );
  //tests.push_back( testETime );

  Options optRenderFlatTiger =
    OptVal( Opt::Draw,   Draw::True ) +
    OptVal( Opt::Encode, Encode::False ) +
    OptVal( Opt::View,   View::Flat ) +
    OptVal( Opt::Source, Source::Tiger );

  Options optAuxR = optRenderFlatTiger +
    OptVal( Opt::Render, Render::Random ) +
    OptVal( Opt::Rep,    Rep::Aux ) +
    OptVal( Opt::Proc,   Proc::Gpu );

  Options optPivotR = optRenderFlatTiger +
    OptVal( Opt::Render, Render::Random ) +
    OptVal( Opt::Rep,    Rep::Pivot ) +
    OptVal( Opt::Proc,   Proc::Gpu );

  Options optClassicR = optRenderFlatTiger +
    OptVal( Opt::Render, Render::Classic );

  Test *testRTime = new Test( Opt::Grid, Res::Fps, 4, 1 );
  //testRTime->addArguments( 20, 40, 60, 80, 100, -1 );
  testRTime->addArguments( 20, 40, -1 );
  testRTime->addEnvironment( "Auxiliary", optAuxR );
  testRTime->addEnvironment( "Pivot",     optPivotR );
  testRTime->addEnvironment( "Classic",   optClassicR );
  tests.push_back( testRTime );

  
  ///////////////////////////////////////////////////////
  // Main loop
  
  reportState();
  glutMainLoop();
}
