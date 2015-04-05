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
/** \file

    IHypergraph as fst. unlike an IHypergraph, a fst may have multiple
    (unweighted) final states. I found it wasn't worth the trouble of allowing
    an arbitrary Weight for final since we always have an explicit epsilon arc
    for that in practice.
*/


#ifndef SDL_HG_FS__FST_HPP
#define SDL_HG_FS__FST_HPP
#pragma once

#include <boost/optional.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/GeneratorTransform.hpp>
#include <sdl/SharedPtr.hpp>
#include <sdl/Hypergraph/Level.hpp>

namespace sdl {
namespace Hypergraph {
namespace fs {

template <class Weight>
struct DistanceForFstArc {
  typedef typename Weight::FloatT result_type;
  template <class Arc>
  result_type operator()(Arc const& arc) const {
    return arc.getDistance();
  }
};

/**
   some defaults.
*/
template <class State, class Distance = float>
struct FstBase {
  /**
     a levelization of a hypergraph is a mapping L from state to (>=0) level, such
     that no arc from tail->head has L(tail)>L(head). if there are cycles, then all
     states on them must therefore be on the same level.

     (unless you're sure, just return 0)
  */
  Level level(State const&) { return 0; }
  /**
     should be a (nearly) admissible (underestimate) heuristic for shortest
     distance from state to any final state. if you have no idea, return 0.

     note: if negative distance arcs exist anywhere, then you might need to
     return a negative heuristic in some cases (well, precisely when the best
     path to final is negative, you would need heuristic to be at least that
     negative or you're imadmissible, and LazyBest might not give the true
     1-best

     TODO: for mutable hg, allow sorting arcs by heuristic+weight.getValue()
  */
  Distance heuristic(State const&) { return Distance(); }

  IVocabularyPtr getVocabulary() const {
    SDL_THROW_LOG(fs.Fst, ProgrammerMistakeException, "Unimplemented: lazy fst getVocabulary()");
    return IVocabularyPtr();
  }
};


inline StateId stateId(StateId x) { return x; }

template <class NonStateId>
inline StateId stateId(NonStateId const& x) { return kNoState; }
/**
   IMutableHypergraph<HgArcT> as Fst.
*/
template <class HgArcT>
struct HypergraphFst {
  typedef StateId State;
  typedef HgArcT HgArc;
  typedef typename HgArc::Weight Weight;
  typedef FstArc<Weight, State> Arc;

  typedef typename Weight::FloatT Distance;
  typedef IMutableHypergraph<HgArc> MutableHg;
  typedef IHypergraph<HgArc> ConstHg;
  typedef shared_ptr<ConstHg const> HgPtr;

  HgPtr pHg;
 protected:
  Levels levels; // for beam search (optional)
  Level nLevels;
 public:

  /**
     useful for acyclic hg + beamed best-first search. otherwise, a waste of
     time (everything will be at level 0 for a cyclic hg; no need to compute if
     you already know it's cyclic)
  */
  void computeLevels() {
    Levelization levelize(*pHg);
    nLevels = levelize.numLevels();
    levelize.moveTo(levels);
  }

  Level level(State s) const {
    return levels(s);
  }

  /**
     should be a (nearly) admissible heuristic for shortest distance from state to any final state. if you have no idea, return 0.
  */
  Distance heuristic(State s) const {
    return pHg->heuristic(s);
  }


  /**
     construct from shared_ptr (then you can use this forever) or ref to
     hypergraph (in which case you can use this only as long as that ref is
     valid. you can cast away const to produce the ref, as we won't be modifying
     hg in any case
  */

  explicit HypergraphFst(shared_ptr<MutableHg const> const& pHg, bool annotations = true) { init(pHg, annotations); }
  explicit HypergraphFst(MutableHg const& hg, bool annotations = true) { init(hg, annotations); }
  explicit HypergraphFst(shared_ptr<ConstHg const> const& pHg, bool annotations = true) { needMutable(*pHg); init(pHg, annotations); }
  explicit HypergraphFst(ConstHg const& hg, bool annotations = true) { needMutable(hg); init(hg, annotations); }

 protected:
  HypergraphFst() {}
  void init(shared_ptr<ConstHg const> const& pHg, bool annotations = true) {
    nLevels = 1;
    this->pHg = pHg;
    needFsm();
    arcFn.init(pHg.get(), annotations);
  }
  void init(shared_ptr<MutableHg const> const& pHg, bool annotations = true) {
    init(boost::static_pointer_cast<ConstHg const>(pHg), annotations);
  }
  void init(ConstHg const& hg, bool annotations = true) {
    init(ptrNoDelete(hg), annotations);
  }

  /**
     note: init doesn't call this (since subclasses may not require mutable hg).
  */
  void needMutable(ConstHg const& hg) {
    if (!hg.isMutable())
      throwNeedMutable();
  }
 public:

  ConstHg const& hg() const { return *pHg; }
  bool final(State s) const
  {
    return s == pHg->final();
  }
  State startState() const {
    return pHg->start();
  }
  typedef typename MutableHg::OutArcsGenerator HgArcs;
  typedef typename ConstHg::FstArcFor FstArcFn;
  FstArcFn arcFn;
  typedef typename HgArcs::iterator HgArcIterator;
  typedef Util::TransformedIteratorGenerator<FstArcFn, HgArcIterator> Arcs;

  Arcs outArcs(StateId sid) const {
    assert(sid != kNoState);
    return Arcs(static_cast<MutableHg const&>(*pHg).outArcs(sid), arcFn);
  }

  IVocabularyPtr getVocabulary() const {
    return hg().getVocabulary();
  }
 private:
  void throwNeedMutable() const {
    SDL_THROW_LOG(Hypergraph.fs.HypergraphFst, ConfigException, "can't make HypergraphFst from non-mutable IHypergraph - try ConstHypergraphFst or MutableHypergraph");
  }
  void needFsm() const {
    if (!pHg->isFsmLike()) {
      SDL_DEBUG(Hypergraph.fs.Fst.needFsm, "not fsm-like:\n"<<*pHg);
      SDL_THROW_LOG(Hypergraph.fs.Fst, ConfigException, "in fst compose A*B, A and B must both be fsts (not general hypergraphs)");
    }
  }
};

template <class HG>
HypergraphFst<HG> fstForHg(shared_ptr<HG> const& hg, bool annotations = true) {
  return HypergraphFst<HG>(hg, annotations);
}

template <class HG>
HypergraphFst<HG> fstForHg(HG &hg, bool annotations = true) {
  return HypergraphFst<HG>(hg, annotations);
}


/**
   IHypergraph as Fst. slower because the out arcs generator is type erased
*/
template <class HgArcT>
struct ConstHypergraphFst : HypergraphFst<HgArcT> {
  typedef StateId State;
  typedef HgArcT HgArc;
  typedef typename HgArc::Weight Weight;
  typedef FstArc<Weight, State> Arc;
  typedef typename Weight::FloatT Distance;
  typedef IHypergraph<HgArc> Hg;

  typedef HypergraphFst<HgArc> Base;

  explicit ConstHypergraphFst(shared_ptr<Hg const> const& pHg, bool annotations = true) { this->init(pHg, annotations); }
  explicit ConstHypergraphFst(Hg const& hg, bool annotations = true) { this->init(hg, annotations); }

  typedef typename Hg::ConstOutArcsGenerator HgArcs;
  typedef Util::TransformedGenerator<HgArcs, typename Hg::FstArcFor, FstArc<Weight> > Arcs;
  Arcs outArcs(StateId sid) const {
    return Arcs(this->pHg->outArcsConst(sid), this->arcFn);
  }
};


}}}

#endif
