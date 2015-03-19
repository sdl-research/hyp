/** \file

    compute inside, outside weights for states in hg. inside(st)*outside(st) =
    sum of weights of all trees (start/axioms)->st->final

    TODO: speed up inside-then-outside for hg; currently results in examining in
    arcs 3 times and out arcs 2 times; could do better with 1 time each (by not
    using StatesTraversal)
*/


#ifndef HYP__HYPERGRAPH_OUTSIDE_ALGORITHM_HPP
#define HYP__HYPERGRAPH_OUTSIDE_ALGORITHM_HPP
#pragma once

#include <vector>
#include <cassert>
#include <sdl/Util/Add.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/StatesTraversal.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace sdl {
namespace Hypergraph {

/**
   sets sum the outside score (with final as the root) for sid, assuming the
   outside scores of all the heads of outarcs of sid are already known (and all
   the inside scores, of course). assumes commutative Weight

   insideScores needs to be generated for all states, not just the nonlexical
   ones (i.e. enable includingLeaves)
*/
template <class Arc>
void outsideFromInside(StateId stateId
                       , IHypergraph<Arc> const& hg
                       , boost::ptr_vector<typename Arc::Weight> const& insideScores
                       , boost::ptr_vector<typename Arc::Weight> const& outsideScores
                       , typename Arc::Weight &sum
                       , StateId final
                       , bool haveInsideForAxiom = true)
{
  SDL_TRACE(Hypergraph.OutsideAlgorithm,
            "Computing outside score for " << stateId << ", sum=" << sum);
  typedef typename Arc::Weight Weight;
  if (stateId == final)
    sum = Weight::one();
  else {
    forall (ArcId aid, hg.outArcIds(stateId)) {
      Arc const& arc = *hg.outArc(stateId, aid);
      SDL_TRACE(Hypergraph.OutsideAlgorithm, " Found out arc: " << arc);
      Weight prod(times(arc.weight(), outsideScores[arc.head()]));
      // normally we say that arc weight comes after tails, but outside is only
      // meaningful for commutative anyway (granted, you could instead have a
      // pair for each state: (L, R) where L are the left-outside and R are the
      // right-outside)
      StateIdContainer const& tails = arc.tails();
      unsigned nself = 0;
      for(StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
        StateId const tail = *i;
        if (tail == stateId && !nself)
          ++nself;
        else if (haveInsideForAxiom || !hg.isAxiom(tail)) {
          assert(tail < insideScores.size());
          timesBy(insideScores[tail], prod);
        }
      }
      SDL_TRACE(Hypergraph.OutsideAlgorithm, " prod=" << prod << " (add #self="<<nself<<" times)");
      while (nself--)
        plusBy(prod, sum);
    }
  }
  SDL_TRACE(Hypergraph.OutsideAlgorithm, " sum now " << sum);
}

/**
   A states visitor that computes the distance to each particular state
   that it's called with
*/
template<class Arc>
struct ComputeOutsideScoreStatesVisitor : public IStatesVisitor {

  typedef typename Arc::Weight Weight;

  ComputeOutsideScoreStatesVisitor(IHypergraph<Arc> const& hg,
                                   boost::ptr_vector<Weight> const& insideScores,
                                   boost::ptr_vector<Weight>* outsideScores,
                                   bool haveInsideForAxiom)
      : hg_(hg)
      , final(hg.final())
      , insideScores_(insideScores)
      , outsideScores_(outsideScores)
      , kZero(Weight::zero())
      , haveInsideForAxiom_(haveInsideForAxiom)
  {}

  /**
     Computes the outside score of a particular state.
  */
  void visit(StateId stateId) {
    assert(outsideScores_);
    Weight &outside = Util::atExpandPtr(*outsideScores_, stateId, kZero);
    outsideFromInside(stateId, hg_, insideScores_, *outsideScores_, outside, final, haveInsideForAxiom_);
    SDL_TRACE(Hypergraph.InsideAlgorithm,
              "Stored outside distance: " << (*outsideScores_)[stateId] << " to state " << stateId);
  }

  Weight const kZero;
  IHypergraph<Arc> const& hg_;
  StateId final;
  boost::ptr_vector<Weight> const& insideScores_;
  boost::ptr_vector<Weight>* outsideScores_;
  bool haveInsideForAxiom_;
};

/**
   Runs the outside algorithm. Assumes there are no cycles.

   \param outsideScore The resulting vector that will be filled

   \param insideScores A vector of inside scores (unused if HG is an
   FSM)
*/
template<class Arc>
void outsideAlgorithm(IHypergraph<Arc> const& hg,
                      boost::ptr_vector<typename Arc::Weight> const& insideScores,
                      boost::ptr_vector<typename Arc::Weight>* outsideScores, bool haveInsideForAxiom = true) {

  // Traverse states in reverse topsorted order (i.e., starting from
  // FINAL root), and compute outsideScore for each state:
  ComputeOutsideScoreStatesVisitor<Arc> outsideScoreComputer(
      hg, insideScores, outsideScores, haveInsideForAxiom);
  ReverseTopsortStatesTraversal<Arc>(hg, &outsideScoreComputer);
}


}}

#endif
