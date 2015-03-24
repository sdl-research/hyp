#define USAGE_HypReverse "Reverse strings in language (todo: support cfg)"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Reverse.hpp>

namespace sdl {
namespace Hypergraph {

struct HypReverse : TransformMain<HypReverse> {  // note base class CRTP (google it)
  HypReverse() : TransformMain<HypReverse>("Reverse", USAGE_HypReverse) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {
    reverse(i, o);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(Reverse)
