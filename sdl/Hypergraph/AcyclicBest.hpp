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

    1-best for a lattice (acyclic graph) - works fine w/ negative costs.

    a similar acyclic-hypergraph version based on Level.hpp would be pretty simple

    TODO: so far path_traits seems potentially useful only for
    InsideAlgorithm-like LogWeight sum-all-paths vs the usual viterbi min. we
    could remove it (except as needed by generic lazy_forest_kbest code)
*/

#ifndef HYPERGRAPH__ACYCLIC_BEST_JG_2013_06_10_HPP
#define HYPERGRAPH__ACYCLIC_BEST_JG_2013_06_10_HPP
#pragma once

#include <sdl/Hypergraph/HypergraphTraits.hpp>
#include <sdl/Hypergraph/InArcs.hpp>
// traits abstraction that essentially is just telling us to add getValue() in
// the ViterbiWeight sense when we compute best paths. used in honor of
// BestPath.hpp. in theory it's more configurable.

namespace sdl {
namespace Hypergraph {

/**
   Mu, Pi are property_maps. see BestPath.hpp.
*/
template <class Arc, class Mu, class Pi, bool IsGraph = true>
struct AcyclicBest {
  typedef graehl::path_traits<IHypergraph<Arc>> path_traits;

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

  std::size_t back_edges_, self_loops_;  // == 0 iff acyclic

  void push_back(StateId st) {
    // top-down. iterate over inarcs
    visitInArcs(*this, st, hg, hgAsMutable);
  }

  enum { checkUnreachable = false };

  void improve(Arc* arc, StateId head) {
    Cost& bestCost = mu[head];
    Cost w = arc->weight().getValue();
    StateIdContainer const& tails = arc->tails();
    if (IsGraph) {
      TailId const tail = tails[0];
      if (tail == head) ++self_loops_;
      Cost const tailCost = mu[tail];
      // maybe count back edges; debug about first one only
      if (checkUnreachable && tailCost == path_traits::unreachable() && tail != head && !back_edges_++) {
        SDL_DEBUG(Hypergraph.AcyclicBest, "back edge tail=" << tail << " of: " << *arc);
      }
      path_traits::extendBy(tailCost, w);
    } else {
      for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
        TailId const tail = *i;
        if (tail == head) ++self_loops_;
        Cost const tailCost = mu[tail];
        if (checkUnreachable && tailCost == path_traits::unreachable() && tail != head && !back_edges_++) {
          SDL_DEBUG(Hypergraph.AcyclicBest, "HG back edge tail=" << tail << " of: " << *arc);
        }
        path_traits::extendBy(tailCost, w);
      }
    }
    SDL_TRACE(Hypergraph.AcyclicBest, "arc " << *arc << " with cost " << bestCost << " vs. previous " << bestCost);
    if (path_traits::better(w, bestCost)) {
      bestCost = w;
      if (pi) put(pi, head, arc);
      SDL_TRACE(Hypergraph.AcyclicBest, "improved head " << head << ": new mu=" << get(mu, head) << " new pi="
                                                         << get(pi, head) << " arc: " << (Arc*)get(pi, head));
    }
  }

  void improveTopDown(Arc* arc, StateId head) {
    assert(IsGraph);
    StateIdContainer const& tails = arc->tails();
    if (tails.empty()) return;
    Cost fromCost = mu[head];
    path_traits::extendBy(arc->weight_.value_, fromCost);
    StateId tail = tails[0];
    Cost& bestCost = mu[tail];
    if (path_traits::better(fromCost, bestCost)) {
      bestCost = fromCost;
      if (pi) put(pi, tail, arc);
    }
  }

  // for top-down (in-arcs)
  void acceptIn(ArcBase* arc, StateId head) { improveTopDown((Arc*)arc, head); }

  void acceptOut(ArcBase* arc, StateId) { improve((Arc*)arc, arc->head_); }

  /// number of tails needing init (for graph, we only need up to start state and max non-terminal)
  StateId muStates_;
  /// one more than the highest pi we set (needs resetting for best-first best-path after, in case we got too
  /// many back edges)
  StateId piStates_;

