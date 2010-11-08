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
RendererRandom *rendererRandomPivotLight;
RendererClassic *rendererClassic;

Shader *shaderGrid;

//Data

Image *imageTiger;
Image *imageText;
Image *imageTextPart;
Image *imageWorld;

//User interface

int resX = 550;
int resY = 600;

int mouseButton = 0;
Vec2 mouseDown;

Float angleX = 0.0f;
Float angleY = 0.0f;
Float zoomZ = 6.0f;

//Options

Options options;
OptionsCount optionsCount;

//Results

Results results;

//Tests

TestGroup tests( "output.txt" );

//Lorem Ipsum

const std::string loremIpsum =
"Lorem ipsum dolor sit amet, consectetur adipisicing elit,\n"
"sed do eiusmod tempor incididunt ut labore et dolore magna\n"
"aliqua. Ut enim ad minim veniam, quis nostrud exercitation\n"
"ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
"Duis aute irure dolor in reprehenderit in voluptate velit\n"
"esse cillum dolore eu fugiat nulla pariatur. Excepteur sint\n"
"occaecat cupidatat non proident, sunt in culpa qui officia\n"
"deserunt mollit anim id est laborum.";

const std::string loremIpsum3 =
  loremIpsum+"\n\n"+loremIpsum+"\n\n"+loremIpsum;



///////////////////////////////////////////////////////////////////
// Forward declaration

void reportState ();

///////////////////////////////////////////////////////////////////
// Functions

/*
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
*/

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

void renderImageClassicFlat (Image *image, bool invert=false)
{
  glViewport( 0,0, resX, resY );
  glDisable( GL_DEPTH_TEST );

  //Get zoom and pan values from settings
  float zoomS = options[ Opt::ZoomS ].toFloat();
  float panX = options[ Opt::PanX ].toFloat();
  float panY = options[ Opt::PanY ].toFloat();
  
  //Translate image according to settings
  matModelView.push();
  matModelView.translate( panX, panY, 0 );

  //Center image to screen
  matModelView.translate( resX * 0.5f, resY * 0.5f, 0 );

  //Scale image according to settings
  matModelView.scale( zoomS, zoomS, zoomS );
  if (invert) matModelView.scale( 1.0f, -1.0f, 1.0f );

  //Center image to origin
  Vec2 sz = image->getSize();
  matModelView.translate( -sz.x * 0.5f, -sz.y * 0.5f, 0.0f );
  matModelView.translate( -image->getMin().x, -image->getMin().y, 0.0f );

  image->renderClassic( rendererClassic );

  matModelView.pop();
}

