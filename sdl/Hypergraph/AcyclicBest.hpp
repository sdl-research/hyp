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
/** \file

    1-best for a lattice (acyclic graph) - works fine w/ negative costs.

    a similar acyclic-hypergraph version based on Level.hpp would be pretty simple
*/

#ifndef HYPERGRAPH__ACYCLIC_BEST_JG_2013_06_10_HPP
#define HYPERGRAPH__ACYCLIC_BEST_JG_2013_06_10_HPP
#pragma once

#include <sdl/Hypergraph/InArcs.hpp>
#include <sdl/Hypergraph/HypergraphTraits.hpp>
// traits abstraction that essentially is just telling us to add getValue() in
// the ViterbiWeight sense when we compute best paths. used in honor of
// BestPath.hpp. in theory it's more configurable.

namespace sdl { namespace Hypergraph {

/**
   Mu, Pi are property_maps. see BestPath.hpp.
*/
template <class Arc, class Mu, class Pi, bool IsGraph = true>
struct AcyclicBest {
  typedef graehl::path_traits<IHypergraph<Arc> > path_traits;

  /**
     mu[StateId] = best cost from start/axioms to state. property_map so by value
  */
  Mu mu;
  // mu[leaf] doesn't matter in this algorithm. (for tails_up_hypergraph i
  // believe we set it to 0). we don't set it to 0 here

  typedef typename Mu::value_type Cost;
  /**
     pi[StateId] = best cost from start/axioms to state. property_map so by value
  */
  Pi pi;
  IHypergraph<Arc> const& hg;
  IMutableHypergraph<Arc> const* hgAsMutable;

  std::size_t back_edges_, self_loops_; // == 0 iff acyclic

  void push_back(StateId st) {
    //top-down. iterate over inarcs
    visitInArcs(*this, st, hg, hgAsMutable);
  }

  enum { checkUnreachable = false };

  void improve(Arc *arc, StateId head) {
    Cost &bestCost = mu[head];
    Cost w = arc->weight().getValue();
    StateIdContainer const& tails = arc->tails();
    if (IsGraph) {
      TailId const tail = tails[0];
      if (tail == head) ++self_loops_;
      Cost const tailCost = mu[tail];
      // maybe count back edges; debug about first one only
      if (checkUnreachable && tailCost == path_traits::unreachable() &&
          tail!=head && !back_edges_++) {
        SDL_DEBUG(Hypergraph.AcyclicBest, "back edge tail=" << tail << " of: "<<*arc);
      }
      path_traits::extendBy(tailCost, w);
    } else {
      for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i!=e; ++i) {
        TailId const tail = *i;
        if (tail == head) ++self_loops_;
        Cost const tailCost = mu[tail];
        if (checkUnreachable && tailCost == path_traits::unreachable() &&
            tail!=head && !back_edges_++) {
          SDL_DEBUG(Hypergraph.AcyclicBest, "HG back edge tail=" << tail << " of: "<<*arc);
        }
        path_traits::extendBy(tailCost, w);
      }
    }
    SDL_TRACE(Hypergraph.AcyclicBest, "arc "<<*arc << " with cost " << bestCost << " vs. previous " << bestCost);
    if (path_traits::better(w, bestCost)) {
      bestCost = w;
      put(pi, head, (ArcHandle)arc);
      SDL_TRACE(Hypergraph.AcyclicBest, "improved head " << head << ": new mu=" << get(mu, head) << " new pi=" << get(pi, head) << " arc: " << (Arc *)get(pi, head));
    }
  }

  // for top-down (in-arcs)
  void acceptIn(Arc *arc, StateId head) {
    improve(arc, head);
  }

  void acceptOut(Arc *arc, StateId) {
    improve(arc, arc->head());
  }

  /// number of tails needing init (for graph, we only need up to start state and max non-terminal)
  StateId muStates_;
  /// one more than the highest pi we set (needs resetting for best-first best-path after, in case we got too many back edges)
  StateId piStates_;

