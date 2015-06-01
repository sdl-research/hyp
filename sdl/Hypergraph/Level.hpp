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

    a levelization of a hypergraph is a mapping L from state to (>=0) level, such
    that no arc from tail->head has L(tail)>L(head). if there are cycles, then all
    states on them must therefore be on the same level.

    Level is a full levelization if s->t implies L(s)<L(t), which is possible iff
    the fst is acyclic

    for now, we compute a full levelization if we can, otherwise put all states on
    level 0 (even though we could more interestingly levelize by effectively
    considering strongly connected components, levelizing the dag of connections
    between them).

    a full levelization has each state at the level of its maximum path length from
    some initial state.

    this is useful for LazyBeamBest (and could be useful for any lazy bottom-up
    beamed Hg, which we don't have yet (maybe useful in parsing)

*/

#ifndef LEVEL_JG201331_HPP
#define LEVEL_JG201331_HPP
#pragma once

#include <algorithm>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Util/MinMax.hpp>
#include <sdl/Util/ShrinkVector.hpp>
#include <sdl/Hypergraph/SortStates.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>

namespace sdl {
namespace Hypergraph {

typedef StateId Level;
static const Level kNoLevel = boost::integer_traits<Level>::const_max;

typedef std::vector<Level> Levels;

inline StateId level(Levels const& levels, StateId state) {
  return state < levels.size() ? levels[state] : 0;
}


/**
   Levelization levels(hg);

   levels(hg.final());

   // gives depth of deepest tree w/ final as head,
   with final depth == 0, (in other words, length of longest path if hg is an
   fst), or 0 if there is none (because of cycles). note: if you have final as
   an axiom, it's up to you to distinguish that from "there's a cycle" if you
   like.

   assert(levels(hg.start())==0);

   for a non-final-connected dag state, we'll set levels(state)==kNoLevel

   (but we won't do this for any other sort of Hg)

   TODO (perhaps): arbitrary semiring instead of just max-distance-from-start -
   this is a possibly faster Inside implementation (that doesn't use a
   preparatory SortStates, at the cost of maintaining an extra counter per
   vertex to trigger when all inarcs have known weight). also doesn't make two
   passes over the in-arcs
*/
struct Levelization : Levels {
  void moveTo(Levels& destLevels) {
    using std::swap;
    destLevels.clear();
    swap(levels(), destLevels);
  }

  Levelization() {}
  template <class Hg>
  Levelization(Hg& hg) {
    init(hg);
  }

  template <class Hg>
  Levelization(Hg const& hg) {
    init(hg);
  }

  template <class Hg>
  void init(Hg& hg) {
    init(const_cast<Hg const&>(hg));
  }

  Levels& levels() { return *this; }

  Levels const& levels() const { return *this; }

  void setZeroLevel() { Util::clearVector(levels()); }

  template <class Hg>
  void init(Hg const& hg) {
    reachedFinalWithoutCycles = false;
    nLevels = 1;
    levels().clear();
    if (hg.final() == kNoState) return;
    if (hg.isFsm()) {
      levels().reserve(hg.size());
      StateId start = hg.start();
      if (start != kNoState && hg.storesOutArcs()) {
        computeLevelsFst(hg, start);
        return;
      }
    }
    SDL_WARN(Hypergraph.Level, "meaningful (DAG) levels supported only for FST (TODO: support acyclic hg)");
    setZeroLevel();
  }

  typedef std::vector<ArcId> RemainingInArcs;

  bool acyclic() const { return reachedFinalWithoutCycles; }
  /**
     1 more than the maximum return of level(s). 1 means cyclic or start==final
  */
  Level numLevels() const { return nLevels; }

 private:
  Level nLevels;  // this being >1 means acyclic; if 0, you still need acyclic() to distinguish between
                  // start=final and cyclic
  bool reachedFinalWithoutCycles;
  struct CountFstInArcs {
    RemainingInArcs& remainIn;
    explicit CountFstInArcs(RemainingInArcs& remainIn) : remainIn(remainIn) {}
    template <class Arc>
    void operator()(Arc* arc) const {
      ++Util::atExpand(remainIn, arc->head());
    }
  };

  /**
     unreachable states are those with ANY inarcs that weren't (forall inarcs)
     reached. if markUnreachable then levels(s) for such a state s will return
     kNoLevel (instead of, incorrectly, level 0)
  */
  template <class Hg>
  void computeLevelsFst(Hg const& hg, StateId start, bool markUnreachable = true) {
    assert(hg.storesOutArcs());
    StateId N = hg.size();
    RemainingInArcs remainIn(N);  // unlike the Hg (have in-arcs) case, we need a
    // separate array for the max vs. remaining
    if (hg.storesInArcs()) {
      for (StateId s = 0; s < N; ++s) remainIn[s] = hg.numInArcs(s);
    } else
      hg.forArcs(CountFstInArcs(remainIn));
    StateId final = hg.final();
    assert(final != kNoState);

    Util::atExpand(levels(), start);
    reachFst(hg, remainIn, start);

    nLevels = levels()[final];
    if (!remainIn[final] && (final == start || nLevels)) {
      ++nLevels;  // number of distinct levels = final level + 1 (they start at level 0)
      // final is reachable *only* by a finite number of paths (the start->final-reaching subgraph is
      // acyclic).
      reachedFinalWithoutCycles = true;
      if (markUnreachable)
        for (StateId s = 0, maxStoredLevel = (StateId)levels().size(); s != maxStoredLevel; ++s)
          if (remainIn[s]) levels()[s] = kNoLevel;
    } else {  // start==final or else there are cycles or else there are no paths from start->final
      // we don't actually know whether final is reachable by any path at all! but if it is, there are
      // infinitely many.
      setZeroLevel();
    }
  }

  template <class Hg>
  void reachFst(Hg const& hg, RemainingInArcs& remainIn, StateId tail) {
    std::size_t nLevels = levels().size();
    assert(tail < nLevels);
    Level headLevel = levels()[tail] + 1;
    for (ArcId aid = 0, nArcs = hg.numOutArcs(tail); aid != nArcs; ++aid) {
      StateId head = hg.outArc(tail, aid)->head();
      if (--remainIn[head] == 0) {
        Util::maxEq(Util::atExpand(levels(), head), headLevel);
        reachFst(hg, remainIn, head);
      }
    }
  }

 public:
  /**
     return level if state is connected to final, otherwise return gibberish
  */
  Level operator()(StateId state) const { return level(levels(), state); }

  /**
     TODO: implement this

     for a hypergraph whose in-arcs all have reachable (from bottom-up) tails,
     we can do the following:

     similar to Empty.hpp except that we activate a state only when we've
     reached it via *all* its in-arcs (Empty is satisfied w/ reachability once
     you've reached on *any*. In both cases, all the tails must be reached
     (starting w/ axioms) before an edge is activated.

     start out with "level" of state being the number of in arcs, and a map of
     "TailsLeft" for each arc (= # of tails initially)

     activate all the axiom states

     when activating a state, decrement its out-arcs' tails-left counter by one,
     and if it reaches zero, activate the edge

     activating an edge means to decrement the in-arcs-remaining counter for the
     head state. if it becomes zero, then compute the max simple path length
     (arcs from an axiom to head) and store the result (the result is already
     computed for all the tails, by induction). note that we can reuse the same
     array for # in arcs remaining and level!

     finally, the array can be smaller than # of states w/ missing element
     meaning level 0 (this would be efficient if you've sorted all the axioms
     (terminals) off to the end, since they never have any in-arcs, and by
     definition have a Level of zero

     it may happen that the final state is never activated. in that case, we
     know there's a cycle. cool! in that case we just clear the array
     (everything at level 0)

     any unreachable states will have a "level" equal to # of in-arcs. this
     shouldn't be a problem. we could fix it by using a sign bit of "level" or
     another array to distinguish "not fully reached (may be in a cycle, whether
     or not connected to final state, or whether final state connected sub-hg is
     acyclic)" vs. "fully reached and we know the level".

     TODO: figure out the most efficient way to assign aggressive strongly
     connected components based levels (check out all the simple-path-reachable
     heads from the not-fully-completed states), and assign the max level
     encountered, or something like that, probably works. needs proof

     alternatively, we could use SortStates and compute max path length for dag
     in a straightforward fashion (detecting back edges and self loops is easy
     too). i believe the approach described above is faster and doesn't require
     storing in+outarcs in the hg (we'd keep track ourselves), but mostly that
     it can be generalized to deal with SCCs

     to handle useless in-arcs, we can separately track "head reached at all" vs
     "head reached for all in-arcs". or we can just require calling
     pruneUnreachable first (on pain of otherwise getting an all-0 levelization
     when it was really a dag w/ some danglers that should have been pruned)

  */
  template <class Arc>
  struct ComputeHgLevels {
    Level* pLevels;
    typedef unordered_map<Arc*, Level> TailsLeft;
    TailsLeft tailsleft;
  };
};

template <class Arc>
bool isAcyclicByLevelization(IHypergraph<Arc> const& hg) {
  return Levelization(hg).acyclic();
}


}}

#endif
