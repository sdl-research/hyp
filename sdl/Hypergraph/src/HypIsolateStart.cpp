#define USAGE_HypIsolateStart                                                                                \
  "Create the complement (determinize first if necessary) of an unweighted fsa hypergraph -- input symbols " \
  "only"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/IsolateStartState.hpp>

namespace sdl {
namespace Hypergraph {

struct HypIsolateStart : TransformMain<HypIsolateStart> {  // note base class CRTP (google it)
  HypIsolateStart() : TransformMain<HypIsolateStart>("IsolateStart", USAGE_HypIsolateStart) {}
  IsolateStartState iso;
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& h) {
    inplace(h, iso);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(IsolateStart)
