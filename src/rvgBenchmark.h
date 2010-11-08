#ifndef RVGBENCHMARK_H
#define RVGBENCHMARK_H 1

#include "rvgIntFloat.h"

class Option;
class OptionSet;
class Options;
class Results;

/////////////////////////////////////////////
//Options

namespace Opt {
  enum Enum {
    Draw        = 0,
    Encode      = 1,
    Proc        = 2,
    Rep         = 3,
    Render      = 4,
    View        = 5,
    Source      = 6,
    Measure     = 7,
    GridSize    = 8,
    ZoomS       = 9,
    PanX        = 10,
    PanY        = 11,
    NumGlyphs   = 12,
    Count       = 13
  };
};

namespace Draw {
  enum Enum {
    False = 0,
    True  = 1,
    Count = 2
  };
};

namespace Encode {
  enum Enum {
    False = 0,
    True  = 1,
    Count = 2
  };
};

namespace Measure {
  enum Enum {
    False = 0,
    True  = 1,
    Count = 2
  };
};

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

namespace Render {
  enum Enum {
    Classic  = 0,
    Random   = 1,
    Count    = 2
  };
};

namespace View {
  enum Enum {
    Flat      = 0,
    Cylinder  = 1,
    PaperFold = 2,
    Count     = 3
  };
};

namespace Source {
  enum Enum {
    Tiger  = 0,
    Text   = 1,
    World  = 2,
    Count  = 3
  };
};

class OptionsCount
{
  int counts[ Opt::Count ];

public:
  OptionsCount()
  {
    counts[ Opt::Draw ]    = Draw::Count;
    counts[ Opt::Encode ]  = Encode::Count;
    counts[ Opt::Proc ]    = Proc::Count;
    counts[ Opt::Rep ]     = Rep::Count;
    counts[ Opt::Render ]  = Render::Count;
    counts[ Opt::View ]    = View::Count;
    counts[ Opt::Source ]  = Source::Count;
    counts[ Opt::Measure ] = Measure::Count;
  }

  int operator[] (int o)
  {
    return counts[o];
  }
};

////////////////////////////////////////////////////////////////
// Option holds a value for a specific option type

class Option
{
public:
  int type;
  IntFloat value;

  Option () {
    type = 0; value = 0;
  }

  Option (int t, int i) {
    type = t; value = i;
  }

  Option (int t, float f) {
    type = t; value = f;
  }
};

////////////////////////////////////////////////////////////////
// OptionSet holds a set of values for specific options types

class OptionSet
{
  friend class Options;
  std::vector< Option > options;

public:

  OptionSet (const Option &opt)
  {
    //Init with a single option
    options.push_back( opt );
  }

  OptionSet& operator+= (const Option &opt)
  {
    //Search for existing option of the same type
    for (Uint o=0; o<options.size(); ++o) {
      if (options[o].type == opt.type) {

        //Set value of existing option
        options[o].value = opt.value;        
        return *this;
      }
    }

    //Insert new option
    options.push_back( opt );
    return *this;
  }

  OptionSet operator+ (const Option &opt)
  {
    OptionSet newset = *this;
    newset += opt;
    return newset;
  }

  OptionSet& operator+= (const OptionSet &set)
  {
    //Insert all the options from the other set
    for (Uint s=0; s<set.options.size(); ++s)
      *this += set.options[ s ];
    return *this;
  }

  OptionSet operator+ (const OptionSet &set)
  {
    OptionSet newset = *this;
    newset += set;
    return newset;
  }
};

//Creation from two Options
inline OptionSet operator+ (const Option& opt1, const Option &opt2)
{
  return OptionSet(opt1) + OptionSet(opt2);
}


////////////////////////////////////////////////////////////////
// Options hold entire universe of option values

class Options
{
  IntFloat values[ Opt::Count ];

  //Default values

  void setDefaults()
  {
    values[ Opt::Draw ]      = Draw::True;
    values[ Opt::Encode ]    = Encode::True;
    values[ Opt::Proc ]      = Proc::Gpu;
    values[ Opt::Rep ]       = Rep::Pivot;
    values[ Opt::Render ]    = Render::Random;
    values[ Opt::View ]      = View::Flat;
    values[ Opt::Source ]    = Source::Tiger;
    values[ Opt::Measure ]   = Measure::False;
    values[ Opt::GridSize ]  = 60;
    values[ Opt::ZoomS ]     = 1.0f;
    values[ Opt::PanX ]      = 0.0f;
    values[ Opt::PanY ]      = 0.0f;
    values[ Opt::NumGlyphs ] = 100;
  }

public:

