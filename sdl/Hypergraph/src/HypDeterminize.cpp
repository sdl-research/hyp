#define USAGE_HypDeterminize "Determinize an unweighted FSA hypergraph -- input symbols only. TODO: support sigma, phi, weights, outputs."
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Determinize.hpp>
#include <sdl/Hypergraph/DeterminizeOptions.hpp>

namespace sdl {
namespace Hypergraph {

struct HypDeterminize : TransformMain<HypDeterminize> { // note base class CRTP (google it)
  HypDeterminize() : TransformMain<HypDeterminize>("Determinize", USAGE_HypDeterminize)
  {}
  DeterminizeOptions detOpt;
  void declare_configurable() {
    this->configurable(&detOpt);
  }

  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    determinize(i, o, detOpt.getFlags());
    return true;
  }
};

}}

HYPERGRAPH_NAMED_MAIN(Determinize)