void renderImageRandomFlat (Image *image, bool invert=false)
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

  glViewport( 0,0, resX, resY );
  glDisable( GL_DEPTH_TEST );

  //Get zoom and pan values from settings
  float zoomS = options[ Opt::ZoomS ].toFloat();
  float panX = options[ Opt::PanX ].toFloat();
  float panY = options[ Opt::PanY ].toFloat();

  //Translate image according to settings
  matModelView.push();
  matModelView.translate( panX, panY, 0 );

  //Center image to screen
  matModelView.translate( resX * 0.5f, resY * 0.5f, 0 );

  //Scale image according to settings
  matModelView.scale( zoomS, zoomS, zoomS );
  if (invert) matModelView.scale( 1.0f, -1.0f, 1.0f );

  //Scale [0,1] quad to match image and center to origin
  Vec2 sz = image->getSize();
  matModelView.translate( -sz.x * 0.5f, -sz.y * 0.5f, 0.0f );
  matModelView.scale( sz.x, sz.y, 1.0f );

  //Transform [0,1] texture coordinates into image position
  matTexture.push();
  matTexture.identity();
  matTexture.translate( image->getMin().x, image->getMin().y, 0.0f );
  matTexture.scale( sz.x, sz.y, 1.0f );

  switch (options[ Opt::Rep ].toInt()) {
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
      float x = -sina;
      float z = cosa;
      
      Vertex vertTop;
      Vertex vertBtm;

      vertTop.coord.set( x, +1.0f, z );
      vertBtm.coord.set( x, -1.0f, z );

      vertTop.texcoord.set( t, 1.0f );
      vertBtm.texcoord.set( t, 0.0f );

      vertTop.normal.set( x, 0.0f, z );
      vertBtm.normal.set( x, 0.0f, z );

      buf.verts.push_back( vertTop );
      buf.verts.push_back( vertBtm );
    }

    buf.toGpu();
  }

  glViewport( 0,0, resX, resY );
  glEnable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );

  matProjection.push();
  matModelView.push();
  matTexture.push();

  //Setup perspective projection
  Matrix4x4 mproj;
  mproj.setPerspectiveFovLH( PI/4, (float)resX/resY, 0.01f, 1000.0f );
  matProjection.load( mproj );

  //Rotate model according to view settings
  matModelView.identity();
  matModelView.translate( 0.0f, 0.0f, zoomZ );
  matModelView.rotate( 1.0f, 0.0f, 0.0f, angleY );
  matModelView.rotate( 0.0f, 1.0f, 0.0f, angleX );

  //Scale cylinder vertically
  matModelView.scale( 1.0f, 2.0f, 1.0f );

  //Transform [0,1] texture coordinates into image position
  Vec2 sz = image->getSize();
  matTexture.identity();
  matTexture.translate( image->getMin().x, image->getMin().y, 0.0f );
  matTexture.scale( sz.x, sz.y, 1.0f );

  //Invert coordinates vertically if needed
  if (invert) {
    matTexture.translate( 0.0f, 1.0f, 0.0f );
    matTexture.scale( 1.0f, -1.0, 1.0f );
  }

  switch (options[ Opt::Rep ].toInt()) {
  case Rep::Aux:   image->renderRandom( rendererRandomAux, &buf, GL_TRIANGLE_STRIP );  break;
  //case Rep::Pivot: image->renderRandom( rendererRandomPivot, &buf, GL_TRIANGLE_STRIP ); break;
  case Rep::Pivot: image->renderRandom( rendererRandomPivotLight, &buf, GL_TRIANGLE_STRIP ); break;
  }

  matProjection.pop();
  matModelView.pop();
  matTexture.pop();
}

