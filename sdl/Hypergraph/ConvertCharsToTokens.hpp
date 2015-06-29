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
/** \file

    Split tokens of input hypergraph into separate character symbols (e.g. for chinese)

    TODO: support block ids for lattice/graph input too (need to go into the Tokens weights)

*/

#ifndef HYP__HYPERGRAPH_CONVERT_CHARS_TO_TOKENS_HPP
#define HYP__HYPERGRAPH_CONVERT_CHARS_TO_TOKENS_HPP
#pragma once

#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Converts a character-based hypergraph into a token-based
   (i.e., word-based) hypergraph. The tokens are determined based on
   the TOK_START and TOK_END chars in the input hypergraph.
*/
template<class Arc>
void convertCharsToTokens(IHypergraph<Arc> const& hgInput,
                          IMutableHypergraph<Arc>* pHgResult);


template <class Arc>
void convertCharsToTokens(IHypergraph<Arc> & hg, IMutableHypergraph<Arc>* pHgResult) {
  forceInArcs(hg);
  convertCharsToTokens((IHypergraph<Arc> const&)hg, pHgResult);
}

/**
   Detokenizes a hypergraph using the <glue>
   markers. Example: The sequence "(<glue>" "foo" "bar" is changed
   to "(foo" "bar".

   \see convertCharsToTokens
*/
template<class Arc>
void detokenize(IHypergraph<Arc> const& hgInput,
                IMutableHypergraph<Arc>* pHgResult);


struct ConvertCharsToTokens : SimpleTransform<ConvertCharsToTokens, Transform::Inout>, TransformOptionsBase {
  ConvertCharsToTokens() {}
  explicit ConvertCharsToTokens(TransformOptionsBase const& base) {}
  template <class Arc>
  void inout(IHypergraph<Arc> const& inHg, IMutableHypergraph<Arc> *pOutHg) const {
    convertCharsToTokens(inHg, pOutHg);
  }
  static char const* type() {
    return "ConvertCharsToTokens";
  }
};


}}

#include <sdl/Hypergraph/src/ConvertCharsToTokens.ipp>

#endif