  Options () { setDefaults(); }

  //Construction, assignment and comparison

  Options (const Options &opt)
  {
    for (int o=0; o < Opt::Count; ++o)
      values[ o ] = opt.values[ o ];
  }

  Options& operator= (const Options &opt)
  {
    for (int o=0; o < Opt::Count; ++o)
      values[ o ] = opt.values[ o ];

    return *this;
  }

  bool operator== (const Options &opt) const
  {
    for (int o=0; o<Opt::Count; ++o)
      if (values[ o ] != opt.values[ o ])
        return false;

    return true;
  }

  bool operator!= (const Options &opt) const
  {
    return !(this->operator==(opt));
  }

  IntFloat& operator[] (int o)
  {
    return values[o];
  }

  //Interaction with OptionSet

  Options (const OptionSet &set)
  {
    setDefaults();
    *this += set;
  }

  Options& operator+= (const OptionSet &set)
  {
    for (Uint s=0; s<set.options.size(); ++s)
      values[ set.options[s].type ] = set.options[s].value;

    return *this;
  }

  Options& operator= (const OptionSet &set)
  {
    return *this += set;
  }

  Options operator+ (const OptionSet &set)
  {
    Options opt = *this;
    opt += set;
    return opt;
  }
};


/////////////////////////////////////////////
//Results

namespace Res {
  enum Enum {
    Fps             = 0,
    ScreenWidth     = 1,
    ScreenHeight    = 2,
    StreamMegaBytes = 3,
    CellBytes       = 4,
    CellWords       = 5,
    CellObjects     = 6,
    CellSegments    = 7,
    Count           = 8
  };
};

class Results
{
  IntFloat values[ Res::Count ];

public:

  Results()
  {
    for (int r=0; r<Res::Count; ++r)
      values[r] = 0;
  }

  Results (const Results &res)
  {
    for (int r=0; r<Res::Count; ++r)
      values[r] = res.values[r];
  }

  Results& operator= (const Results &res)
  {
    for (int r=0; r<Res::Count; ++r)
      values[r] = res.values[r];

    return *this;
  }

  IntFloat& operator[] (int r)
  {
    return values[r];
  }
};

///////////////////////////////////////////////
//Test - maps all the arguments in the domain
//to values in codomain for every environment

struct Env
{
  std::string name;
  Options options;
  std::vector< IntFloat > values;
};

class Test
{
  friend class TestGroup;

private:

  //Settings
  std::string name;
  int repeatCount;
  int dropCount;

  int domain;
  std::vector< IntFloat > arguments;

  int codomain;
  std::vector< Env* > environments;

  //Runtime counters
  int repIndex;
  int envIndex;
  int argIndex;
  std::vector< IntFloat > values;

  void averageValues ();
  void resizeValues ();

public:

  //Setup functions
  Test (const std::string &name, Opt::Enum domain, Res::Enum codomain, int repeatCount=1, int dropCount=0);
  ~Test();

  template <class T>
  void addArgument (T a)
  {
    arguments.push_back( a );
    resizeValues();
  }

  template <class T>
  void addArguments (T *a, int size)
  {
    int count = size / sizeof(T);
    for (int c=0; c<count; ++c)
      arguments.push_back( a[c] );
    resizeValues();
  }

  void addEnvironment (const std::string &name, const Options &opt)
  {
    Env *env = new Env;
    env->name = name;
    env->options = opt;
    environments.push_back( env );
    resizeValues();
  }

  //Run functions
  void reset();
  Options next();
  void results (Results &res);
  bool running();
  bool done();
  
  //Query functions
  int getDomain() { return domain; }
  int getCodomain() { return codomain; }
};

///////////////////////////////////////////////////
//TestGroup - maps all the arguments in the domain
//to values in codomain for every environment

class TestGroup
{
  int index;
  std::string file;
  std::vector< Test* > tests;

  void fileClear ();
  void fileOutput (Test *test);

public:

  //Setup functions
  TestGroup (const std::string &file);
  void addTest (Test *test);
  
  //Run functions
  void reset();
  Options next();
  void results (Results &res);
  bool running ();
  bool done ();
};



void initTests (TestGroup &group);

#endif//RVGBENCHMARK_h
