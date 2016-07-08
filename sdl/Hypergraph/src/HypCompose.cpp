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
#define SDL_TRANSFORM_MAIN_LOG_WEIGHT 1
#define SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT 1
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/Compose.hpp>
#include <sdl/Hypergraph/TransformMain.hpp>

namespace sdl {
namespace Hypergraph {

#define USAGE_HypCompose "Compose cfg*fsm*...*fsm. you may use --properties=first-tail-out-arcs if not cfg."

struct HypCompose : TransformMain<HypCompose> {
  static bool nbestHypergraphDefault() { return false; }  // for backward compat w/ regtests mostly

  HypCompose() : TransformMain<HypCompose>("Compose", USAGE_HypCompose) {
    composeOpt.addFstOption = false;
    composeOpt.fstCompose = true;
    this->configureProperties = true;
  }

  void declare_configurable() { this->configurable(&composeOpt); }

  static BestOutput bestOutput() { return kBestOutput; }
  static LineInputs lineInputs() { return kNoLineInputs; }

  // 1 is first input which may be cfg. if cfg composing then need outarcs.
  Properties properties(int i) const { return i == 1 ? (kStoreInArcs | kStoreOutArcs) : kStoreOutArcs; }

  ComposeTransformOptions composeOpt;

  enum {
    has_transform1 = false,
    has_transform2 = true,
  };

  char const* transform2sep() const { return " * "; }

  template <class Arc>
  bool transform2mm(IMutableHypergraph<Arc>& hg1, IMutableHypergraph<Arc>& hg2, IMutableHypergraph<Arc>* result) {
    ComposeTransform<Arc> c(composeOpt);
    c.setFst(hg2);
    c.inout(hg1, result);
    return true;
  }
};
}
}

HYPERGRAPH_NAMED_MAIN(Compose)
