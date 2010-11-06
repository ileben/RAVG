#include "rvgMain.h"
#include "rvgBenchmark.h"

Test::Test (const std::string &name, Opt::Enum domain, Res::Enum codomain, int repeatCount, int dropCount)
  : name(name), domain(domain), codomain(codomain), repeatCount(repeatCount), dropCount(dropCount)
{
  if (repeatCount <= 0)
    repeatCount = 1;

  if (dropCount >= repeatCount)
    dropCount = repeatCount-1;

  reset();
}

Test::~Test()
{
  for (Uint e=0; e<environments.size(); ++e)
    delete environments[e];
}

void Test::reset()
{
  repIndex = -1;
  envIndex = -1;
  argIndex = -1;
}

Options Test::next()
{
  if (running())
  {
    //Move to next
    if (++repIndex >= repeatCount) {
      repIndex = 0;
      if (++argIndex >= (int)arguments.size()) {
        argIndex = 0;
        if (++envIndex >= (int)environments.size()) {
          envIndex = 0;
        }}}
  }
  else
  {
    //Move to first
    repIndex = 0;
    argIndex = 0;
    envIndex = 0;
  }

  std::cout
    << "Moving on to env:" <<  environments[ envIndex ]->name
    << " arg:" << arguments[ argIndex ]
    << " rep:" << repIndex
    << std::endl;

  //Get options for the current environment and adjust argument
  Options opt = environments[ envIndex ]->options;
  opt[ domain ] = arguments[ argIndex ];
  return opt;
}

void Test::results (Results &res)
{
  if (running())
  {
    std::cout << "Storing results for rep:" << repIndex << std::endl;

    //Store value into list and average if last
    values[ repIndex ] = res[ codomain ];
    if (repIndex == repeatCount-1)
      averageValues();
  }
}

bool Test::running()
{
  //Check if next() has been called since reset
  return (envIndex > -1 &&
          argIndex > -1 &&
          repIndex > -1);
}

bool Test::done()
{
  //Check if last repetition of last argument in last environment reached
  return (envIndex >= (int) environments.size()-1 &&
          argIndex >= (int) arguments.size()-1 &&
          repIndex >= (int) repeatCount-1);
}

void Test::averageValues ()
{
  IntFloat avgValue = 0;
  int numValues = 0;

  //Find average of all values after dropping first few
  for (Uint32 v=dropCount; v<values.size(); ++v) {
    std::cout << "Average part: " << values[v] << std::endl;
    avgValue += values[v];
    numValues += 1;
  }

  //Store as value corresponding to current argument
  if (numValues > 0)
    environments[ envIndex ]->values[ argIndex ] = avgValue / numValues;
  else environments[ envIndex ]->values[ argIndex ] = 0;

  std::cout << "Average: " << environments[ envIndex ]->values[ argIndex ] << std::endl;
}

void Test::resizeValues ()
{
  //Make sure the number of temporary values matches the number of repetitions
  values.resize( repeatCount, 0 );

  //Make sure the number of environment values matches the number of arguments
  for (Uint32 e=0; e<environments.size(); ++e)
    environments[e]->values.resize( arguments.size(), 0 );
}


////////////////////////////////////////////////////////////////////////////////


TestGroup::TestGroup (const std::string &file)
{
  this->file = file;
  reset();
}

void TestGroup::addTest (Test *test)
{
  tests.push_back( test );
}

void TestGroup::reset()
{
  index = -1;
  //fileClear();
}

Options TestGroup::next()
{
  if (running())
  {
    //Check if test done
    if (tests[ index ]->done())
    {
      //Move to next test
      if (++index >= (int)tests.size()) index = 0;
      tests[ index ]->reset();
    }
  }
  else
  {
    //Move to first test
    index = 0;
    tests[ index ]->reset();
  }

  //Move to next stage of the current test
  return tests[ index ]->next();
}

void TestGroup::results (Results &res)
{
  if (running())
  {
    //Store results for current stage of current test
    tests[ index ]->results( res );
    if (tests[ index ]->done())
    {
      //Output test results to file
      if (index == 0) fileClear();
      fileOutput( tests[ index ] );
    }
  }
}

bool TestGroup::running ()
{
  //Check if next() has been called since reset
  return index > -1;
}

bool TestGroup::done ()
{
  //Check if last test done
  if (!running()) return false;
  else return (index >= (int) tests.size()-1 &&
               tests[ index ]->done());
}

void TestGroup::fileClear ()
{
  //Open file just to truncate its contents
  std::ofstream os( file.c_str(), std::ofstream::trunc );
  if (os.is_open()) os.close();
}

