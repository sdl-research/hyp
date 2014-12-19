#define TRANSFORM HgFct

#define USAGE "Debugging binary, for internal use."
#define VERSION "v1"

#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>

namespace sdl {
namespace Hypergraph {

template <class Arc>
void testFct(IMutableHypergraph<Arc> &h) {
}

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)

  typedef TransformMain<TRANSFORM> Base;

  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}

  Properties properties(int i) const {
    return kDefaultProperties | kStoreInArcs | kStoreOutArcs;
  }

  bool printFinal() const {
    return false;
  }

  enum { has_inplace_transform1 = true };

  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    testFct(h);
    return true;
  }
};


}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
