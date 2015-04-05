// Copyright 2014-2015 SDL plc
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
#define USAGE_HypPruneToBest "replace hypergraph with just the nbest paths"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/PruneNonBest.hpp>

namespace sdl {
namespace Hypergraph {

struct HypPruneToBest : TransformMain<HypPruneToBest> {
  typedef TransformMain<HypPruneToBest> Base;
  HypPruneToBest() : Base("PruneToBest", USAGE_HypPruneToBest), opt(1) {}
  void declare_configurable() { this->configurable(&opt); }

  PruneToNbestOptions opt;

  Properties properties(int i) const { return kDefaultProperties | kStoreInArcs; }

  enum { has_inplace_input_transform = true, has_transform1 = false };

  bool printFinal() const { return true; }

  static bool nbestHypergraphDefault() { return false; }

  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc>& hg, int n) const {
    justBest(hg, opt);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(PruneToBest)
