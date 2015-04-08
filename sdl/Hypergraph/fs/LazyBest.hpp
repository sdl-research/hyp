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

    expand all paths from source best-first in a lazy fsm, stopping when finding
    the best path to a final state, or when fully expanded and no path exists.

    it's likely more efficient to use integerized states via fs/Cache.hpp

    TODO: expose an optional heuristic, e.g. compute exact outside for each of
    two things to be composed, then the lazy composition could have heuristic
    distance = the sum (optimistic)

    TODO: nbest (hopefully making good use of exact outside scores for states
    we've found best path -> final from)
*/

#ifndef LAZYBEST_JG2013123_HPP
#define LAZYBEST_JG2013123_HPP
#pragma once

#include <sdl/Hypergraph/fs/Fst.hpp>
#include <sdl/Hypergraph/fs/Path.hpp>
#include <sdl/Util/PriorityQueue.hpp>
#include <boost/property_map/property_map.hpp>
#include <sdl/Pool/object_pool.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/NonNullPointee.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Hypergraph {
namespace fs {


struct LazyBestOptions {
  float expandMoreArcs;
  bool removeEpsilon;
  bool projectOutput;
  bool annotations;

  std::string logNumWordsName;

  LazyBestOptions() : expandMoreArcs(), removeEpsilon(true), projectOutput(false), annotations(true) {}

  /**
     if you'll be using lazy-best on a hg that isn't best-first sorted.
  */
  void fullyExpandOutArcs() {
    expandMoreArcs = std::numeric_limits<float>::max() * 0.5f;
    // note: not infinity (that would depend on math compilation flags to work properly;

    // TODO: test whether we can use infinity
  }

  template <class Config>
  void configure(Config& config) {
    config.is("LazyBest");
    config("best-first lazy fst best-path search");
    config("expand-more-arcs", &expandMoreArcs)(
        "to counter for slightly not-best-first arcs, estimate the cost of the next arc as the last arc less "
        "this margin. if this is INF then every arc of a state will be expanded when it's reached").init(0);
    config("remove-epsilon", &removeEpsilon)
        .init(true)("possibly remove some epsilon transitions in the result (if prune-to-nbest: 1)");
    config("project-output", &projectOutput)
        .self_init()("output an fsa using the composition's output symbols (setting input == output");
    config("log-num-words-name", &logNumWordsName)(
        "if nonempty, log # of output words under sdl.PerformancePer.[log-num-words-name]");
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
    config("annotations", &annotations).self_init()("preserve annotations (e.g. constraints) on input hg");
#else
    config("annotations", &annotations).init(false)("(not enabled in this build; must be false)").verbose();
#endif
  }

  PrependForInputStatePtr prependForStateId;
  Syms const* prependFor(StateId s) {
    return s != kNoState && s < prependForStateId->size() ? &(*prependForStateId)[s] : NULL;
  }

  template <class State>
  Syms const* prependFor(State const& s) {
#if 1
    return NULL;
#else
    if (!prependForStateId) return NULL;
    StateId s = stateId(s);
    return s != kNoState && s < prependForStateId->size() ? &(*prependForStateId)[s] : NULL;
#endif
  }
};


/**
   returns 1-best path, if any, from start->final in an fst

   usage:

   typedef FeatureWeight Weight;
   typedef ArcTpl<Weight> Arc;

   IHypergraph<Arc>  hg;
   typedef HypergraphFst<Arc> Fst;
   Fst fst(hg);
   typedef FstArc<Weight, StateId> PathArc;
   typedef Path<FstArc> FstPath;
   FstPath path;
   LazyBest<Fst>(hg, path);
   path.computeWeight();
   forall (PathArc const& arc, path) {}

*/


template <class Fst, class DistanceFn = DistanceForFstArc<typename Fst::Weight> >
struct LazyBest : DistanceFn {
  typedef DistanceFn DistanceF;
  typedef typename DistanceF::result_type Distance;
  typedef typename Fst::Weight Weight;
  typedef typename Fst::State State;
  typedef typename Fst::Arc FstArc;
  typedef typename Fst::Arcs FstArcs;
  typedef Path<FstArc> FstPath;
  typedef Path<FstArcNoState<Weight> > FstPathNoState;

  /// set from outside for constraints in TrainableCapitalizerModule
  shared_ptr<PrependForInputStateFn<State> > prependForInputState;

  typedef std::size_t QueueIndex;

  Fst const& fst;

  struct Best : FstArc {  // FstArc:: labelPair, weight, dst
    Best() {}

    Best(FstArc const& arc, Distance heuristic) : FstArc(arc), heuristic(heuristic) {}

    /// identity for Bests set:
    bool operator==(Best const& o) const { return this->dst == o.dst; }
    friend inline std::size_t hash_value(Best const& x) { return boost::hash<State>()(x.dst); }
    Distance distance;

    /**
       fn of this->dst only.
    */
    Distance heuristic;

    /**
       must call before pushing to priority queue.
    */
    void estimateSuccessor(Distance insideMinusMargin) {
      estimatedSuccessorDistance = insideMinusMargin + heuristic;
    }

    /**
       must call before pushing to priority queue.
    */
    void initDistance(Distance dist) {
      distance = dist;
      estimatedSuccessorDistance = dist + heuristic;
    }

    void improveArc(FstArc const& arc) {
      assert(arc.dst == this->dst);
      this->labelPair = arc.labelPair;
      this->weight = arc.weight;
      IF_SDL_HYPERGRAPH_FS_ANNOTATIONS(this->annotations = arc.annotations;)
    }

    void init(Fst const& fst, Best* prev = 0) {
      predecessor = prev;
      arcs = fst.outArcs(this->dst);
    }

    /// derivation of best path from start to state:
    Best* predecessor;  // null if state is start. prev->state =>inArc state

    /// generation of successors and placement in global priority queue:
    Distance estimatedSuccessorDistance;
    FstArcs arcs;
    QueueIndex index;  // for adjusting existing queue items. could set to -1 on construct but not needed
  };


 private:
  /**
     for unordered_set.
  */
  struct BestHandle {
    Best* p;
    bool operator==(BestHandle const& o) const { return p->dst == o.p->dst; }
    friend inline std::size_t hash_value(BestHandle const& o) { return boost::hash<State>()(o.p->dst); }
    Best* operator->() const { return p; }
    operator Best*() const { return p; }
    Best& operator*() const { return *p; }
  };


  typedef Best* BestP;

  /**
     for priority queue.
  */
  struct BestDistancePropertyMap {
    typedef boost::readable_property_map_tag category;
    typedef Distance value_type;
    typedef value_type const& reference;
    typedef BestP key_type;
    friend inline value_type const& get(BestDistancePropertyMap const&, key_type bestp) {
      return bestp->estimatedSuccessorDistance;
    }
  };

  /**
     for priority queue.
  */
  struct BestIndexPropertyMap {
    typedef boost::lvalue_property_map_tag category;
    typedef QueueIndex value_type;
    typedef value_type& reference;
    typedef BestP key_type;
    reference operator[](key_type bestp) const { return bestp->index; }
    friend inline void put(BestIndexPropertyMap const&, key_type bestp, value_type const& v) {
      bestp->index = v;
    }
    friend inline value_type const get(BestIndexPropertyMap const&, key_type bestp) { return bestp->index; }
  };

  typedef Pool::object_pool<Best> BestsPool;
  BestsPool bestsPool;  // O(n) individual destroy, so free only at end

  typedef std::less<Distance> BetterDistance;
  typedef std::vector<BestP> QueueVector;
  typedef Util::d_ary_heap_indirect<BestP, 4, BestDistancePropertyMap, BestIndexPropertyMap, BetterDistance,
                                    QueueVector, QueueIndex, Util::NonNullPointeeEqual> Queue;
  Queue queue;

  // at most one Expand active for a state in Queue, enforced by best map.

  // TODO: policy for negative cost edge improving already existing Expand (reset arcs to beginning to redo
  // successors?) - see 'rereach' in BestPath

  typedef unordered_set<BestP, Util::NonNullPointeeHash<Best>, Util::NonNullPointeeEqual> Bests;
  typedef typename Bests::iterator BestsIter;
  Bests bests;

 public:
  LazyBestOptions opt;

  /**
     \param bestPathOut out: gets arcs for 1best. may be either FstPath or FstPathNoState
  */
  template <class FstPathMaybeWithState>
  LazyBest(Fst const& fst, FstPathMaybeWithState& bestPathOut, LazyBestOptions const& opt = LazyBestOptions())
      : fst(fst), opt(opt) {
    State const& start = fst.startState();
    bestPathOut.startState(start);
    if (fst.final(start))
      bestPathOut.totalDistance = 0;
    else {
      Best seed;
      // we don't bother to allocate out of pool because you're not allowed to use
      // any of the data structures after constructor, which does all the work
      seed.setDst(start);
      seed.initDistance(0);
      seed.init(fst);
      bests.insert(&seed);
      queue.push(&seed);
      bestToFinal(bestPathOut);
    }
  }

 private:
  /**
     \param bestPathOut out: gets arcs for 1best. may be either FstPath or FstPathNoState
  */
  template <class FstPathMaybeWithState>
  void pathFromBest(BestP p, FstPathMaybeWithState& bestPathOut) {
    assert(p);
    bestPathOut.totalDistance = p->distance;
    for (; p->predecessor; p = p->predecessor)  // skip the start-state pseudo-arc
      bestPathOut.prepend(static_cast<FstArc const&>(*p));
  }

  /**
     \param bestPathOut out: gets arcs for 1best. may be either FstPath or FstPathNoState
  */
  template <class FstPathMaybeWithState>
  void bestToFinal(FstPathMaybeWithState& bestPathOut) {
    while (!queue.empty()) {
      Best& from = *queue.top();
      if (fst.final(from.dst)) {
        pathFromBest(&from, bestPathOut);
        return;  // success
      }
      if (from.arcs) {
        FstArc const& arc
            = from.arcs();  // FstArc is a base class for Best; unordered_set only looks at that part
        Distance distance = from.distance + arc.getDistance();
        from.estimateSuccessor(distance - opt.expandMoreArcs);
        queue.adjust_top();  // note: moving items around in queue doesn't invalidate the pointed-to Best

        // check for new state or improvement in distance to existing
        std::pair<typename Bests::iterator, bool> iNew = bests.insert((Best*)&arc);

        if (iNew.second) {  // didn't exist before.
          BestP bestForDst = bestsPool.construct(
              arc, fst.heuristic(arc.dst));  // construct full object to reside in set/queue
          bestForDst->initDistance(distance);
          bestForDst->init(fst, &from);
          const_cast<BestP&>(*iNew.first) = bestForDst;  // same key/hash/equal so ok
          queue.push(bestForDst);
          // TODO: valgrind jump depends on uninit warning - compile w/ option for d_ary_heap that prevent
          // this?
        } else {
          BestP bestForDst = *iNew.first;
          if (distance >= (*iNew.first)->distance)  // no better than existing item
            continue;
          // improvement of existing item in queue
          bestForDst->improveArc(arc);  // set new better arc; doesn't change key/hash/equal
          bestForDst->init(fst, &from);  // set new src state, reset generator
          bestForDst->initDistance(distance);
          queue.update(bestForDst);
        }
      } else  // no more arcs
        queue.pop();
    }
  }
};


template <class Fst, class FstPath>
bool lazyBest(Fst const& fst, FstPath& path, LazyBestOptions const& opt = LazyBestOptions()) {
  LazyBest<Fst>(fst, path, opt);
  return path;
}

template <class Fst>
Path<typename Fst::Arc> lazyBestWithState(Fst const& fst, LazyBestOptions const& opt = LazyBestOptions()) {
  Path<typename Fst::Arc> path;
  lazyBest(fst, path, opt);
  return path;
}

template <class Fst>
PathNoState<typename Fst::Weight> lazyBest(Fst const& fst, LazyBestOptions const& opt = LazyBestOptions()) {
  PathNoState<typename Fst::Weight> path;
  lazyBest(fst, path, opt);
  return path;
}

template <class HgPtr, class FstPath>
bool lazyBestForHgPtr(HgPtr const& sortedHg, FstPath& path, LazyBestOptions const& opt = LazyBestOptions()) {
  typedef typename Util::RemoveConstPointer<HgPtr>::type Hg;
  typedef typename Hg::Arc Arc;
  typedef typename Hg::FstArcT FstArc;
  typedef IMutableHypergraph<Arc> MutableHg;
  if (sortedHg->isMutable()) {
    typedef HypergraphFst<Arc> Fst;
    Fst fst(boost::dynamic_pointer_cast<MutableHg const>(sortedHg), opt.annotations);
    lazyBest(fst, path, opt);
  } else {
    typedef ConstHypergraphFst<Arc> Fst;
    Fst fst(sortedHg, opt.annotations);
    lazyBest(fst, path, opt);
  }
  return path;
}

/**
   compute best path without sorting out arcs.
*/
template <class Arc, class FstPath>
bool lazyBestForHgUnsorted(IHypergraph<Arc> const& hg, FstPath& path) {
  LazyBestOptions opt;
  opt.fullyExpandOutArcs();
  return lazyBestForHgPtr(ptrNoDelete(hg), path);
}

template <class Arc>
PathNoState<typename Arc::Weight> lazyBestForHgUnsorted(IHypergraph<Arc> const& hg) {
  PathNoState<typename Arc::Weight> path;
  lazyBestForHgUnsorted(hg, path);
  return path;
}

template <class Arc, class FstPath>
bool lazyBestForHg(IHypergraph<Arc> const& hg, FstPath& path, LazyBestOptions const& opt = LazyBestOptions()) {
  return lazyBestForHgPtr(ensureProperties(hg, kOutArcsSortedBestFirst), path, opt);
}

template <class Arc, class FstPath>
bool lazyBestForHg(IHypergraph<Arc>& hg, FstPath& path, LazyBestOptions const& opt = LazyBestOptions()) {
  return lazyBestForHgPtr(ensureProperties(hg, kOutArcsSortedBestFirst), path, opt);
}

template <class Hg, class FstPath>
bool lazyBestForAnyHg(Hg& hg, FstPath& path, LazyBestOptions const& opt = LazyBestOptions()) {
  return lazyBestForHgPtr(ensureProperties(hg, kOutArcsSortedBestFirst), path, opt);
}

template <class Hg, class FstPath>
bool lazyBestForHgConst(Hg const& hg, FstPath& path, LazyBestOptions const& opt = LazyBestOptions()) {
  typedef typename Hg::Arc Arc;
  return lazyBestForHg(static_cast<IHypergraph<Arc> const&>(hg), path, opt);
}

template <class Fst, class Arc>
bool lazyBestToHg(Fst& fst, IMutableHypergraph<Arc>& outHg, LazyBestOptions const& opt = LazyBestOptions()) {
  outHg.setVocabulary(fst.getVocabulary());
  typedef typename Fst::Arc FstArc;
  typedef typename Fst::Weight Weight;
  Path<FstArcNoState<Weight> > path;
  LazyBest<Fst> lazyBest(fst, path, opt);
  //  SDL_DEBUG(Hypergraph.fs.LazyBest, print(path, outHg.getVocabulary()));
  std::size_t nWords
      = path.addToHypergraph(outHg, kNoState, kNoState, opt.removeEpsilon, opt.projectOutput, opt.annotations);
  if (!opt.logNumWordsName.empty())
    LOG_INFO_NAMESTR(kPerformancePerLogPrefix + opt.logNumWordsName, nWords << " output words in 1-best for "
                                                                            << opt.logNumWordsName);
  return path;
}


}}}

#endif
