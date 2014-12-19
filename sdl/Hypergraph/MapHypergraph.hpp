/** \file

    lazily transform an input hypergraph's weights
*/

#ifndef HYP__HYPERGRAPH_MAPHYPERGRAPH_HPP
#define HYP__HYPERGRAPH_MAPHYPERGRAPH_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/ConvertWeight.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/Override.hpp>

#include <utility>  // make_pair
#include <boost/unordered_map.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Maps each arc on demand.
*/
template <class FromA, class ToA, class MapFct>
class MapHypergraph SDL_FINAL : public IHypergraph<ToA> {

 public:
  typedef FromA FromArc;
  typedef ToA ToArc;
  typedef MapFct Mapper;
  typedef boost::unordered_map<FromArc*, ToArc*> ArcsMap;

  MapHypergraph(IHypergraph<FromA> const& hg, Mapper mapper = Mapper()) : hg_(hg), mapper_(mapper) {
    this->setStartFinalFrom(hg);
  }

  ~MapHypergraph() {
    // TODO: memory leak if memoization is turned off
    forall (typename ArcsMap::value_type pairOldNewArcs, oldArcsToNewArcs_) {
      ToArc* newArc = pairOldNewArcs.second;
      delete newArc;
      // We're not deleting old arc (pairOldNewArcs.first) because it
      // was created outside of this class.
    }
  }

  /** since arc mapping doesn't affect state labels, defer. */
  bool outputLabelFollowsInput(StateId sid) const OVERRIDE { return hg_.outputLabelFollowsInput(sid); }

  bool outputLabelFollowsInput() const OVERRIDE { return hg_.outputLabelFollowsInput(); }

  virtual MapHypergraph<FromArc, ToArc, Mapper>* clone() const OVERRIDE {
    return new MapHypergraph<FromArc, ToArc, Mapper>(hg_, mapper_);
  }

  StateId size() const OVERRIDE { return hg_.size(); }

  StateIdRange getStateIds() const { return hg_.getStateIds(); }

  ArcIdRange inArcIds(StateId sid) const { return hg_.inArcIds(sid); }

  ArcIdRange outArcIds(StateId sid) const { return hg_.outArcIds(sid); }

  ArcId numInArcs(StateId sid) const OVERRIDE { return hg_.numInArcs(sid); }

  ArcId numOutArcs(StateId sid) const OVERRIDE { return hg_.numOutArcs(sid); }

  ToArc* inArc(StateId sid, ArcId aid) const OVERRIDE {
    FromArc* fromArc = hg_.inArc(sid, aid);
    return mappedArc(fromArc);
  }

  ToArc* outArc(StateId sid, ArcId aid) const OVERRIDE {
    FromArc* fromArc = hg_.outArc(sid, aid);
    return mappedArc(fromArc);
  }

  Sym inputLabel(StateId sid) const OVERRIDE { return hg_.inputLabel(sid); }

  Sym outputLabel(StateId sid) const OVERRIDE { return hg_.outputLabel(sid); }

  Properties properties() const OVERRIDE { return hg_.properties(); }

  IVocabularyPtr getVocabulary() const OVERRIDE { return hg_.getVocabulary(); }

 private:
  ToArc* mappedArc(FromArc* fromArc) const {
    ToArc* mappedArc = (ToArc*)NULL;
    typename ArcsMap::const_iterator found = oldArcsToNewArcs_.find(fromArc);
    if (found == oldArcsToNewArcs_.end()) {
      mappedArc = mapper_(fromArc);
      oldArcsToNewArcs_.insert(std::make_pair(fromArc, mappedArc));
    } else { mappedArc = found->second; }
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
    ToArc* result = new ToArc(Head(arc->head()), Tails(arc->tails().begin(), arc->tails().end()), weight_);
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
    ToArc* result = new ToArc(Head(arc->head()), Tails(arc->tails().begin(), arc->tails().end()), weight);
    return result;
  }

  IHypergraph<FromArc> const& hg_;
  IVocabularyPtr pVoc_;
};


}}

#endif