  /**
     compute mu, pi for states reachable from start. caller already initialized
     these - mu to unreachable, pi to NULL (see BestPath)

     terminates early if # back edges exceeds maxBackEdges (leaving mu/pi in
     their partially completed state)
  */
  AcyclicBest(IHypergraph<Arc> const& hg, Mu mu, Pi pi, std::size_t maxBackEdges)
      : mu(mu), pi(pi), hg(hg), hgAsMutable() {
    muStates_ = hg.size();
    piStates_ = muStates_;

    StateId final = hg.final();
    if (final == kNoState)
      SDL_THROW_LOG(Hypergraph.AcyclicBest, EmptySetException,
                    "your hypergraph had no final state (and so no paths)");

    Properties hgprop = hg.uncomputedProperties();
    bool const inArcs = hgprop & kStoreInArcs;
    bool const outArcs = hgprop & kStoreAnyOutArcs;
    bool const useOutArcs = IsGraph && outArcs;  // we require outarcs for graph+nbest

    if (!useOutArcs && !inArcs)
      SDL_THROW_LOG(Hypergraph.AcyclicBest, ConfigException,
                    "must store inarcs unless your hypergraph is a simple graph");
    assert(inArcs || outArcs);

    /*
      TODO: allow !IsGraph; use reached-tails approach of e.g. Hypergraph/Level.hpp or
      Hypergraph/Empty.hpp. see e.g. S <- NP VP; NP <- x; VP <- y NP; the VP gets
      popped before NP in InArcs.
    */
    assert(IsGraph);

    StateId start = hg.start();
    if (IsGraph) {
      if (start == kNoState)
        SDL_THROW_LOG(Hypergraph.AcyclicBest, EmptySetException,
                      "your simple graph had no start state (and so no paths)");
    }

    if (hg.isMutable()) hgAsMutable = static_cast<IMutableHypergraph<Arc> const*>(&hg);

    back_edges_ = 0;
    self_loops_ = 0;
    // we wouldn't need to track this except we choose to set the kAcyclic
    // property and self-loops technically violate it (perhaps we could redefine
    // kAcyclic to spare the effort)

    std::vector<StateId> orderReverse;
    orderReverse.reserve(hg.sizeForHeads());
    typedef StateId const* I;
    assert(!get(pi, start));
    assert(!get(pi, final));
    if (useOutArcs) {
      // essentially two different-order algorithms for viterbi. one iterates
      // over in arcs the other out. both are correct for acyclic. this is all
      // to avoid requiring storing in *and* out arcs - either is fine
      back_edges_ = orderTailsLast(hg, orderReverse, maxBackEdges, IsGraph);
    } else {
      if (!IsGraph) SDL_THROW_LOG(AcyclicBest, ProgrammerMistakeException, "hg acyclic best: //TODO@JG");
      back_edges_ = orderHeadsLast(hg, orderReverse, maxBackEdges, IsGraph);
    }

    SDL_DEBUG(Hypergraph.AcyclicBest, "acyclic best: " << back_edges_ << " back edges  - max allowed is "
                                                       << maxBackEdges);
    SDL_TRACE(Hypergraph.AcyclicBest, "reverse acyclic order: " << printer(orderReverse, Util::singleline())
                                                                << " for hg:\n"
                                                                << hg);
    if (back_edges_ > maxBackEdges) return;
    if (orderReverse.empty()) return;

    if (useOutArcs) {
      for (StateId i = 0, N = muStates_; i < N; ++i)
        put(mu, i, hg.isAxiom(i) ? path_traits::start() : path_traits::unreachable());
      for (I i = &orderReverse.back(), last = &orderReverse.front();;) {
        StateId tail = *i;
        visitOutArcs(*this, tail, hg, hgAsMutable);
        if (i == last) break;
        --i;
      }
    } else {
      for (StateId i = 0, N = muStates_; i < N; ++i) put(mu, i, path_traits::unreachable());
      put(mu, final, path_traits::start());
      for (I i = &orderReverse.back(), last = &orderReverse.front();;) {
        StateId tail = *i;
        visitInArcs(*this, tail, hg, hgAsMutable);
        if (i == last) break;
        --i;
      }
      ArcBase* lastarc = 0;
      Cost mucost = 0;
      Cost pathcost = mu[start];
      for (StateId s = start; s != final;) {
        ArcBase* nextarc = get(pi, s);
        if (!nextarc)
          SDL_THROW_LOG(Hypergraph.AcyclicBest, EmptySetException,
                        "path from start->final not recovered from top-down AcyclicBest");
        put(pi, s, lastarc);
        mu[s] = mucost;
        path_traits::extendBy(nextarc->arcweight<Arc>().value_, mucost);
        s = (lastarc = nextarc)->head_;
      }
      put(pi, final, lastarc);
      mu[final] = mucost;
      assert(std::fabs(mucost - pathcost) < 0.1);
    }
    if (acyclic() && hgAsMutable) const_cast<IMutableHypergraph<Arc>*>(hgAsMutable)->addProperties(kAcyclic);
  }

  void reserve(std::size_t nState) {}

  void resetPi() {
    for (StateId i = 0, N = piStates_; i < N; ++i) pi[i] = 0;
  }

  bool acyclic() const { return !back_edges_ && !self_loops_; }

  bool exactBest() const { return !back_edges_; }
};


}}

#endif
