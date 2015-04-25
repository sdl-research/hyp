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
#define SDL_TRANSFORM_MAIN_LOG_WEIGHT 1
#define SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT 1
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Compose.hpp>

namespace sdl {
namespace Hypergraph {

#define USAGE_HypCompose "Compose cfg*fsm*...*fsm"

struct HypCompose : TransformMain<HypCompose> {
  static bool nbestHypergraphDefault() { return false; }  // for backward compat w/ regtests mostly

  HypCompose() : TransformMain<HypCompose>("Compose", USAGE_HypCompose) {
    opt.require_ins();
    composeOpt.addFstOption = false;
    composeOpt.fstCompose = true;
  }

  void declare_configurable() { this->configurable(&composeOpt); }

  static BestOutput bestOutput() { return kBestOutput; }
  static LineInputs lineInputs() { return kNoLineInputs; }

  ComposeTransformOptions composeOpt;

#if 0
  Properties properties(int i) const {  //0 is out, 1 is cfg (or if fst*fst we want outarcs), 2 and on are all fsms
    return i == 1 ? (kComposeCfgRequiredProperties|kFsmOutProperties) : kFsmOutProperties;
  }
#endif

  enum {
    has_transform1 = false,
    has_transform2 = true,
  };

  char const* transform2sep() const { return " * "; }

  template <class Arc>
  bool transform2mm(IMutableHypergraph<Arc>& hg1, IMutableHypergraph<Arc>& hg2,
                    IMutableHypergraph<Arc>* result) {
    ComposeTransform<Arc> c(composeOpt);
    c.setFst(hg2);
    c.inout(hg1, result);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(Compose)
