#define PROGNAME HgConvertStrings

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





