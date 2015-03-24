#define USAGE_HypConvertCharsToTokens \
  "concatenate 'character' tokens between <tok> </tok> into single 'word' tokens"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/ConvertCharsToTokens.hpp>

namespace sdl {
namespace Hypergraph {

struct HypConvertCharsToTokens : TransformMain<HypConvertCharsToTokens> {
  HypConvertCharsToTokens()
      : TransformMain<HypConvertCharsToTokens>("ConvertCharsToTokens", USAGE_HypConvertCharsToTokens) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {
    convertCharsToTokens(i, o);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(ConvertCharsToTokens)
