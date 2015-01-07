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
