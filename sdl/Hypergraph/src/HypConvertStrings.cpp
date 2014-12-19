#define PROGNAME HgConvertStrings
#define USAGE "Convert a single line to an FSA accepting its words, (or if -c, unicode chars). If multiple lines, NULL byte separates outputs"
#define VERSION "v1"



#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define HG_MAIN




















namespace po = boost::program_options;


namespace Hypergraph {



struct PROGNAME : HypergraphMainBase {




    this->configurable(&opt);
  }


  void run() {


    typedef ViterbiWeightTpl<float> Weight;
    typedef ArcTpl<Weight> Arc;

    std::string line;
    bool first = true;
    std::size_t linei = 0;








      first = false;
    }
    ++linei;
  }
};





