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

    use with MapHypergraph to collect sets of possible ngrams.
*/

#ifndef HYP__HYPERGRAPH_NGRAMWEIGHTMAPPER_HPP
#define HYP__HYPERGRAPH_NGRAMWEIGHTMAPPER_HPP
#pragma once

#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/NgramWeight.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <cstddef>

namespace sdl {
namespace Hypergraph {

/**
   For use in MapHypergraph; sets all weights be of type
   NgramWeight, to keep track of ngram spans.
*/
template <class SourceArc>
struct NgramWeightMapper {

  typedef NgramWeightTpl<typename SourceArc::Weight> Tarweight;
  typedef ArcTpl<Tarweight> TargetArc;

  NgramWeightMapper(IHypergraph<SourceArc> const& hg, std::size_t maxlen) : hg_(hg), maxlen_(maxlen) {}

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
    } else {
      targetArc->setWeight(Tarweight(inputLabel, maxlen_, sourceArc->weight()));
    }

    return targetArc;
  }

  IHypergraph<SourceArc> const& hg_;
  std::size_t maxlen_;
};


}}

#endif
