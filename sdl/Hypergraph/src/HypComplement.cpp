#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
















#define TRANSFORM HgComplement

#define VERSION "v1"
#define HG_TRANSFORM_MAIN



namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    fs::complement(i, o);
    return true;
  }
};




