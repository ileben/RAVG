#ifndef RVGBENCHMARK_H
#define RVGBENCHMARK_H 1

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
    Count   = 7
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
    counts[ Opt::Encode ]  = Encode::Count;
    counts[ Opt::Draw ]    = Draw::Count;
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

class Options
{
  int values[ Opt::Count ];

public:
  Options ()
  {
    values[ Opt::Encode ]  = Encode::True;
    values[ Opt::Draw ]    = Draw::True;
    values[ Opt::Proc ]    = Proc::Gpu;
    values[ Opt::Rep ]     = Rep::Pivot;
    values[ Opt::Render ]  = Render::Random;
    values[ Opt::View ]    = View::Flat;
    values[ Opt::Source]   = Source::Tiger;
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
};

/////////////////////////////////////////////
//Results

namespace Result {
  enum Enum {
    Fps      = 0,
    Memory   = 1,
    Count    = 2
  };
};

class Results
{
  int values[ Result::Count ];

public:

  Results()
  {
    for (int r=0; r<Result::Count; ++r)
      values[r] = 0;
  }

  Results (const Results &res)
  {
    for (int r=0; r<Result::Count; ++r)
      values[r] = res.values[r];
  }

  Results& operator= (const Results &res)
  {
    for (int r=0; r<Result::Count; ++r)
      values[r] = res.values[r];

    return *this;
  }

  int operator[] (int r) const
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

  Opt::Enum domain;
  std::vector< int > arguments;

  Result::Enum codomain;
  std::vector< Env > environments;

  //Runtime counters
  int repIndex;
  int envIndex;
  int argIndex;
  std::vector< int > values;

public:

  Test (Opt::Enum domain, int repeatCount=1)
    : domain(domain), repeatCount(repeatCount)
  {
    reset();
  }

  void addArgument (int a)
  {
    arguments.push_back(a);
  }

  void addEnvironment (const std::string &name, const Options &opt)
  {
    Env env;
    env.name = name;
    env.options = opt;
    environments.push_back( env );
  }

  void reset()
  {
    repIndex = 0;
    envIndex = 0;
    argIndex = 0;
    values.clear();
  }

  Options first()
  {
    //Reset to first test
    reset();

    //Get options for the current environment and adjust argument
    Options opt = environments[ envIndex ].options;
    opt[ domain ] = arguments[ argIndex ];
    return opt;
  }

  Options next()
  {
    //Move to next repetition, argument or environment
    if (++repIndex == repeatCount) {
      repIndex = 0;
      if (++argIndex == arguments.size()) {
        argIndex = 0;
        if (++envIndex == environments.size()) {
          return first();
        }}}

    //Get options for the current environment and adjust argument
    Options opt = environments[ envIndex ].options;
    opt[ domain ] = arguments[ argIndex ];
    return opt;
  }

  void results (const Results &res)
  {
    //Store value into list and check if last
    values.push_back( res[ codomain ] );
    if (repIndex == repeatCount-1) {

      //Find average of all values
      int avgValue = 0;
      for (Uint32 v=0; v<values.size(); ++v)
        avgValue += values[v];

      //Store as value corresponding to current argument
      environments[ envIndex ].values.push_back( avgValue );
      values.clear();
    }
  }

  bool done()
  {
    //Check if last repetition of last argument in last environment reached
    return (envIndex == environments.size()-1 &&
            argIndex == arguments.size()-1 &&
            repIndex == repeatCount-1);
  }
};


#endif//RVGBENCHMARK_h
