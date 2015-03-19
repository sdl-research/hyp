#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/BestPath.hpp>
#include <sdl/Hypergraph/GetString.hpp>
#include <sdl/Exception.hpp>

namespace sdl {
namespace Hypergraph {

struct HypBest : TransformMain<HypBest> { // note base class CRTP (google it)
  typedef TransformMain<HypBest> Base;
  HypBest()
      : Base("HypBest", BestPathOptions::usage(), "v1")
  {
    optBestOutputs.enable = true;
    // since best-paths is now an optional standard part of TransformMain, there's not much going on here
  }
  Properties properties(int i) const {
    return kDefaultProperties | kStoreInArcs;
  }
  enum { has_inplace_input_transform=false, has_transform1=false };
  static int defaultSemiring() { return TransformMainBase::viterbi; }
  static BestOutput bestOutput() { return kBestOutput; }
  static LineInputs lineInputs() { return kNoLineInputs; }

  void validate_parameters_more() {
  }
};

}}

INT_MAIN(sdl::Hypergraph::HypBest)
