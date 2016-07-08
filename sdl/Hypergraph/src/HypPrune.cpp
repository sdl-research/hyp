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
#define USAGE_HypPrune                                                                                 \
  "Print nothing if input hypergraph is empty (i.e., cannot reach final state from start and lexical " \
  "leaves); otherwise print input hypergraph with useless states/arcs removed"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/Prune.hpp>
#include <sdl/Hypergraph/TransformMain.hpp>

namespace sdl {
namespace Hypergraph {


struct HypPrune : TransformMain<HypPrune> {
  typedef TransformMain<HypPrune> Base;
  HypPrune() : Base("Prune", USAGE_HypPrune) {}

  PruneOptions pruneOptions;
  Properties properties(int i) const { return kStoreOutArcs; }
  void declare_configurable() { this->configurable(&pruneOptions); }

  enum { has_inplace_input_transform = true, has_transform1 = false };

  bool printFinal() const { return true; }

  static bool nbestHypergraphDefault() { return false; }

  template <class Arc>
  bool inputTransformInplace(IMutableHypergraph<Arc>& hg, int) const {
    pruneUnreachable(hg, pruneOptions);
    return true;
  }
};
}
}

HYPERGRAPH_NAMED_MAIN(Prune)
