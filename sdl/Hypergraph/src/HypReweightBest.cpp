#define TRANSFORM HypReweightBest
#define USAGE ReweightOptions::caption()
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/ReweightBest.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // CRTP
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION)
  {
  }
  void declare_configurable() {
    this->configurable(&rw.opt);
  }

  ReweightBest rw;
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    rw.inplace(h);
    return true;
  }
  void validate_parameters_more() {
    rw.opt.validate();
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
