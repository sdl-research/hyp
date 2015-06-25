// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    Single argv[0] (link/copy) or argv[1]-selected Hyp* utilities.
*/

#include <sdl/Util/Locale.hpp>
#include <graehl/shared/named_main.hpp>

#ifndef SDL_MINIMAL_HYP_MAIN
#define SDL_MINIMAL_HYP_MAIN 0
#endif
#define SDL_TRANSFORM_MAIN_LOG_WEIGHT 1
#define SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT 1
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/src/HypBest.cpp>
#include <sdl/Hypergraph/src/HypCompose.cpp>
#include <sdl/Hypergraph/src/HypInside.cpp>
#include <sdl/Hypergraph/src/HypEmpty.cpp>
#include <sdl/Hypergraph/src/HypPruneToBest.cpp>
#include <sdl/Hypergraph/src/HypPushWeights.cpp>
#if !SDL_MINIMAL_HYP_MAIN
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
#endif

#if SDL_FIX_LOCALE
sdl::Util::DefaultLocaleFastCout initCout;
#endif

#if HAVE_OPENFST
#define SDL_FOR_OPENFST_HYP_MAINS(x) x(HypToReplaceFst)
#else
#define SDL_FOR_OPENFST_HYP_MAINS(x)
#endif

#define SDL_HYP_FOR_MAINS_MINIMAL(x) \
  x(HypBest) \
  x(HypCompose) \
  x(HypInside) \
  x(HypEmpty) \
  x(HypPruneToBest) \
  x(HypPushWeights)

#if SDL_MINIMAL_HYP_MAIN
#define SDL_HYP_FOR_MAINS(x) SDL_HYP_FOR_MAINS_MINIMAL(x)
#else
#define SDL_HYP_FOR_MAINS(x) SDL_HYP_FOR_MAINS_MINIMAL(x) \
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
  x(HypToMosesLattice) \
  x(HypConvertStrings) \
  x(HypDraw) \
  x(HypFsmDraw) \
  x(HypToOpenFstText) \
  x(HypGetString) \
  x(HypTrie) \
  x(HypProject) \
  SDL_FOR_OPENFST_HYP_MAINS(x)
#endif

int main(int argc, char* argv[]) {
  return graehl::run_named_main(argc, argv);
}
