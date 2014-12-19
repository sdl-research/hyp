#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/PruneToNbest.hpp>

namespace sdl {
namespace Hypergraph {

#define USAGE "replace hypergraph with just the 1best path"
#define VERSION "v1"

struct HypPruneToBest : TransformMain<HypPruneToBest> {
  typedef TransformMain<HypPruneToBest> Base;
  HypPruneToBest() : Base("HypPruneToBest", USAGE, VERSION)
                  , opt(1)
  {}
  void declare_configurable() {
    this->configurable(&opt);
  }

  PruneToNbestOptions opt;

  Properties properties(int i) const {
    return kDefaultProperties | kStoreInArcs;
  }

  enum { has_inplace_input_transform = true, has_transform1 = false };

  bool printFinal() const { return true; }

  static bool nbestHypergraphDefault() { return false; }

  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc> &hg, int n) const {
    justNbest(hg, opt);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::HypPruneToBest)