void renderImageRandomFold (Image *image, bool invert=false)
{
  //Setup paper fold buffer

  static bool bufInit = false;
  static VertexBuffer buf;
  if (!bufInit)
  {
    bufInit = true;

    //Coordinates
    const int numCoords = 7;
    float xCoords[ numCoords ] = { -1.0f, -0.8f, -0.6f, -0.2f, 0.0f, 0.5f, 1.0f };
    //float yCoords[ numCoords ] = { -1, 1, -1, 1, -1, 1, -1 };
    float yCoords[ numCoords ] = { -1, 2, -1, 1.4f, -1, 0.8f, -1 };
    float zCoords[ 2 ] = { -1.0f, 1.0f };
    float yScale = 0.2f;
    //float yScale = 1.0f;

    //Calculate unfolded width of the paper sheet
    float totalWidth = 0.0f;
    for (int c=0; c<numCoords-1; ++c)
    {
      Vec3 p1( xCoords[c+0], yCoords[c+0] * yScale, 0.0f );
      Vec3 p2( xCoords[c+1], yCoords[c+1] * yScale, 0.0f );
      totalWidth += (p2 - p1).norm();
    }

    //Generate a quad for every fold
    float curWidth = 0.0f, nextWidth = 0.0f;
    for (int c=0; c<numCoords-1; ++c, curWidth = nextWidth)
    {
      //Four vertices of the quad
      Vec3 coords[4] = {
        Vec3( xCoords[c+0], yCoords[c+0] * yScale, zCoords[0] ),
        Vec3( xCoords[c+1], yCoords[c+1] * yScale, zCoords[0] ),
        Vec3( xCoords[c+1], yCoords[c+1] * yScale, zCoords[1] ),
        Vec3( xCoords[c+0], yCoords[c+0] * yScale, zCoords[1] )
      };

      //Single normal for the entire quad
      Vec3 side1 = coords[1] - coords[0];
      Vec3 side2 = coords[3] - coords[0];
      Vec3 normal = Vec::Cross( side1, side2 );
      normal.normalize();

      //Texture coordinates as a fraction of unfolded width
      nextWidth = curWidth + side1.norm();
      Vec2 texcoords[4] = {
        Vec2( curWidth  / totalWidth, 0.0f ),
        Vec2( nextWidth / totalWidth, 0.0f ),
        Vec2( nextWidth / totalWidth, 1.0f ),
        Vec2( curWidth  / totalWidth, 1.0f )
      };

      //Add vertices to buffer
      for (int v=0; v<4; ++v)
      {
        Vertex vert;
        vert.coord = coords[v];
        vert.texcoord = texcoords[v];
        vert.normal = normal;
        buf.verts.push_back( vert );
      }
    }

    buf.toGpu();
  }

  glViewport( 0,0, resX, resY );
  glEnable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );

  matProjection.push();
  matModelView.push();
  matTexture.push();

  //Setup perspective projection
  Matrix4x4 mproj;
  mproj.setPerspectiveFovLH( PI/4, (float)resX/resY, 0.01f, 1000.0f );
  matProjection.load( mproj );

  //Rotate model according to view settings
  matModelView.identity();
  matModelView.translate( 0.0f, 0.0f, zoomZ );
  matModelView.rotate( 1.0f, 0.0f, 0.0f, angleY );
  matModelView.rotate( 0.0f, 1.0f, 0.0f, angleX );

  //Transform [0,1] texture coordinates into image position
  Vec2 sz = image->getSize();
  matTexture.identity();
  matTexture.translate( image->getMin().x, image->getMin().y, 0.0f );
  matTexture.scale( sz.x, sz.y, 1.0f );

  //Invert coordinates vertically if needed
  if (invert) {
    matTexture.translate( 0.0f, 1.0f, 0.0f );
    matTexture.scale( 1.0f, -1.0, 1.0f );
  }

  switch (options[ Opt::Rep ].toInt()) {
  case Rep::Aux:   image->renderRandom( rendererRandomAux, &buf, GL_QUADS );  break;
  //case Rep::Pivot: image->renderRandom( rendererRandomPivot, &buf, GL_QUADS ); break;
  case Rep::Pivot: image->renderRandom( rendererRandomPivotLight, &buf, GL_QUADS ); break;
  }

  matProjection.pop();
  matModelView.pop();
  matTexture.pop();
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
  Uint32 totalStreamLen;
  encoder->getTotalStreamInfo( totalStreamLen );
  
  Uint32 maxCellLen = 0;
  Uint32 maxCellObjects = 0;
  Uint32 maxCellSegments = 0;

  for (int x=0; x < encoder->gridSize.x; ++x) {
    for (int y=0; y < encoder->gridSize.y; ++y) {

      Uint32 cellLen = 0, cellObjects = 0, cellSegments = 0;
      encoder->getCellStreamInfo( x, y, cellLen, cellObjects, cellSegments );

      if (cellLen > maxCellLen) maxCellLen = cellLen;
      if (cellObjects > maxCellObjects) maxCellObjects = cellObjects;
      if (cellSegments > maxCellSegments) maxCellSegments = cellSegments;
    }}

  results[ Res::StreamMegaBytes ]  = (float) (totalStreamLen * 4) / (1024 * 1024);
  results[ Res::CellBytes ]    = (int) maxCellLen * 4;
  results[ Res::CellWords ]    = (int) maxCellLen;
  results[ Res::CellObjects ]  = (int) maxCellObjects;
  results[ Res::CellSegments ] = (int) maxCellSegments;
}

void adjustTigerImage ()
{
  //Adjust grid resolution to match settings
  int g = options[ Opt::GridSize ].toInt();
  imageTiger->setGridResolution( g, g );
}

