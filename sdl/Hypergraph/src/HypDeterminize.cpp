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
#define USAGE_HypDeterminize "Determinize an unweighted FSA hypergraph -- input symbols only. TODO: support sigma, phi, weights, outputs."
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Determinize.hpp>
#include <sdl/Hypergraph/DeterminizeOptions.hpp>

namespace sdl {
namespace Hypergraph {

struct HypDeterminize : TransformMain<HypDeterminize> { // note base class CRTP (google it)
  HypDeterminize() : TransformMain<HypDeterminize>("Determinize", USAGE_HypDeterminize)
  {}
  DeterminizeOptions detOpt;
  void declare_configurable() {
    this->configurable(&detOpt);
  }

  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    determinize(i, o, detOpt.getFlags());
    return true;
  }
};

}}

HYPERGRAPH_NAMED_MAIN(Determinize)
