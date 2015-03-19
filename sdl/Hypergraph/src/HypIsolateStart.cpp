#define TRANSFORM HgIsolateStartState
#define USAGE "Create the complement (determinize first if necessary) of an unweighted fsa hypergraph -- input symbols only"
#define VERSION "v1"

#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/IsolateStartState.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}
  IsolateStartState iso;
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    inplace(h, iso);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
