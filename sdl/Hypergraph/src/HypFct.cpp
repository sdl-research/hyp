#define USAGE_HypFct "Debugging binary, for internal use."

#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>

namespace sdl {
namespace Hypergraph {

template <class Arc>
void testFct(IMutableHypergraph<Arc>& h) {
  // your test here
}

struct HypFct : TransformMain<HypFct> {
  HypFct() : TransformMain<HypFct>("Fct", USAGE_HypFct) {}

  Properties properties(int i) const { return kDefaultProperties | kStoreInArcs | kStoreFirstTailOutArcs; }

  bool printFinal() const { return false; }

  enum { has_inplace_transform1 = true };

  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& h) {
    testFct(h);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(Fct)