void adjustWorldImage()
{
  //Adjust grid resolution to match settings
  int g = options[ Opt::GridSize ].toInt();
  imageWorld->setGridResolution( g, g );
}

void adjustTextImage ()
{
  //Get limit on number of glyphs from the settings
  int numGlyphs = options[ Opt::NumGlyphs ].toInt();

  //Clear the partial text image
  imageTextPart->removeAllObjects();

  //Iterate glyphs in the original image until limit met
  int g=0, w=0, numLines = 1, maxWidth = 1;
  for (Uint c=0; g<numGlyphs && c<loremIpsum3.length(); ++c)
  {
    //Check for line break and increase line count
    const char& chr = loremIpsum3[ c ];
    if (chr == '\n') {
      numLines++;
      w = 0;
      continue;
    }

    //Copy glyph from original to partial image
    imageTextPart->addObject( imageText->getObject(g) );
    g++; w++;

    //Update longest line
    if (w > maxWidth)
      maxWidth = w;
  }

  //Adjust grid resolution to keep a constant number of cells per glyph
  //imageTextPart->setGridResolution( maxWidth * 4, numLines * 4 );
  imageTextPart->setGridResolution( maxWidth * 2, numLines * 2 );
  //imageTextPart->setGridResolution( maxWidth, numLines );
}

