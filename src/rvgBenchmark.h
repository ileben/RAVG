#ifndef RVGBENCHMARK_H
#define RVGBENCHMARK_H 1

class OptVal;
class Options;
class Results;

/////////////////////////////////////////////
//Options

namespace Opt {
  enum Enum {
    Draw    = 0,
    Encode  = 1,
    Proc    = 2,
    Rep     = 3,
    Render  = 4,
    View    = 5,
    Source  = 6,
    Grid    = 7,
    Count   = 8
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
    Count     = 2
  };
};

namespace Source {
  enum Enum {
    Tiger  = 0,
    Text   = 1,
    Count  = 2
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
    counts[ Opt::Source]   = Source::Count;
  }

  int operator[] (int o)
  {
    return counts[o];
  }
};

class OptVal
{
public:
  Opt::Enum type;
  int value;

  OptVal (Opt::Enum t, int v) {
    type = t; value = v;
  }
};

class Options
{
  int values[ Opt::Count ];

public:
  Options ()
  {
    values[ Opt::Draw ]    = Draw::True;
    values[ Opt::Encode ]  = Encode::True;
    values[ Opt::Proc ]    = Proc::Gpu;
    values[ Opt::Rep ]     = Rep::Pivot;
    values[ Opt::Render ]  = Render::Random;
    values[ Opt::View ]    = View::Flat;
    values[ Opt::Source]   = Source::Tiger;
    values[ Opt::Grid]     = 60;
  }

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
    return ! (*this == opt);
  }

  int& operator[] (int o)
  {
    return values[o];
  }

  Options& operator += (const OptVal &ov)
  {
    values[ ov.type ] = ov.value;
    return *this;
  }

  Options operator+ (const OptVal &ov)
  {
    Options opt = *this;
    opt += ov;
    return opt;
  }
};

Options operator+ (const OptVal &ov1, const OptVal &ov2)
{
  Options opt;
  opt += ov1;
  opt += ov2;
  return opt;
}

/////////////////////////////////////////////
//Results

namespace Res {
  enum Enum {
    Fps      = 0,
    Memory   = 1,
    Count    = 2
  };
};

class Results
{
  int values[ Res::Count ];

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

  int& operator[] (int r)
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
  std::vector< int > values;
};

class Test
{ 
private:

  //Settings
  int repeatCount;
  int dropCount;

  Opt::Enum domain;
  std::vector< int > arguments;

  Res::Enum codomain;
  std::vector< Env* > environments;

  //Runtime counters
  int repIndex;
  int envIndex;
  int argIndex;
  std::vector< int > values;

  void averageValues ()
  {
    int avgValue = 0;
    int numValues = 0;

    //Find average of all values after dropping first few
    for (Uint32 v=dropCount; v<values.size(); ++v) {
      std::cout << "Average part: " << values[v] << std::endl;
      avgValue += values[v];
      numValues += 1;
    }

    //Store as value corresponding to current argument
    environments[ envIndex ]->values[ argIndex ] = avgValue / numValues;
    std::cout << "Average: " << environments[ envIndex ]->values[ argIndex ] << std::endl;
  }

  void resizeValues ()
  {
    //Make sure the number of temporary values matches the number of repetitions
    values.resize( repeatCount, 0 );

    //Make sure the number of environment values matches the number of arguments
    for (Uint32 e=0; e<environments.size(); ++e)
      environments[e]->values.resize( arguments.size(), 0 );
  }

public:

  Test (Opt::Enum domain, Res::Enum codomain, int repeatCount=1, int dropCount=0)
    : domain(domain), codomain(codomain), repeatCount(repeatCount), dropCount(dropCount)
  {
    if (repeatCount <= 0)
      repeatCount = 1;

    if (dropCount >= repeatCount)
      dropCount = repeatCount-1;

    reset();
  }

  ~Test()
  {
    for (Uint32 e=0; e<environments.size(); ++e)
      delete environments[e];
  }

  void addArgument (int a)
  {
    arguments.push_back(a);
    resizeValues();
  }

  void addArguments (int a, ...)
  {
    va_list va;
    va_start (va, a);
    do
    {
      arguments.push_back(a);
      a = va_arg( va, int );
    }
    while (a != -1);
    va_end( va );
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

  void reset()
  {
    repIndex = -1;
    envIndex = -1;
    argIndex = -1;
  }

  Options next()
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

  void results (Results &res)
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

  bool running()
  {
    //Check if next() has been called since reset
    return (envIndex > -1 &&
            argIndex > -1 &&
            repIndex > -1);
  }

  bool done()
  {
    //Check if last repetition of last argument in last environment reached
    return (envIndex >= (int) environments.size()-1 &&
            argIndex >= (int) arguments.size()-1 &&
            repIndex >= (int) repeatCount-1);
  }
};


#endif//RVGBENCHMARK_h