void TestGroup::fileOutput (Test *test)
{
  //Open output file for appending
  std::ofstream os( file.c_str(), std::ofstream::app );
  if (!os.is_open()) {
    std::cout << "Failed opening file '" << file << "'" << std::endl;
    return;
  }

  //Write test name
  os << test->name << std::endl;
  os << std::endl;

  int nameWidth = strlen( "Argument" );
  int valueWidth = 10;

  //Find maxmimum width of the first column
  for (Uint e=0; e<test->environments.size(); ++e) {
    Env *env = test->environments[e];
    if ((int)env->name.length() > nameWidth)
      nameWidth = env->name.length();
  }

  //Write line containing all argument values
  os.width( nameWidth + 2);
  os << std::left << "Argument" << ",";

  for (Uint a=0; a<test->arguments.size(); ++a) {
    os.precision( 2 );
    os.width( valueWidth );
    os << std::right << std::fixed << test->arguments[a] << ",";
  }

  os << std::endl;

  //Write a line containing all result values for each environment
  for (Uint e=0; e<test->environments.size(); ++e)
  {
    Env *env = test->environments[e];
    os.width( nameWidth + 2);
    os << std::left << env->name << ",";

    for (Uint v=0; v<env->values.size(); ++v) {
      os.precision( 2 );
      os.width( valueWidth );
      os << std::right << std::fixed << env->values[v] << ",";
    }

    os << std::endl;
  }

  //Write an empty line and close file
  os << std::endl;
  os.close();
}


////////////////////////////////////////////////////////////////////////////////


