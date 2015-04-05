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
#define USAGE_HypDraw "Print graphviz (dot) equivalent of hypergraph."
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/HypergraphDrawer.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>

namespace sdl {
namespace Hypergraph {

struct HypDraw : TransformMain<HypDraw> {
  HypDraw() : TransformMain<HypDraw>("HypDraw", USAGE_HypDraw) { opt.allow_ins(); }
  Properties properties(int i) const { return kStoreInArcs; }
  enum { has_transform1 = false, has_transform2 = false, has_inplace_input_transform = true };
  template <class Arc>
  bool inputTransformInPlace(IHypergraph<Arc> const& i, int) {
    drawHypergraph(out(), i) << '\n';
    return true;
  }
  bool printFinal() const { return false; }
};


}}

HYPERGRAPH_NAMED_MAIN(Draw)
