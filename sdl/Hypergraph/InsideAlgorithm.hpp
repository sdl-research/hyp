// Copyright 2014 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**
   \file

   Inside algorithm function and helper class.

   \author Markus Dreyer
*/

#ifndef HYP__HYPERGRAPH_INSIDE_ALGORITHM_HPP
#define HYP__HYPERGRAPH_INSIDE_ALGORITHM_HPP
#pragma once

#include <vector>

#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/ArcWeight.hpp>
#include <sdl/Hypergraph/StatesTraversal.hpp>
#include <sdl/Hypergraph/ArcWeight.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Add.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace sdl {
namespace Hypergraph {

/**
   A states visitor that computes the distance to each
   particular state that it's called with.

   axioms here use StateWtFn(stateid). axioms are terminal-labeled states and
   the start state if it exists.
*/
template <class HG, class StateWtFn, class ArcWtFn = ArcWeight<typename StateWtFn::Weight>,
          class Distances = boost::ptr_vector<typename ArcWtFn::Weight>, bool IncludingAxioms = false>
struct ComputeDistanceStatesVisitor : IStatesVisitor, private ArcWtFn, private StateWtFn {
  typedef typename ArcWtFn::Weight Weight;
  typedef HG InputHypergraph;
  typedef typename HG::Arc Arc;

  /**
     \param distances The resulting distances (must already be
     constructed, will be resized if necessary and filled).
     *

     \param axiomsWeights Specify your own axiom weights, @see
     ArcWeight.

     \param cacheStateWt true => if stateWtFn is expensive, expand the storage for
     'distances' to include no-inarc (leaf) states. false => recompute
     stateWtFn(leaf) and don't store it in distances.
  */
  ComputeDistanceStatesVisitor(HG const& hg, Distances& distances, StateWtFn const& stateWtFn = StateWtFn(),
                               ArcWtFn const& arcWtFn = ArcWtFn(), bool useStateWt = true)
      : ArcWtFn(arcWtFn)
      , StateWtFn(stateWtFn)
      , kZero(Weight::zero())
      , hg_(hg)
      , distances_(distances)
      , maxNonAxiom_(0)
  {}

  /**
     Computes the distance to a particular state.
  */
  void visit(StateId sid) {
    // topo visit calls for tails first. so all the distances_[tail] are already computed.
    std::size_t const cntInArcs = hg_.numInArcs(sid);
    bool axiom = hg_.isAxiom(sid);
    Weight& sum = Util::atExpandPtr(distances_, sid, kZero);
    // keeping this reference is ok because there's no recursion in loops below:
    if (axiom) {
      setOne(sum);
      return;
    }
    if (!IncludingAxioms && sid > maxNonAxiom_)
      maxNonAxiom_ = sid;
    for (ArcId aid = 0; aid < cntInArcs; ++aid) {
      Arc const& arc = *hg_.inArc(sid, aid);
      Weight prod(Weight::one());
      // we start from one here, and add arcw at the very end because we use
      // some noncommutative semirings (block, ngram, token weight)
      StateIdContainer const& tails = arc.tails();
      for (typename StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
        StateId const tail = *i;
        if (hg_.isAxiom(tail)) {
          StateWtFn::timesByState(tail, prod);
          SDL_TRACE(FinalOutput.InsideAlgorithm, "for " << sid << " from " << tail << " prod = " << prod
                                                        << " = old * " << StateWtFn::operator()(tail));
        } else if (tail >= distances_.size()) {
          SDL_THROW_LOG(Hypergraph.InsideAlgorithm, CycleException,
                        "back edge tail=" << tail << " => head=" << sid
                                          << " - input was cyclic or wasn't topo-sorted");
        } else {
          Hypergraph::timesBy(distances_[tail], prod);
          SDL_TRACE(FinalOutput.InsideAlgorithm, "for " << sid << " from " << tail << " prod = " << prod
                                                        << " = old * " << distances_[tail]);
        }
      }
      ArcWtFn::timesBy(&arc, prod);
      // arcw has to come after the states for non-commutative e.g. SingleBlockWeight
      SDL_TRACE(FinalOutput.InsideAlgorithm, "after arc wt for " << Util::print(arc, hg_) << " for " << sid
                                                                 << " prod = " << prod);

      Hypergraph::plusBy(prod, sum);
      SDL_TRACE(FinalOutput.InsideAlgorithm, "for " << sid << " sum " << sum << " = old + " << prod);
    }
  }

