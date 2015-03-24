#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/SubUnion.hpp>

namespace sdl {
namespace Hypergraph {

struct HypSubUnion : TransformMain<HypSubUnion> {
  SubUnionOptions opt;

  HypSubUnion() : TransformMain<HypSubUnion>("HypSubUnion", SubUnionOptions::usage()) {
    opt.requirePathOverlap = false;
    opt.addStandardUnion = false;
  }
  void declare_configurable() { this->configurable(&opt); }

  Properties properties(int i) const { return kDefaultProperties | kStoreInArcs; }

  enum {
    has_inplace_input_transform = true,
    has_transform1 = false,
    has_transform2 = true,
    out_every = false
  };  // means multiple input files
  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc>& hg, int n) {
    return true;
  }

  template <class Arc>
  bool transform2mm(IMutableHypergraph<Arc>& hg1, IMutableHypergraph<Arc>& hg2, IMutableHypergraph<Arc>* o) {
    subUnion(hg1, hg2, o, opt);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(SubUnion)
