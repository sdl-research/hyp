#define USAGE_HypInvert "Invert (swap) lexical ('input' 'output') state labels of hypergraph."
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Invert.hpp>

namespace sdl {
namespace Hypergraph {

struct HypInvert : TransformMain<HypInvert> {  // note base class CRTP (google it)
  HypInvert() : TransformMain<HypInvert>("Invert", USAGE_HypInvert) {}
  Properties properties(int i) const { return kStoreInArcs; }
  enum { has_inplace_transform1 = true, has_transform1 = false };

  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& hg) {
    invert(hg);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(Invert)
