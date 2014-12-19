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

   axioms here use StateWtFn(stateid). axioms are states with no inarcs (not
   necessarily terminal-labeled only).  FIXME: this is wrong if you delete
   unwanted arcs (because e.g. they have 0 weight) and don't remove the
   unreachable states also
*/
template<class HG
         , class StateWtFn
         , class ArcWtFn = ArcWeight<typename StateWtFn::Weight>
         , class Distances = boost::ptr_vector<typename ArcWtFn::Weight > >
struct ComputeDistanceStatesVisitor
    : IStatesVisitor, private ArcWtFn, private StateWtFn
{
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
  ComputeDistanceStatesVisitor(IHypergraph<Arc> const& hg
                               , Distances &distances
                               , StateWtFn const& stateWtFn = StateWtFn()
                               , ArcWtFn const& arcWtFn = ArcWtFn()
                               , bool cacheStateWt = true)
      : ArcWtFn(arcWtFn)
      , StateWtFn(stateWtFn)
      , kZero(Weight::zero())
      , hg(hg)
      , distances(distances)
      , cacheStateWt_(cacheStateWt)
  {}

  /**
     Computes the distance to a particular state.
  */
  void visit(StateId sid) {
    //topo visit calls for tails first. so all the distances[tail] are already computed.
    std::size_t const cntInArcs = hg.numInArcs(sid);
    if (!cntInArcs && !cacheStateWt_) // see code below: we'll never visit no-in-arcs states.
      return;
    Weight &sum = Util::atExpandPtr(distances, sid, kZero);
    // keeping this reference is ok because there's no recursion in loops below:
    if (!cntInArcs)
      sum = StateWtFn::operator()(sid);
    else {
      for (ArcId aid = 0; aid < cntInArcs; ++aid) {
        Arc const& arc = *hg.inArc(sid, aid);
        Weight const& arcw = ArcWtFn::operator()(&arc); // timesBy at the end because of noncommutative ngram, block weights.
        Weight prod(Weight::one());
        // we start from one here, and add arcw at the very end because we use
        // some noncommutative semirings (block, ngram, token weight)
        StateIdContainer const& tails = arc.tails();
        for (typename StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i!=e; ++i) {
          StateId const tail=*i;
          bool const tailDone = tail < distances.size();
          if (!cacheStateWt_ && !hg.numInArcs(tail)) {
            // necessary to check !inarcs here instead of tailDone in case states weren't sorted but cacheStateWt was false
            Hypergraph::timesBy(StateWtFn::operator()(tail), prod);
          } else if (!tailDone) {
            SDL_THROW_LOG(Hypergraph.InsideAlgorithm, CycleException,
                          "back edge tail=" << tail << " => head=" << sid << " - input was cyclic or wasn't topo-sorted");
          } else {
            Hypergraph::timesBy(distances[tail], prod);
          }
        }
        Hypergraph::timesBy(arcw, prod);
        Hypergraph::plusBy(prod, sum);
      }
    }
  }

 private:
  Weight kZero;
  IHypergraph<Arc> const& hg;
  Distances &distances;
  bool cacheStateWt_;
};

/**
   Runs the inside algorithm. Assumes there are no cycles.

   // state weights are only used for states w/ no inarcs - not
   // exactly the same as what AxiomWeightHypergraph does

   \param maxNonLexical if set is the greatest non-leaf state id
   (i.e. state weights will be used for all states > than this). side
   effect: pDistances doesn't grow larger than maxNonLexical+1
*/
template<class Arc, class StateWtFn, class ArcWtFn, class Distances>
void insideAlgorithm(
    IHypergraph<Arc> const& hg
    , Distances *pDistances
    , StateWtFn const& stateWtFn
    , ArcWtFn const& arcWtFn
    , StateId maxNotTerminal = (StateId)-1)
{
  shared_ptr<IHypergraph<Arc> const> phg = ensureProperties(hg, kStoreInArcs);

  SDL_DEBUG(Hypergraph.InsideAlgorithm,
            "Start inside alg on hypergraph, setting distances for states [0,..."
            <<maxNotTerminal << "]:\n" << hg);

  // Traverse states in topsorted order, and compute distance for each state:
  ComputeDistanceStatesVisitor<Arc, StateWtFn, ArcWtFn, Distances> distanceComputer(
      *phg, *pDistances, stateWtFn, arcWtFn, maxNotTerminal == (StateId)-1);
  TopsortStatesTraversal<Arc>(*phg, &distanceComputer, maxNotTerminal);
}

template<class Arc, class StateWtFn, class Distances>
void insideAlgorithm(
    IHypergraph<Arc> const& hg,
    Distances* pDistances,
    StateWtFn const& stateWtFn)
{
  insideAlgorithm(hg, pDistances, stateWtFn, ArcWeight<typename Arc::Weight>());
}


template<class Arc>
void insideAlgorithm(
    IHypergraph<Arc> const& hg,
    boost::ptr_vector<typename Arc::Weight> *pDistances)
{
  insideAlgorithm(hg, pDistances, OneStateWeight<typename Arc::Weight>());
}


}}

#endif
