#define USAGE_HypPruneToBest "replace hypergraph with just the nbest paths"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/PruneNonBest.hpp>

namespace sdl {
namespace Hypergraph {

struct HypPruneToBest : TransformMain<HypPruneToBest> {
  typedef TransformMain<HypPruneToBest> Base;
  HypPruneToBest() : Base("PruneToBest", USAGE_HypPruneToBest), opt(1) {}
  void declare_configurable() { this->configurable(&opt); }

  PruneToNbestOptions opt;

  Properties properties(int i) const { return kDefaultProperties | kStoreInArcs; }

  enum { has_inplace_input_transform = true, has_transform1 = false };

  bool printFinal() const { return true; }

  static bool nbestHypergraphDefault() { return false; }

  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc>& hg, int n) const {
    justBest(hg, opt);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(PruneToBest)
