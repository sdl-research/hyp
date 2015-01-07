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
#include <sdl/Hypergraph/Prune.hpp>

namespace sdl {
namespace Hypergraph {

#define USAGE "Print nothing if input hypergraph is empty (i.e., cannot reach final state from start and lexical leaves); otherwise print input hypergraph with useless states/arcs removed"
#define VERSION "v1"

struct HypPrune : TransformMain<HypPrune> {
  typedef TransformMain<HypPrune> Base;
  HypPrune() : Base("HypPrune", USAGE, VERSION)
  {}
  void declare_configurable() {
    this->configurable(&popt);
  }

  PruneOptions popt;
  Properties properties(int i) const {
    return kStoreOutArcs;
  }

  enum { has_inplace_input_transform = true, has_transform1 = false };

  bool printFinal() const { return true; }

  static bool nbestHypergraphDefault() { return false; }

  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc> &i, int n) const {
    PruneUnreachable<Arc> p(popt);
    inplace_always(i, p);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::HypPrune)