  /**
     compute mu, pi for states reachable from start. caller already initialized
     these - mu to unreachable, pi to NULL (see BestPath)

     terminates early if # back edges exceeds maxBackEdges (leaving mu/pi in
     their partially completed state)
  */
  AcyclicBest(IHypergraph<Arc> const& hg, Mu mu, Pi pi, std::size_t maxBackEdges)
      : mu(mu)
      , pi(pi)
      , hg(hg)
      , hgAsMutable()
  {
    muStates_ = hg.size();
    piStates_ = muStates_;

    bool const outArcs = hg.storesOutArcs();
    bool const useOutArcs = IsGraph && outArcs;

    if (!useOutArcs && !hg.storesInArcs())
      SDL_THROW_LOG(Hypergraph.AcyclicBest, ConfigException, "must store inarcs unless your hypergraph is a simple graph");

    /*
      TODO: allow !IsGraph; use reached-tails approach of e.g. Hypergraph/Level.hpp or
      Hypergraph/Empty.hpp. see e.g. S <- NP VP; NP <- x; VP <- y NP; the VP gets
      popped before NP in InArcs.
    */
    assert(IsGraph);

    if (IsGraph) {
      StateId start = hg.start();
      if (start == kNoState)
        SDL_THROW_LOG(Hypergraph.AcyclicBest, EmptySetException, "your simple graph had no start state (and so no paths)");
    }

    initMu();

    if (hg.isMutable())
      hgAsMutable = static_cast<IMutableHypergraph<Arc> const *>(&hg);

    typedef typename InArcs<Arc>::ArcsContainer ArcsContainer;

    // topo sort (top-down) without stack-killing recursion.


    back_edges_ = 0;
    self_loops_ = 0;
    // we wouldn't need to track this except we choose to set the kAcyclic
    // property and self-loops technically violate it (perhaps we could redefine
    // kAcyclic to spare the effort)

    if (useOutArcs) {
      // essentially two different-order algorithms for viterbi. one iterates
      // over in arcs the other out. both are correct for acyclic. this is all
      // to avoid requiring storing in *and* out arcs - either is fine
      FirstTailOutArcs<Arc> adj(hg);

      std::vector<StateId> orderReverse;
      StateId N = hg.size();
      orderReverse.reserve(N);
      back_edges_ = adj.orderTailsLast(orderReverse, maxBackEdges, IsGraph);

      if (orderReverse.empty()) return;
      typedef StateId const* I;
      for (I i = &orderReverse.back(), last = &orderReverse.front(); ; ) {
        StateId tail = *i;
        visitOutArcs(*this, tail, hg, hgAsMutable);
        if (i == last) break;
        --i;
      }
    } else {
      InArcs<Arc> adj(hg);
      if (IsGraph)
        back_edges_ = adj.orderHeadsLast(*this, maxBackEdges, IsGraph);
      else
        SDL_THROW_LOG(AcyclicBest, ProgrammerMistakeException, "hg acyclic best: //TODO@JG");
    }
    if (acyclic() && hgAsMutable)
      const_cast<IMutableHypergraph<Arc> *>(hgAsMutable)->addProperties(kAcyclic);
  }

  void initMu() {
    for (StateId i = 0, N = muStates_; i<N; ++i) {
      put(mu, i,
          hg.isAxiom(i) ? path_traits::start() : path_traits::unreachable());
      // initial best-cost[state] is axiom(state)? 0 : infinity
      assert(pi[i] == 0);
    }
  }

  void reserve(std::size_t nState) {}

  void resetPi() {
    for (StateId i = 0, N = piStates_; i<N; ++i)
      pi[i] = 0;
  }


  typedef std::vector<StateId> Order;
  Order order_; //TODO: can leave this empty if we already have sorted states

  bool acyclic() const {
    return !back_edges_ && !self_loops_;
  }

  bool exactBest() const {
    return !back_edges_;
  }

};


}}

#endif
