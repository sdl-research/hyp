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
#define USAGE_HypComplement                                                                                  \
  "Create the complement (determinize first if necessary) of an unweighted fsa hypergraph -- input symbols " \
  "only"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/fs/Complement.hpp>

namespace sdl {
namespace Hypergraph {

struct HypComplement : TransformMain<HypComplement> {  // note base class CRTP (google it)
  HypComplement() : TransformMain<HypComplement>("Complement", USAGE_HypComplement) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {
    fs::complement(i, o);
    return true;
  }
};
}
}

HYPERGRAPH_NAMED_MAIN(Complement)
