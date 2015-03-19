/** \file

    used to treat a LogWeight as a ViterbiWeight or vice versa (so
    FeatureExpectations can use the right insideAlgorithm semiring)
*/

#ifndef HYP__HYPERGRAPH_CAST_HYPERGRAPH_HPP
#define HYP__HYPERGRAPH_CAST_HYPERGRAPH_HPP
#pragma once

#include <sdl/SharedPtr.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/IVocabulary.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Casts each arc on demand to a different arc type. Arc types
   must be convertible using reinterpret_cast (true in typical
   cases). This is the most efficient way to use a hypergraph with a
   different semiring (e.g., summing instead of max'ing by casting
   from ViterbiArc to LogArc).
*/
template <class FromA, class ToA>
class CastHypergraph SDL_FINAL : public IHypergraph<ToA> {

 public:
  typedef FromA FromArc;
  typedef ToA ToArc;

  CastHypergraph(IHypergraph<FromA> const& hg) : hg_(hg) { this->setStartFinalFrom(hg); }

  virtual CastHypergraph<FromArc, ToArc>* clone() const OVERRIDE {
    return new CastHypergraph<FromArc, ToArc>(hg_);
  }

  /**
     Casts the arc using reinterpret_cast and returns it.
  */
  virtual ToArc* inArc(StateId sid, ArcId aid) const OVERRIDE {
    return reinterpret_cast<ToArc*>(hg_.inArc(sid, aid));
  }

  /**
     Casts the arc using reinterpret_cast and returns it.
  */
  virtual ToArc* outArc(StateId sid, ArcId aid) const OVERRIDE {
    return reinterpret_cast<ToArc*>(hg_.outArc(sid, aid));
  }

  // Simple delegation:
  ArcId numInArcs(StateId sid) const OVERRIDE { return hg_.numInArcs(sid); }
  ArcId numOutArcs(StateId sid) const OVERRIDE { return hg_.numOutArcs(sid); }
  virtual StateId size() const OVERRIDE { return hg_.size(); }
  virtual Sym inputLabel(StateId sid) const OVERRIDE { return hg_.inputLabel(sid); }
  virtual Sym outputLabel(StateId sid) const OVERRIDE { return hg_.outputLabel(sid); }
  virtual Properties properties() const OVERRIDE { return hg_.properties(); }
  virtual IVocabularyPtr getVocabulary() const OVERRIDE { return hg_.getVocabulary(); }

  /** since arc mapping doesn't affect state labels, defer. */
  bool outputLabelFollowsInput(StateId sid) const OVERRIDE { return hg_.outputLabelFollowsInput(sid); }

  bool outputLabelFollowsInput() const OVERRIDE { return hg_.outputLabelFollowsInput(); }

  IHypergraph<FromArc> const& hg_;
};


}}

#endif
