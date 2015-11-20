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

    for MapHypergraph to help you compute tokens for Statistical Tokenizer
*/


#ifndef HYP__HYPERGRAPH_TOKENWEIGHTMAPPER_HPP
#define HYP__HYPERGRAPH_TOKENWEIGHTMAPPER_HPP
#pragma once

#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/TokenWeight.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Returns an arc whose weight is of type TokenWeight; to be
   used with MapHypergraph.

   The head state of the arc is the end of the token. The token
   content is separately determined by the tails of the arc (each tail
   contains its own span, which spans a token).
 */
template <class FromArc>
struct SetTokenWeightMapper {
  typedef typename FromArc::Weight Weight;
  typedef TokenWeightTpl<Weight> TokenWeight;
  typedef ArcTpl<TokenWeight> ToArc;
  ToArc* operator()(FromArc const* arc) const {
    // Token spans from first tail to head:
    StateIdContainer const& tails = arc->tails();
    assert(!tails.empty());
    TokenWeight w(Token::createEmptyToken(tails[0], arc->head()), arc->weight());
    ToArc* toArc = new ToArc(arc->head(), tails, w);
    SDL_DEBUG(Hypergraph.SetTokenWeightMapper, "Created " << *toArc);
    return toArc;
  }
};


}}

#endif