void display ()
{
  glClearColor( 1,1,1,1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  Image *image;
  bool invert=false;

  //Pick image to match settings
  switch (options[ Opt::Source ].toInt()) {
  case Source::Tiger:{
    image = imageTiger;
    adjustTigerImage();
    invert = true;
    break;}
  
  case Source::Text:
    image = imageTextPart;
    adjustTextImage();
    break;

  case Source::World:
    image = imageWorld;
    adjustWorldImage();
    invert = true;
    break;
  }

  //Calculate image screen size
  float zoomS = options[ Opt::ZoomS ].toFloat();
  ivec2 screenSize = ivec2( image->getSize() * zoomS );
  results[ Res::ScreenWidth ] = screenSize.x;
  results[ Res::ScreenHeight ] = screenSize.y;


  //Check selected renderer type
  if (options[ Opt::Render ] == Render::Classic)
  {
    //Update image
    static bool first = true;
    static Options prevOptions = options;
    if (first || (prevOptions != options))
    {
      image->update();
      prevOptions = options;
      first = false;
    }

    //Render image classic
    if (options[ Opt::Draw ].toInt())
      renderImageClassicFlat( image, invert );
  }
  else if (options[ Opt::Render ] == Render::Random)
  {
    //Encode image
    static bool first = true;
    static Options prevOptions = options;
    if (first || (prevOptions != options) || options[ Opt::Encode ].toInt())
    {
      prevOptions = options;
      first = false;

      switch (options[ Opt::Proc ].toInt()) {
      case Proc::Cpu:

        switch (options[ Opt::Rep ].toInt()) {
        case Rep::Aux:   image->encodeCpu( encoderCpuAux ); break;
        case Rep::Pivot: image->encodeCpu( encoderCpuPivot ); break;
        }

        //Measure size of image data
        if (options[ Opt::Measure ].toInt()) {
          switch (options[ Opt::Rep ].toInt()) {
          case Rep::Aux:   measureData( encoderCpuAux ); break;
          case Rep::Pivot: measureData( encoderCpuPivot ); break;
          }
        }

        break;
      case Proc::Gpu:

        switch (options[ Opt::Rep ].toInt()) {
        case Rep::Aux:   image->encodeGpu( encoderGpuAux ); break;
        case Rep::Pivot: image->encodeGpu( encoderGpuPivot ); break;
        } break;
      }
    }

    //Render image random-access
    if (options[ Opt::Draw ].toInt()) {
      switch (options[ Opt::View ].toInt()) {
      case View::Flat:     renderImageRandomFlat( image, invert ); break;
      case View::Cylinder: renderImageRandomCylinder( image, invert ); break;
      case View::PaperFold: renderImageRandomFold( image, invert ); break;
      }
    }
  }

  //Add to fps
  static int fps = 0;
  fps++;

  //Check if 1 second elapsed
  static int lastUpdate = glutGet( GLUT_ELAPSED_TIME );
  int now = glutGet( GLUT_ELAPSED_TIME );
  if (now - lastUpdate > 1000)
  {
    //Write out FPS result
    results[ Res::Fps ] = fps;
    std::cout << "Fps: " << fps << std::endl;
    
    //Submit results
    if (tests.running())
    {
      tests.results( results );
      if (tests.done()) {
        std::cout << "~~~~~~~~~ TESTS DONE ~~~~~~~~~" << std::endl;
        tests.reset();
      }else{
        options = tests.next();
        reportState();
      }
    }

    //Reset fps
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

  std::cout << (options[ Opt::Draw ].toInt() ? "(F1) Drawing ON" : "(F1) Drawing OFF" ) << std::endl;
  std::cout << (options[ Opt::Encode ].toInt() ? "(F2) Encoding ON" : "(F2) Encoding OFF" ) << std::endl;

  switch (options[ Opt::Proc ].toInt()) {
  case Proc::Cpu: std::cout << "(F3) Encoding with CPU" << std::endl; break;
  case Proc::Gpu: std::cout << "(F3) Encoding with GPU" << std::endl; break;
  }

  switch (options[ Opt::Rep ].toInt()) {
  case Rep::Aux:   std::cout << "(F4) Representation AUX" << std::endl; break;
  case Rep::Pivot: std::cout << "(F4) Representation PIVOT" << std::endl; break;
  }

  switch (options[ Opt::Render ].toInt()) {
  case Render::Classic:  std::cout << "(F5) Renderer Classic" << std::endl; break;
  case Render::Random:   std::cout << "(F5) Renderer Random-Access" << std::endl; break;
  }

  switch (options[ Opt::View ].toInt()) {
  case View::Flat:     std::cout << "(F6) View Flat" << std::endl; break;
  case View::Cylinder: std::cout << "(F6) View Cylinder" << std::endl; break;
  }

  switch (options[ Opt::Source ].toInt()) {
  case Source::Tiger: std::cout << "(F7) Image Tiger" << std::endl; break;
  case Source::Text:  std::cout << "(F7) Image Text" << std::endl; break;
  }

  std::cout << (options[ Opt::Measure ].toInt() ? "(F8) Measure ON" : "(F8) Measure OFF" ) << std::endl;

  float zoomS = options[ Opt::ZoomS ].toFloat();
  std::cout << "Zoom: " << zoomS << std::endl;

  int gridSize = options[ Opt::GridSize ].toInt();
  std::cout << "Grid: " << gridSize << "x" << gridSize << std::endl;

  int numGlyphs = options[ Opt::NumGlyphs ].toInt();
  std::cout << "Glyphs: " << numGlyphs << std::endl;

  ivec2 screenSize = ivec2( imageTiger->getSize() * zoomS );
  std::cout << "Image screen size: " << screenSize.x << "x" << screenSize.y << "px" << std::endl;

  std::cout << std::endl;
}

void keyDown (unsigned char key, int x, int y)
{
  if (key == '+')
  {
    int gridSize = options[ Opt::GridSize ].toInt();
    gridSize += 10;
    options[ Opt::GridSize ] = gridSize;
  }
  else if (key == '-')
  {
    int gridSize = options[ Opt::GridSize ].toInt();
    if (gridSize >= 10) gridSize -= 10;
    options[ Opt::GridSize ] = gridSize;
  }
  else if (key == '*')
  {
    int numGlyphs = options[ Opt::NumGlyphs ].toInt();
    numGlyphs += 10;
    options[ Opt::NumGlyphs ] = numGlyphs;
  }
  else if (key == '/')
  {
    int numGlyphs = options[ Opt::NumGlyphs ].toInt();
    if (numGlyphs > 10) numGlyphs -= 10;
    options[ Opt::NumGlyphs ] = numGlyphs;
  }

  reportState();
}

void specialKeyDown (int key, int x, int y)
{
  /*
  if (key == GLUT_KEY_F7)
  {
    std::cout << "Compiling..." << std::endl;
    //shader1->load();
    //shaderQuad->load();
    //shaderStream->load();
  }*/

  //if (key >= GLUT_KEY_F1 && key < GLUT_KEY_F1 + Opt::Count)
  if (key >= GLUT_KEY_F1 && key <= GLUT_KEY_F8)
  {
    int opt = key - GLUT_KEY_F1;
    options[ opt ] = (options[ opt ].toInt() + 1) % optionsCount[ opt ];
    reportState();
  }

  if (key == GLUT_KEY_F12)
  {
    //Stop tests if running
    if (tests.running())
      tests.reset();
    else
    {
      //Begin tests if not
      options = tests.next();
      reportState();
    }
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

  if (options[ Opt::View ] == View::Cylinder ||
      options[ Opt::View ] == View::PaperFold )
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
      float panX = options[ Opt::PanX ].toFloat();
      float panY = options[ Opt::PanY ].toFloat();
      panX += mouseDiff.x;
      panY += -mouseDiff.y;
      options[ Opt::PanX ] = panX;
      options[ Opt::PanY ] = panY;
    }
    else if (mouseButton == GLUT_RIGHT_BUTTON)
    {
      float zoomS = options[ Opt::ZoomS ].toFloat();
      zoomS *= 1.0f + mouseDiff.y / 100.0f;
      options[ Opt::ZoomS ] = zoomS;
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
  int ret = glutCreateWindow( "Random Access Vector Graphics" );

  //glutGameModeString( "1280x1024:32@60" );
  //glutEnterGameMode();
  
  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyDown );
  //glutKeyboardUpFunc( keyUp );
  glutSpecialFunc( specialKeyDown );
  glutMouseFunc( mouseClick );
  glutMotionFunc( mouseMove );
  //glutPassiveMotionFunc( mouseMove );
  glutIdleFunc( animate );

  int numSamples = 0;
  glGetIntegerv( GL_SAMPLES, &numSamples );
  std::cout << "Number of FSAA samples: " << numSamples << std::endl;
}

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
  rendererRandomPivotLight = new RendererRandomPivotLight;
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

  imageTiger = loadSvg( "../svg/tiger.svg" );

  int g = options[ Opt::GridSize ].toInt();
  imageTiger->setGridResolution( g, g );
  imageTiger->encodeCpu( encoderCpuPivot );
  measureData( encoderCpuPivot );

  int streamBytes = (int) results[ Res::StreamMegaBytes ].toFloat() * 1024 * 1024;
  std::cout << "Total stream bytes: " << bytesToString( streamBytes ) << std::endl;
  std::cout << "Max cell bytes: " << bytesToString( results[ Res::CellBytes ].toInt() ) << std::endl;
  std::cout << "Max cell words: " << results[ Res::CellWords ] << std::endl;
  std::cout << "Max cell objects: " << results[ Res::CellObjects ] << std::endl;
  std::cout << "Max cell segments: " << results[ Res::CellSegments ] << std::endl;

  ///////////////////////////////////////////////////////
  // Text

  Font *f = new Font( "Timeless.ttf", 20 );
  //imageText = f->getWord( "Hello\nWorld" );
  //imageText->setGridResolution( 50, 10 );
  imageText = f->getWord( loremIpsum3 );
  imageText->setGridResolution( 200, 200 );

  imageTextPart = new Image();
  imageTextPart->setGridResolution( 200, 200 );

  ///////////////////////////////////////////////////////
  // World

  imageWorld = loadSvg( "../svg/worldmap_with_plane.svg" );
  //imageWorld = loadSvg( "../svg/worldmap.svg" );
  //imageWorld = loadSvg( "../svg/plane_only.svg" );
  imageWorld->setGridResolution( 200, 200 );

  ///////////////////////////////////////////////////////
  // Tests

  initTests( tests );
  
  ///////////////////////////////////////////////////////
  // Main loop
  
  reportState();
  glutMainLoop();
}
