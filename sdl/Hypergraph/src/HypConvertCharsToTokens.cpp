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
#define USAGE_HypConvertCharsToTokens \
  "concatenate 'character' tokens between <tok> </tok> into single 'word' tokens"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/ConvertCharsToTokens.hpp>

namespace sdl {
namespace Hypergraph {

struct HypConvertCharsToTokens : TransformMain<HypConvertCharsToTokens> {
  HypConvertCharsToTokens()
      : TransformMain<HypConvertCharsToTokens>("ConvertCharsToTokens", USAGE_HypConvertCharsToTokens) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {
    convertCharsToTokens(i, o);
    return true;
  }
};
}
}

HYPERGRAPH_NAMED_MAIN(ConvertCharsToTokens)



