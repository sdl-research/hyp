#define TRANSFORM HypConvertCharsToTokens
#define USAGE "concatenate 'character' tokens between <tok> </tok> into single 'word' tokens"
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/ConvertCharsToTokens.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    convertCharsToTokens(i, o);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
