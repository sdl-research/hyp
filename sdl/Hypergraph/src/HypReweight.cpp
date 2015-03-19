#define TRANSFORM HypReweight
#define USAGE ReweightOptions::caption()
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Reweight.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // CRTP
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION)
  {
  }
  void declare_configurable() {
    this->configurable(&rw);
  }

  Reweight rw;
  static RandomSeed randomSeed() { return kRandomSeed; }
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    rw.inplace(h);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
