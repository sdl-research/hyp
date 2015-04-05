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
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/ReweightBest.hpp>

namespace sdl {
namespace Hypergraph {

struct HypReweightBest : TransformMain<HypReweightBest> {  // CRTP
  typedef TransformMain<HypReweightBest> Base;
  HypReweightBest() : Base("ReweightBest", ReweightOptions::caption()) {}
  void declare_configurable() { this->configurable(&rw.opt); }

  ReweightBest rw;
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& h) {
    rw.inplace(h);
    return true;
  }
  void validate_parameters_more() { rw.opt.validate(); }
};


}}

HYPERGRAPH_NAMED_MAIN(ReweightBest)
