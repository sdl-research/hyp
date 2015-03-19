#define TRANSFORM HypDeterminize
#define USAGE "Determinize an unweighted FSA hypergraph -- input symbols only. TODO: support sigma, phi, weights, outputs."
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Determinize.hpp>
#include <sdl/Hypergraph/DeterminizeOptions.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION)
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

INT_MAIN(sdl::Hypergraph::TRANSFORM)
