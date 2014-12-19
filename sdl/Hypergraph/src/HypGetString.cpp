#define TRANSFORM HgGetString
#define USAGE "get the single string found by taking the first edge into each node starting from final (TOP)"
#define VERSION "v1"
#define HG_TRANSFORM_MAIN

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
















namespace po = boost::program_options;


namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;










  }
};




