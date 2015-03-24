#define USAGE_HypEqual "equal cfg*fsm*...*fsm"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/fs/Equal.hpp>

namespace sdl {
namespace Hypergraph {

struct HypEqual : TransformMain<HypEqual> {

  HypEqual() : TransformMain<HypEqual>("Equal", USAGE_HypEqual) {}

  Properties properties(int i) const { return kDefaultProperties | kStoreOutArcs | kStoreInArcs; }

  enum { has_transform1 = false, has_inplace_transform2 = true };

  template <class Arc>
  bool transform2InPlace(IMutableHypergraph<Arc>& l, IHypergraph<Arc> const& r) {
    SDL_DEBUG(Hypergraph.HgEqual, "Equal input 1:\n" << l);
    SDL_DEBUG(Hypergraph.HgEqual, "Equal input 2:\n" << r);
    bool const eq = fs::equal(l, r);
    std::cout << eq << '\n';
    SDL_DEBUG(Hypergraph.HgEqual, "Result:\n" << eq);
    return eq;
  }
  bool printFinal() const { return false; }
};


}}

HYPERGRAPH_NAMED_MAIN(Equal)
