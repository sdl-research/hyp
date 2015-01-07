// Copyright 2014 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Concat.hpp>
#include <sdl/Hypergraph/SortArcs.hpp>

namespace sdl {
namespace Hypergraph {

#define USAGE "Concatenate cfg*...*cfg (cross product of yields).\n (If all are FSMs, result is also FSM)."
#define VERSION "v1"

// FIXME: weird problem with TransformMain cascades of inplace transform2

struct HypConcat : TransformMain<HypConcat> {

  HypConcat() : TransformMain<HypConcat>("HypConcat", USAGE, VERSION) {
    this->configurable(&opt);
    configureInputs();
  }

  // TODO: inefficient to always store in and out arcs
  Properties properties(int i) const {  // 0 is out, 1 is cfg, 2 and on are all fsms
    return kDefaultProperties | kStoreOutArcs | kStoreInArcs;
  }

  ConcatOptions opt;

  enum { has_transform1 = false, has_inplace_transform2 = true };

  template <class Arc>
  bool transform2InPlace(IMutableHypergraph<Arc>& l, IHypergraph<Arc> const& r) {
    SDL_DEBUG(Hypergraph.HgConcat, "Concat input 1:\n" << l);
    SDL_DEBUG(Hypergraph.HgConcat, "Concat input 2:\n" << r);
    hgConcat(&l, r, opt);
    SDL_DEBUG(Hypergraph.HgConcat, "Result:\n" << l);
    return true;
  }
};


}}

INT_MAIN(sdl::Hypergraph::HypConcat)
