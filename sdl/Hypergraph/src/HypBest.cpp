#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>

namespace sdl {
namespace Hypergraph {

struct HypBest : TransformMain<HypBest> {
  HypBest() : TransformMain<HypBest>("Best", BestPathOptions::usage()) {
    optBestOutputs.enable = true;
    // since best-paths is now an optional standard part of TransformMain, there's not much going on here
  }
  Properties properties(int i) const { return kDefaultProperties | kStoreInArcs; }
  enum { has_inplace_input_transform = false, has_transform1 = false };
  static int defaultSemiring() { return TransformMainBase::viterbi; }
  static BestOutput bestOutput() { return kBestOutput; }
  static LineInputs lineInputs() { return kNoLineInputs; }

  void validate_parameters_more() {}
};


}}

HYPERGRAPH_NAMED_MAIN(Best)
