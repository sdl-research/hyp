#define USAGE_HypGetString \
  "get the single string found by taking the first edge into each node starting from final (TOP)"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/GetString.hpp>

namespace sdl {
namespace Hypergraph {

struct HypGetString : TransformMain<HypGetString> {  // note base class CRTP (google it)
  HypGetString() : TransformMain<HypGetString>("GetString", USAGE_HypGetString) {}
  bool printFinal() const { return false; }
  Properties properties(int) const { return kStoreInArcs; }
  enum { has_transform1 = false, has_inplace_input_transform = 1 };
  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc>& hg, int lineno) const {
    std::string str = sdl::Hypergraph::getString(hg);
    out() << str << '\n';
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(GetString)
