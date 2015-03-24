#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Reweight.hpp>

namespace sdl {
namespace Hypergraph {

struct HypReweight : TransformMain<HypReweight> {
  typedef TransformMain<HypReweight> Base;
  HypReweight() : Base("Reweight", ReweightOptions::caption()) {}
  void declare_configurable() { this->configurable(&rw); }

  Reweight rw;
  static RandomSeed randomSeed() { return kRandomSeed; }
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& h) {
    rw.inplace(h);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(Reweight)