void initTests (TestGroup &group)
{
  //////////////////////////////////////////////////////////////
  // Arguments

  int argsGridSize[] = { 10, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200 };
  //float argsGridSize[] = { 40, 60, 80 };
  
  //float argsZoomScale[] = { 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f };
  float argsZoomScale[] = { 0.6f, 0.8f, 1.0f, 1.2f, 1.4f, 1.6f, 1.8f };
  //float argsZoomScale[] = { 0.6f, 1.0f, 1.5f };

  int argsNumGlyphs[] = { 200, 300, 400, 500, 600, 700, 800, 900, 1000 };
  
  int numRepeat = 4;
  //int numRepeat = 2;
  
  int numDrop = 1;

  /////////////////////////////////////////////////////////////
  // Options

  Options optMeasureTiger = 
    Option( Opt::Draw,    Draw::False ) +
    Option( Opt::Encode,  Encode::True ) +
    Option( Opt::Measure, Measure::True ) +
    Option( Opt::Source,  Source::Tiger );

  Options optEncodeTiger = 
    Option( Opt::Draw,   Draw::False ) +
    Option( Opt::Encode, Encode::True ) +
    Option( Opt::Source, Source::Tiger );

  Options optRenderTiger =
    Option( Opt::Draw,   Draw::True ) +
    Option( Opt::Encode, Encode::False ) +
    Option( Opt::View,   View::Flat ) +
    Option( Opt::Source, Source::Tiger );

  Options optEncodeRenderTiger = 
    Option( Opt::Draw,   Draw::True ) +
    Option( Opt::Encode, Encode::True ) +
    Option( Opt::View,   View::Flat ) +
    Option( Opt::Source, Source::Tiger );

  Options optRenderText =
    Option( Opt::Draw,   Draw::True ) +
    Option( Opt::Encode, Encode::False ) +
    Option( Opt::View,   View::Flat ) +
    Option( Opt::Source, Source::Text );

  Options optEncodeRenderText =
    Option( Opt::Draw,   Draw::True ) +
    Option( Opt::Encode, Encode::True ) +
    Option( Opt::View,   View::Flat ) +
    Option( Opt::Source, Source::Text );

  OptionSet optRandomAuxCpu =
    Option( Opt::Render, Render::Random ) +
    Option( Opt::Rep,    Rep::Aux ) +
    Option( Opt::Proc,   Proc::Cpu );

  OptionSet optRandomPivotCpu = 
    Option( Opt::Render, Render::Random ) +
    Option( Opt::Rep,    Rep::Pivot ) +
    Option( Opt::Proc,   Proc::Cpu );

  OptionSet optRandomAuxGpu =
    Option( Opt::Render, Render::Random ) +
    Option( Opt::Rep,    Rep::Aux ) +
    Option( Opt::Proc,   Proc::Gpu );

  OptionSet optRandomPivotGpu = 
    Option( Opt::Render, Render::Random ) +
    Option( Opt::Rep,    Rep::Pivot ) +
    Option( Opt::Proc,   Proc::Gpu );

  OptionSet optClassic =
    Option( Opt::Render, Render::Classic );


  //////////////////////////////////////////////////
  // Tests

  Test *testMemory = new Test(
    "Vector image memory size at various grid sizes",
    Opt::GridSize, Res::StreamMegaBytes, 1, 0 );

  testMemory->addArguments( argsGridSize, sizeof(argsGridSize) );
  testMemory->addEnvironment( "Pivot",     optMeasureTiger + optRandomPivotCpu );
  testMemory->addEnvironment( "Auxiliary", optMeasureTiger + optRandomAuxCpu );


  Test *testTimeE = new Test(
    "Encoding speed at various grid sizes",
    Opt::GridSize, Res::Fps, numRepeat, numDrop );

  testTimeE->addArguments( argsGridSize, sizeof(argsGridSize) );
  testTimeE->addEnvironment( "GPU Pivot",     optEncodeTiger + optRandomPivotGpu );
  testTimeE->addEnvironment( "GPU Auxiliary", optEncodeTiger + optRandomAuxGpu );
  testTimeE->addEnvironment( "CPU Pivot",     optEncodeTiger + optRandomPivotCpu );
  testTimeE->addEnvironment( "CPU Auxiliary", optEncodeTiger + optRandomAuxCpu );


  Test *testTimeR = new Test( 
    "Rendering speed at various grid sizes",
    Opt::GridSize, Res::Fps, numRepeat, numDrop );

  testTimeR->addArguments( argsGridSize, sizeof(argsGridSize) );
  testTimeR->addEnvironment( "Pivot",     optRenderTiger + optRandomPivotGpu );
  testTimeR->addEnvironment( "Auxiliary", optRenderTiger + optRandomAuxGpu );
  testTimeR->addEnvironment( "Classic",   optRenderTiger + optClassic );


  Test *testTimeER = new Test( 
    "Encoding + Rendering speed at various grid sizes",
    Opt::GridSize, Res::Fps, numRepeat, numDrop );

  testTimeER->addArguments( argsGridSize, sizeof(argsGridSize) );
  testTimeER->addEnvironment( "GPU Pivot",     optEncodeRenderTiger + optRandomPivotGpu );
  testTimeER->addEnvironment( "GPU Auxiliary", optEncodeRenderTiger + optRandomAuxGpu );
  testTimeER->addEnvironment( "CPU Pivot",     optEncodeRenderTiger + optRandomPivotCpu );
  testTimeER->addEnvironment( "CPU Auxiliary", optEncodeRenderTiger + optRandomAuxCpu );


  Test *testSizeZoom = new Test(
    "Image screen width in pixels at various levels of zoom",
    Opt::ZoomS, Res::ScreenWidth, 1, 0 );

  testSizeZoom->addArguments( argsZoomScale, sizeof(argsZoomScale) );
  testSizeZoom->addEnvironment( "Width",     optRenderTiger + optRandomPivotGpu );


  Test *testTimeZoom = new Test(
    "Rendering speed at various levels of zoom",
    Opt::ZoomS, Res::Fps, numRepeat, numDrop );

  testTimeZoom->addArguments( argsZoomScale, sizeof(argsZoomScale) );
  testTimeZoom->addEnvironment( "Pivot",     optRenderTiger + Option( Opt::GridSize, 120 ) + optRandomPivotGpu );
  testTimeZoom->addEnvironment( "Auxiliary", optRenderTiger + Option( Opt::GridSize, 120 ) + optRandomAuxGpu );
  testTimeZoom->addEnvironment( "Classic",   optRenderTiger + Option( Opt::GridSize, 120 ) + optClassic );


  Test *testTimeGlyphs = new Test(
    "Rendering speed at various number of glyphs in the image",
    Opt::NumGlyphs, Res::Fps, numRepeat, numDrop );

  testTimeGlyphs->addArguments( argsNumGlyphs, sizeof(argsNumGlyphs) );
  testTimeGlyphs->addEnvironment( "Pivot",        optRenderText + optRandomPivotGpu );
  testTimeGlyphs->addEnvironment( "Encode Pivot", optEncodeRenderText + optRandomPivotGpu );
  testTimeGlyphs->addEnvironment( "Classic",      optRenderText + optClassic );


  ////////////////////////////////////////////
  // Test selection

  //group.addTest( testMemory );
  //group.addTest( testTimeE );
  //group.addTest( testTimeR );
  //group.addTest( testTimeER );
  group.addTest( testSizeZoom );
  group.addTest( testTimeZoom );
  //group.addTest( testTimeGlyphs );
}
