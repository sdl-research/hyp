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

    lazily transform an input hypergraph's weights
*/

#ifndef HYP__HYPERGRAPH_MAPHYPERGRAPH_HPP
#define HYP__HYPERGRAPH_MAPHYPERGRAPH_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/ConvertWeight.hpp>
#include <sdl/Util/LogHelper.hpp>


#include <utility>  // make_pair
#include <sdl/Util/Unordered.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Maps each arc on demand.
*/
template <class FromA, class ToA, class MapFct>
class MapHypergraph final : public IHypergraph<ToA> {

 public:
  typedef FromA FromArc;
  typedef ToA ToArc;
  typedef MapFct Mapper;
  typedef hash_map<FromArc*, ToArc*> ArcsMap;

  MapHypergraph(IHypergraph<FromA> const& hg, Mapper mapper = Mapper()) : hg_(hg), mapper_(mapper) {
    Util::setEmptyKey(oldArcsToNewArcs_);
    this->start_ = hg.start();
    this->final_ = hg.final();
  }
  ~MapHypergraph() {
    // TODO: memory leak if memoization is turned off
    for (typename ArcsMap::value_type pairOldNewArcs : oldArcsToNewArcs_) {
      ToArc* newArc = pairOldNewArcs.second;
      delete newArc;
      // We're not deleting old arc (pairOldNewArcs.first) because it
      // was created outside of this class.
    }
  }

  /** since arc mapping doesn't affect state labels, defer. */
  bool outputLabelFollowsInput(StateId s) const override { return hg_.outputLabelFollowsInput(s); }

  bool outputLabelFollowsInput() const override { return hg_.outputLabelFollowsInput(); }

  virtual MapHypergraph<FromArc, ToArc, Mapper>* clone() const override {
    return new MapHypergraph<FromArc, ToArc, Mapper>(hg_, mapper_);
  }

  StateId size() const override { return hg_.size(); }

  StateIdRange getStateIds() const { return hg_.getStateIds(); }

  ArcIdRange inArcIds(StateId s) const { return hg_.inArcIds(s); }

  ArcIdRange outArcIds(StateId s) const { return hg_.outArcIds(s); }

  ArcId numInArcs(StateId s) const override { return hg_.numInArcs(s); }

  ArcId numOutArcs(StateId s) const override { return hg_.numOutArcs(s); }

  ToArc* inArc(StateId s, ArcId arcid) const override {
    FromArc* fromArc = hg_.inArc(s, arcid);
    return mappedArc(fromArc);
  }

  ToArc* outArc(StateId s, ArcId arcid) const override {
    FromArc* fromArc = hg_.outArc(s, arcid);
    return mappedArc(fromArc);
  }

  Sym inputLabel(StateId s) const override { return hg_.inputLabel(s); }

  Sym outputLabel(StateId s) const override { return hg_.outputLabel(s); }

  Properties properties() const override { return hg_.properties(); }

  IVocabularyPtr getVocabulary() const override { return hg_.getVocabulary(); }
  IVocabulary* vocab() const override { return hg_.vocab(); }
  Properties uncomputedProperties() const override { return hg_.uncomputedProperties(); }

 private:
  ToArc* mappedArc(FromArc* fromArc) const {
    ToArc* mappedArc = (ToArc*)NULL;
    typename ArcsMap::const_iterator found = oldArcsToNewArcs_.find(fromArc);
    if (found == oldArcsToNewArcs_.end()) {
      mappedArc = mapper_(fromArc);
      oldArcsToNewArcs_.insert(std::make_pair(fromArc, mappedArc));
    } else {
      mappedArc = found->second;
    }
    return mappedArc;
  }

  IHypergraph<FromArc> const& hg_;
  Mapper mapper_;
  mutable ArcsMap oldArcsToNewArcs_;
};

// Mappers

/**
   For use in MapHypergraph; converts arc type.
*/
template <class FromArc, class ToArc>
struct ConvertArcTypeMapper {
  ToArc* operator()(FromArc const* arc) const {
    typedef typename FromArc::Weight FromWeight;
    typedef typename ToArc::Weight ToWeight;
    ToWeight toWeight;
    WeightConverter<FromWeight, ToWeight>(arc->weight(), toWeight);
    return new ToArc(Head(arc->head()), Tails(arc->tails().begin(), arc->tails().end()), toWeight);
  }
};

/**
   For use in MapHypergraph; converts arc type and sets every
   arc to the specified weight.
*/
template <class FromArc, class ToArc>
struct SetWeightMapper {

  typedef typename ToArc::Weight ToWeight;

  SetWeightMapper(ToWeight w) : weight_(w) {}

  ToArc* operator()(FromArc* arc) const {
    ToArc* result = new ToArc(arc->head_, arc->tails_, weight_);
    return result;
  }

  ToWeight weight_;
};

/**
   For use in finite-state MapHypergraph; sets arc weight to
   one if arc label is a word (as opposed to epsilon or other special
   symbols); good for measuring path lengths.
*/
template <class FromArc, class ToArc>
struct LengthMapper {

  typedef typename ToArc::Weight ToWeight;

  LengthMapper(IHypergraph<FromArc> const& hg) : hg_(hg), pVoc_(hg.getVocabulary()) {}

  ToArc* operator()(FromArc* arc) const {
    Sym symid = getFsmInputLabel(hg_, *arc);
    const bool isTerminal = symid.isTerminal();
    const ToWeight weight = isTerminal ? ToWeight(1.0) : ToWeight(0.0);
    ToArc* result = new ToArc(arc->head_, arc->tails_, weight);
    return result;
  }

  IHypergraph<FromArc> const& hg_;
  IVocabularyPtr pVoc_;
};


}}

#endif
