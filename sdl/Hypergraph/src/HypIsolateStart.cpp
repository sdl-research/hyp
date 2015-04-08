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
#define USAGE_HypIsolateStart                                                                                \
  "Create the complement (determinize first if necessary) of an unweighted fsa hypergraph -- input symbols " \
  "only"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/IsolateStartState.hpp>

namespace sdl {
namespace Hypergraph {

struct HypIsolateStart : TransformMain<HypIsolateStart> {  // note base class CRTP (google it)
  HypIsolateStart() : TransformMain<HypIsolateStart>("IsolateStart", USAGE_HypIsolateStart) {}
  IsolateStartState iso;
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& h) {
    inplace(h, iso);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(IsolateStart)
