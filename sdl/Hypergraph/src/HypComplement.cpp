#define TRANSFORM HypComplement
#define USAGE "Create the complement (determinize first if necessary) of an unweighted fsa hypergraph -- input symbols only"
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/fs/Complement.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    fs::complement(i, o);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
