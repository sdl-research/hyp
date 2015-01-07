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
#define TRANSFORM HgFct

#define USAGE "Debugging binary, for internal use."
#define VERSION "v1"

#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>

namespace sdl {
namespace Hypergraph {

template <class Arc>
void testFct(IMutableHypergraph<Arc> &h) {
}

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)

  typedef TransformMain<TRANSFORM> Base;

  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}

  Properties properties(int i) const {
    return kDefaultProperties | kStoreInArcs | kStoreOutArcs;
  }

  bool printFinal() const {
    return false;
  }

  enum { has_inplace_transform1 = true };

  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    testFct(h);
    return true;
  }
};


}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
