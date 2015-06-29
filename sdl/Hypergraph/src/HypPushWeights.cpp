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
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/PushWeights.hpp>

namespace sdl {
namespace Hypergraph {

struct HypPushWeights : TransformMain<HypPushWeights> {  // CRTP
  typedef TransformMain<HypPushWeights> Base;
  HypPushWeights() : Base(PushWeights::type(), PushWeights::caption()) {
    this->configureProperties = true;
  }
  void declare_configurable() { this->configurable(&x); }

  Properties properties(int i) const { return this->properties_else(x.inAddProps()); }
  PushWeights x;
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1Inplace(IMutableHypergraph<Arc>& h) {
    x.inplace(h);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(PushWeights)
