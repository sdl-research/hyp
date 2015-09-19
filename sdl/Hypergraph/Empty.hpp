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

    emptiness checking for CFG.
*/

#ifndef HYP__HYPERGRAPH__EMPTY_HPP
#define HYP__HYPERGRAPH__EMPTY_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>
#include <sdl/Util/BitSet.hpp>

#ifndef NDEBUG
#include <sdl/Util/DefaultPrintRange.hpp>
#endif

#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Util/GraphDfs.hpp>

namespace sdl {
namespace Hypergraph {

// outarcs for duplicate tails have duplicate mentions of arc in
// exactly the right way, so this isn't used.
struct CountUniqueTails {
  mutable Util::BitSet seen;
  CountUniqueTails(StateId nStates) : seen(nStates) {}
  TailId operator()(StateIdContainer const& tails) const { return (*this)(tails.begin(), tails.end()); }
  TailId operator()(StateId const* b, StateId const* e) const {
    TailId sum = 0;
    for (StateId const* i = b; i != e; ++i)
      if (Util::latch(seen, *i)) ++sum;
    for (StateId const* i = b; i != e; ++i) seen.reset(*i);
    return sum;
  }
};

struct Reach {
  HypergraphBase const* h;
  typedef shared_ptr<HypergraphBase const> HP;
  HP holdh;
  StateSet reached;
  typedef unordered_map<ArcBase*, unsigned> TailsLeft;
  TailsLeft tailsleft;
  bool computeUseful;
  Util::Graph outside;

  shared_ptr<StateSet> useful(StateId goal) const {
    assert(computeUseful);
    shared_ptr<StateSet> r(new StateSet());
    StateSet& useful = *r;
    Util::VertexColors c;
    Util::dfsColor(outside, c, goal);
    StateId N = (StateId)c.size();
    assert(N >= h->sizeForHeads());
    useful.clear();
    useful.resize(N);
    for (StateId s = 0, e = (StateId)c.size(); s != e; ++s)
      if (s == goal ? Util::test(reached, s) : c[s] != Util::unvisitedColor()) useful.set(s);
    return r;
  }

  shared_ptr<StateSet> useful() const { return useful(final); }

  // TODO: special case for fsm/graph - no need for tail count - every arc is
  // immediately usable. and no need to make a copy if storing first tail only

  template <class Arc>
  Reach(IHypergraph<Arc> const& hg, bool computeUseful = false, bool stopAtFinal = false)
      : reached(hg.size())
      , computeUseful(computeUseful)
      , outside((Util::GraphSize)(computeUseful ? hg.size() : 0))
      , stopAtFinal(stopAtFinal)
      , final(hg.final()) {
    if (hg.isGraph()) {
      StateId const start = hg.start();
      if (start != kNoState) {
        holdh = ensureFirstTailOutArcs(hg);
        h = holdh.get();
        assert(h->storesOutArcs());
        reachGraph(start);
        return;
      }  // else fall through to CFG case:
    }
    holdh = ensureOutArcs(hg);
    h = holdh.get();
    assert(h->storesAllOutArcs());
    h->arcsOnce(*this, tailsleft);
    h->forAxioms(*this);
  }

  /// for initializing tailsleft
  unsigned operator()(ArcBase const* arc) const {
    return arc->getNumTails();
    // outarcs for duplicate tails have duplicate mentions of arc in
    // exactly the right way that we should ignore the duplicate issue
    unsigned tails = arc->getNumTails();
    return tails;
  }

  bool tailFinishes(ArcBase* arc) {
    assert(tailsleft[arc]);
    return --tailsleft[arc] == 0;
  }

  /// for bottom-up reachability
  void operator()(HypergraphBase const& hs, StateId s) {
    reach(s);
  }

  void reachGraph(StateId state) {
    assert(state != kNoState);
    if (!Util::latch(reached, state)) return;
    if (stopAtFinal && state == final) return;
    for (ArcId i = 0, f = h->numOutArcs(state); i < f; ++i) reachGraph(h->outArc(state, i));
  }

  void reachGraph(ArcBase* arc) {
    StateId const head = arc->head();
    reachArc(arc, head);
    reachGraph(head);
  }

  void reachArc(ArcBase* arc, StateId head) {
    if (computeUseful) {
      for (StateIdContainer::const_iterator i = arc->tails().begin(), e = arc->tails().end(); i != e; ++i) {
        StateId tail = *i;
        boost::add_edge(head, tail, outside);
      }
    }
  }

  void reach(StateId s) {
    if (!Util::latch(reached, s)) return;
    if (stopAtFinal && s == final) return;
    for (ArcId a = 0, f = h->numOutArcs(s); a != f; ++a) {
      ArcBase* arc = h->outArc(s, a);
      if (tailFinishes(arc)) {
        StateId const head = arc->head();
        reachArc(arc, head);
        reach(head);
      }
    }
  }

  bool final_reached() const {
    bool r = Util::test(reached, final);
    return r;
  }

  bool const stopAtFinal;
  StateId const final;
};

template <class Arc>
inline bool empty(IHypergraph<Arc> const& hg) {
  if (hg.prunedEmpty())
    return true;
  else {
    assert(hg.final() != Hypergraph::kNoState);
    Reach r(hg, false, true);
    return !r.final_reached();
  }
}

template <class A>
bool pruneEmpty(IMutableHypergraph<A>& a) {
  if (empty(a)) {
    a.setEmpty();
    return true;
  }
  return false;
}


}}

#endif
