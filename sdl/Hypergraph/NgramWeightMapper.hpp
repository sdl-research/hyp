/** \file

    use with MapHypergraph to collect sets of possible ngrams.
*/

#ifndef HYP__HYPERGRAPH_NGRAMWEIGHTMAPPER_HPP
#define HYP__HYPERGRAPH_NGRAMWEIGHTMAPPER_HPP
#pragma once

#include <cstddef>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/NgramWeight.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>

namespace sdl {
namespace Hypergraph {

/**
   For use in MapHypergraph; sets all weights be of type
   NgramWeight, to keep track of ngram spans.
*/
template<class SourceArc>
struct NgramWeightMapper {

  typedef NgramWeightTpl<typename SourceArc::Weight> Tarweight;
  typedef ArcTpl<Tarweight> TargetArc;

  NgramWeightMapper(IHypergraph<SourceArc> const& hg,
                    std::size_t maxlen)
      : hg_(hg), maxlen_(maxlen) {}

  TargetArc* operator()(SourceArc const* sourceArc) const {
    assert(sourceArc->isFsmArc());

    TargetArc* targetArc = new TargetArc(sourceArc->head(), sourceArc->tails());
    // sets the n-gram (unigram) on the arc. when these arcs are
    // combined in dynamic programming, the unigrams combine into
    // bigrams, trigrams etc.
    StateId labelStateId = sourceArc->getTail(1);
    Sym inputLabel = hg_.inputLabel(labelStateId);
    if (inputLabel == EPSILON::ID) {
      targetArc->setWeight(Tarweight::one());
    }
    else {
      targetArc->setWeight(
          Tarweight(inputLabel, maxlen_, sourceArc->weight()));
    }

    return targetArc;
  }

  IHypergraph<SourceArc> const& hg_;
  std::size_t maxlen_;

};


}}

#endif
