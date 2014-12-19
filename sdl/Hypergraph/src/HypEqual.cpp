#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/fs/Equal.hpp>
#include <sdl/Hypergraph/SortArcs.hpp>

namespace sdl {
namespace Hypergraph {
#define USAGE "equal cfg*fsm*...*fsm"
#define VERSION "v1"

struct HypEqual : TransformMain<HypEqual> {

  HypEqual() : TransformMain<HypEqual>("HypEqual", USAGE, VERSION) {}

  Properties properties(int i) const {
    return kDefaultProperties | kStoreOutArcs | kStoreInArcs;
  }

  enum { has_transform1 = false, has_inplace_transform2 = true };

  template <class Arc>
  bool transform2InPlace(
      IMutableHypergraph<Arc> & l,
      IHypergraph<Arc> const& r) {
    SDL_DEBUG(Hypergraph.HgEqual, "Equal input 1:\n" << l);
    SDL_DEBUG(Hypergraph.HgEqual, "Equal input 2:\n" << r);
    bool eq = fs::equal(l, r);
    std::cout << eq << '\n';
    SDL_DEBUG(Hypergraph.HgEqual, "Result:\n" << eq);
    return eq;
  }
  bool printFinal() const
  {
    return false;
  }
};


}}

INT_MAIN(sdl::Hypergraph::HypEqual)