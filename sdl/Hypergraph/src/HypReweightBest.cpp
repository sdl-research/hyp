#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/ReweightBest.hpp>

namespace sdl {
namespace Hypergraph {

struct HypReweightBest : TransformMain<HypReweightBest> {  // CRTP
  typedef TransformMain<HypReweightBest> Base;
  HypReweightBest() : Base("ReweightBest", ReweightOptions::caption()) {}
  void declare_configurable() { this->configurable(&rw.opt); }

  ReweightBest rw;
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& h) {
    rw.inplace(h);
    return true;
  }
  void validate_parameters_more() { rw.opt.validate(); }
};


}}

HYPERGRAPH_NAMED_MAIN(ReweightBest)
