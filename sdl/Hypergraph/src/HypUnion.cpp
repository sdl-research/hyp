#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Union.hpp>
#include <sdl/Hypergraph/SortArcs.hpp>

namespace sdl {

namespace Hypergraph {

#define USAGE "Create the union of multiple hypergraphs\n (if all are FSM, the result is also FSM)"
#define VERSION "v1"

struct HypUnion : TransformMain<HypUnion> {

  HypUnion() : TransformMain<HypUnion>("HypUnion", USAGE, VERSION) {
    opt.require_ins();
    configureInputs();
  }

  Properties properties(int i) const {  // 0 is out, 1 is cfg, 2 and on are all FSMs
    return kDefaultProperties | kStoreFirstTailOutArcs | kStoreInArcs;
  }

  enum { has_transform1 = false, has_inplace_transform2 = true };

  char const* transform2sep() const { return " + "; }

  template <class Arc>
  bool transform2InPlace(IMutableHypergraph<Arc>& hg2, IHypergraph<Arc> const& hg1) {
    SDL_DEBUG(Hypergraph.HgUnion, "Union input 1:\n" << hg1);
    SDL_DEBUG(Hypergraph.HgUnion, "Union input 2:\n" << hg2);
    hgUnion(hg1, &hg2);
    SDL_DEBUG(Hypergraph.HgUnion, "Result:\n" << hg2);
    return true;
  }
};


}}

INT_MAIN(sdl::Hypergraph::HypUnion)
