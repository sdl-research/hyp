// Copyright 2014 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#define USAGE_HypSamplePath                                                                               \
  "Generate sample path from hypergraph. Currently, this binary only supports uniform sampling (but C++ " \
  "library allows more options)."
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/SamplePath.hpp>

namespace sdl {
namespace Hypergraph {

struct HypSamplePath : TransformMain<HypSamplePath> {
  HypSamplePath() : TransformMain<HypSamplePath>("SamplePath", USAGE_HypSamplePath) {}

  Properties properties(int i) const {  // i=0 means output
    return kStoreInArcs;  // we'll sample top down
  }

  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {
    UniformInArcSampler<Arc> sampler;
    samplePath(i, sampler, o);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(SamplePath)
