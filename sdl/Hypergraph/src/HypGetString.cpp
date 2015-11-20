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
#define USAGE_HypGetString \
  "get the single string found by taking the first edge into each node starting from final (TOP)"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/GetString.hpp>

namespace sdl {
namespace Hypergraph {

struct HypGetString : TransformMain<HypGetString> {  // note base class CRTP (google it)
  HypGetString() : TransformMain<HypGetString>("GetString", USAGE_HypGetString) {}
  bool printFinal() const { return false; }
  Properties properties(int) const { return kStoreInArcs; }
  enum { has_transform1 = false, has_inplace_input_transform = 1 };
  template <class Arc>
  bool inputTransformInplace(IMutableHypergraph<Arc>& hg, int lineno) const {
    out() << sdl::Hypergraph::getString(hg) << '\n';
    return true;
  }
};
}
}

HYPERGRAPH_NAMED_MAIN(GetString)



