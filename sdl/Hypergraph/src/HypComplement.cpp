#define USAGE_HypComplement                                                                                  \
  "Create the complement (determinize first if necessary) of an unweighted fsa hypergraph -- input symbols " \
  "only"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/fs/Complement.hpp>

namespace sdl {
namespace Hypergraph {

struct HypComplement : TransformMain<HypComplement> {  // note base class CRTP (google it)
  HypComplement() : TransformMain<HypComplement>("Complement", USAGE_HypComplement) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {
    fs::complement(i, o);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(Complement)
