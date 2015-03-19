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
template<class FromArc>
struct SetTokenWeightMapper {
  typedef typename FromArc::Weight Weight;
  typedef TokenWeightTpl<Weight> TokenWeight;
  typedef ArcTpl<TokenWeight> ToArc;
  ToArc* operator()(FromArc const* arc) const {
    assert(arc->tails().size() > 0);
    // Token spans from first tail to head:
    TokenWeight w(Token::createEmptyToken(arc->getTail(0), arc->head()),
                  arc->weight());
    ToArc* toArc = new ToArc(arc->head(), arc->tails(), w);
    SDL_DEBUG(Hypergraph.SetTokenWeightMapper, "Created " << *toArc);
    return toArc;
  }
};


}}

#endif