  ~ComputeDistanceStatesVisitor() {
    if (!IncludingAxioms)
      distances_.resize(maxNonAxiom_ + 1);
  }

 private:
  Weight kZero;
  HG const& hg_;
  Distances& distances_;
  StateId maxNonAxiom_;
};

/**
   Runs the inside algorithm. Assumes there are no cycles.

   // state weights are only used for states w/ no inarcs - not
   // exactly the same as what AxiomWeightHypergraph does

   \param maxNonLexical if set is the greatest non-leaf state id
   (i.e. state weights will be used for all states > than this). side
   effect: pDistances doesn't grow larger than maxNonLexical+1
*/
template <bool IncludingAxioms, class Arc, class StateWtFn, class ArcWtFn, class Distances>
void insideAlgorithmWithAxioms(IHypergraph<Arc> const& hg, Distances* pDistances, StateWtFn const& stateWtFn,
                               ArcWtFn const& arcWtFn, StateId maxNotTerminal = (StateId)-1) {
  typedef IHypergraph<Arc> HG;
  shared_ptr<HG const> phg = ensureProperties(hg, kStoreInArcs);

  SDL_DEBUG(Hypergraph.InsideAlgorithm, "Start inside alg on hypergraph, setting distances for states [0,..."
                                        << maxNotTerminal << "]:\n" << hg);

  // Traverse states in topsorted order, and compute distance for each state:
  ComputeDistanceStatesVisitor<HG, StateWtFn, ArcWtFn, Distances, IncludingAxioms> distanceComputer(
      *phg, *pDistances, stateWtFn, arcWtFn);
  TopsortStatesTraversal<Arc>(*phg, &distanceComputer, maxNotTerminal);
}

template <class Arc, class StateWtFn, class ArcWtFn, class Distances>
void insideAlgorithmMaybeAxioms(bool includingAxioms, IHypergraph<Arc> const& hg, Distances* pDistances,
                                StateWtFn const& stateWtFn, ArcWtFn const& arcWtFn,
                                StateId maxNotTerminal = (StateId)-1) {
  if (includingAxioms)
    insideAlgorithmWithAxioms<true, Arc, StateWtFn, ArcWtFn, Distances>(hg, pDistances, stateWtFn, arcWtFn,
                                                                        maxNotTerminal);
  else
    insideAlgorithmWithAxioms<false, Arc, StateWtFn, ArcWtFn, Distances>(hg, pDistances, stateWtFn, arcWtFn,
                                                                         maxNotTerminal);
}

template <class Arc, class StateWtFn, class ArcWtFn, class Distances>
void insideAlgorithmIncludingAxioms(IHypergraph<Arc> const& hg, Distances* pDistances,
                                    StateWtFn const& stateWtFn, ArcWtFn const& arcWtFn,
                                    StateId maxNotTerminal = (StateId)-1) {
  insideAlgorithmMaybeAxioms(true, hg, pDistances, stateWtFn, arcWtFn, maxNotTerminal);
}

template <class Arc, class StateWtFn, class ArcWtFn, class Distances>
void insideAlgorithm(IHypergraph<Arc> const& hg, Distances* pDistances, StateWtFn const& stateWtFn,
                     ArcWtFn const& arcWtFn, StateId maxNotTerminal = (StateId)-1) {
  insideAlgorithmMaybeAxioms(false, hg, pDistances, stateWtFn, arcWtFn, maxNotTerminal);
}

template <class Arc, class StateWtFn, class Distances>
void insideAlgorithm(IHypergraph<Arc> const& hg, Distances* pDistances, StateWtFn const& stateWtFn,
                     bool includingAxioms) {
  insideAlgorithmMaybeAxioms(includingAxioms, hg, pDistances, stateWtFn, ArcWeight<typename Arc::Weight>());
}


template <class Arc>
void insideAlgorithm(IHypergraph<Arc> const& hg, boost::ptr_vector<typename Arc::Weight>* pDistances,
                     bool includingAxioms = false) {
  insideAlgorithm(hg, pDistances, OneStateWeight<typename Arc::Weight>(), includingAxioms);
}


}}

#endif
