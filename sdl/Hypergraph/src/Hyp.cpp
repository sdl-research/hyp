/** \file

    Single argv[0] (link/copy) or argv[1]-selected Hyp* utilities.
*/

#include <sdl/Util/Locale.hpp>
#include <sdl/graehl/shared/named_main.hpp>

#ifndef SDL_INLINE_HYP_CPPS
#define SDL_INLINE_HYP_CPPS 1
// seprate compilation => linking is actually slower
#endif

#if SDL_INLINE_HYP_CPPS
#define SDL_TRANSFORM_MAIN_LOG_WEIGHT 1
#define SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT 1
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/src/HypBest.cpp>
#include <sdl/Hypergraph/src/HypCompose.cpp>
#include <sdl/Hypergraph/src/HypInside.cpp>
#include <sdl/Hypergraph/src/HypEmpty.cpp>
#include <sdl/Hypergraph/src/HypReweightBest.cpp>
#include <sdl/Hypergraph/src/HypConvertCharsToTokens.cpp>
#include <sdl/Hypergraph/src/HypComplement.cpp>
#include <sdl/Hypergraph/src/HypInvert.cpp>
#include <sdl/Hypergraph/src/HypIsolateStart.cpp>
#include <sdl/Hypergraph/src/HypDeterminize.cpp>
#include <sdl/Hypergraph/src/HypPrune.cpp>
#include <sdl/Hypergraph/src/HypSamplePath.cpp>
#include <sdl/Hypergraph/src/HypUnion.cpp>
#include <sdl/Hypergraph/src/HypSubUnion.cpp>
#include <sdl/Hypergraph/src/HypConcat.cpp>
#include <sdl/Hypergraph/src/HypReverse.cpp>
#include <sdl/Hypergraph/src/HypReweight.cpp>
#include <sdl/Hypergraph/src/HypPruneToBest.cpp>
#include <sdl/Hypergraph/src/HypToMosesLattice.cpp>
#include <sdl/Hypergraph/src/HypConvertStrings.cpp>
#include <sdl/Hypergraph/src/HypDraw.cpp>
#include <sdl/Hypergraph/src/HypFsmDraw.cpp>
#include <sdl/Hypergraph/src/HypToOpenFstText.cpp>
#include <sdl/Hypergraph/src/HypGetString.cpp>
#include <sdl/Hypergraph/src/HypTrie.cpp>
#include <sdl/Hypergraph/src/HypProject.cpp>
#if HAVE_OPENFST
#include <sdl/Hypergraph/src/HypToReplaceFst.cpp>
#endif
#else
#include <sdl/graehl/shared/force_link.hpp>
#endif

#if HAVE_OPENFST
#define SDL_FOR_OPENFST_HYP_MAINS(x) x(HypToReplaceFst)
#else
#define SDL_FOR_OPENFST_HYP_MAINS(x)
#endif

#define SDL_HYP_FOR_MAINS(x) \
  x(HypBest) \
  x(HypCompose) \
  x(HypInside) \
  x(HypEmpty) \
  x(HypReweightBest) \
  x(HypConvertCharsToTokens) \
  x(HypComplement) \
  x(HypInvert) \
  x(HypIsolateStart) \
  x(HypDeterminize) \
  x(HypPrune) \
  x(HypSamplePath) \
  x(HypUnion) \
  x(HypSubUnion) \
  x(HypConcat) \
  x(HypReverse) \
  x(HypReweight) \
  x(HypPruneToBest) \
  x(HypToMosesLattice) \
  x(HypConvertStrings) \
  x(HypDraw) \
  x(HypFsmDraw) \
  x(HypToOpenFstText) \
  x(HypGetString) \
  x(HypTrie) \
  x(HypProject) \
  SDL_FOR_OPENFST_HYP_MAINS(x)


namespace sdl {
namespace Hypergraph {
void forceLinkMains() {
#if !SDL_INLINE_HYP_CPPS
  SDL_HYP_FOR_MAINS(GRAEHL_FORCE_LINK_CLASS)
#endif
}
}
}

int main(int argc, char* argv[]) {
  sdl::Hypergraph::forceLinkMains();
  return graehl::run_named_main(argc, argv);
}
