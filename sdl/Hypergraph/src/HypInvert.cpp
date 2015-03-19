#define TRANSFORM HypInvert
#define USAGE "Invert (swap) lexical ('input' 'output') state labels of hypergraph."
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Invert.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}
  Properties properties(int i) const {
    return kStoreInArcs;
  }
  enum { has_inplace_transform1 = true, has_transform1 = false };

  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& hg) {
    invert(hg);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
