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

    all-pairs summary of paths (using Weight plus, times).
*/

#ifndef HYPERGRAPH_ALL_PAIRS_SHORTESTDISTANCE_HPP
#define HYPERGRAPH_ALL_PAIRS_SHORTESTDISTANCE_HPP
#pragma once

#include <sdl/Hypergraph/WeightUtil.hpp>

#include <cmath>
#include <sdl/Util/Forall.hpp>

#include <sdl/Util/Matrix.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Hypergraph/ArcWeight.hpp>

namespace sdl {
namespace Hypergraph {


/**
   Seeds the distances matrix with the hypergraph arc weights.

   assumes distances was initialized to all Weight::zero() - note: Weight() may be
   Weight::one()

   for graph, not hypergraph.

   TODO: Johnson's algorithm is a faster all-pairs shortest-distance algorithm
   than Floyd-Warshall.

*/
template <class Arc, class ArcWtFn>
void floydWarshallInit(IHypergraph<Arc> const& hg, Util::Matrix<typename ArcWtFn::Weight>* pdistances,
                       ArcWtFn arcWtFn) {
  typedef typename ArcWtFn::Weight Weight;
  Util::Matrix<Weight>& dist = *pdistances;
  dist.setDiagonal(Weight::one(), Weight::zero());
  for (StateId tail = 0, numStates = (StateId)dist.getNumRows(); tail < numStates; ++tail) {
    Weight* rowTail = dist.row(tail);
    forall (ArcId aid, hg.outArcIds(tail)) {
      Arc* arc = hg.outArc(tail, aid);
      StateId head = arc->head();
      Hypergraph::plusBy(arcWtFn(arc), rowTail[head]);
    }
  }
}

/**
   for k from 1 to |V|
    for i from 1 to |V|
     for j from 1 to |V|
      if dist[i][k] + dist[k][j] < dist[i][j] then
       dist[i][j] = dist[i][k] + dist[k][j]

   from http://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
*/
template <class Weight>
void floydWarshallOverMatrix(Util::Matrix<Weight>* pdistances) {
  Util::Matrix<Weight>& dist = *pdistances;
  std::size_t const n = dist.getNumRows();
  assert(n == dist.getNumCols());
  for (StateId k = 0; k < n; ++k) {
    Weight* rowk = dist.row(k);
    for (StateId i = 0; i < n; ++i) {
      if (k == i) continue;  // needed so log semiring has correct results
      Weight* rowi = dist.row(i);
      Weight const& wik = rowi[k];
      if (isZero(wik)) continue;
      for (StateId j = 0; j < n; ++j) {
        if (i == j) continue;
        Weight const& wkj = rowk[j];
        if (isZero(wkj)) continue;
        Hypergraph::plusBy(times(wik, wkj), rowi[j]);
      }
    }
  }
}


/**
   cubic time all-pairs (shortest) paths.

   assumes distances was initialized to all Weight::zero() - note: Weight() may
   be Weight::one()
*/
template <class Arc, class ArcWtFn>
void floydWarshall(IHypergraph<Arc> const& hg, Util::Matrix<typename ArcWtFn::Weight>* distances,
                   ArcWtFn const& arcWtFn) {
  if (!hg.isFsm())
    SDL_THROW_LOG(Hypergraph, ConfigException, "Current floydWarshall implementation needs FSM input");
  floydWarshallInit(hg, distances, arcWtFn);
  floydWarshallOverMatrix(distances);
}

template <class Arc>
void floydWarshall(IHypergraph<Arc> const& hg, Util::Matrix<typename Arc::Weight>* distances) {
  floydWarshall(hg, distances, ArcWeight<typename Arc::Weight>());
}

/**

   AllPairSortedDag is faster than floydWarshall (but works for DAG only).

   assuming input is a topologically sorted (distances matrix is NxN with
   0...N-1 being the nonlexical (including start) states), we can compute
   all-pairs (shortest) paths by repeatedly, for each start state, running the
   viterbi (shortest) paths algorithm. this is only quadratic; floydWarshall
   handles cycles but is cubic

   Also, an early bool KeepFn(typename Arc::Weight) can say when (by returning
   false) a path's weight is inconsequential enough that we don't want to extend
   the path further. for example: we want to find all pairs of states with the
   least (so ViterbiWeight) # of words under some bounds, for PhraseBased
   distortion limit. or: ignore low probability paths when summing (good luck
   defining "low probability" for MT, though)

   TODO: alternative (sparse) output instead of (same as floydWarshall)
   distances Matrix. but since overall algorithm is potentially quadratic, may
   not matter

   this could be restructured: semiring of sparse vector of (source
   state, weight), one viterbi pass. but probably no faster.

   as with floydWarshall, you might want to use an AxiomWeightHypergraph to
   modify the weights on the original hg arcs

   distances is assumed to be initialized to all zeros (as any freshly
   constructed Matrix would be), or more precisely - only the actually adjacent
   (before KeepFn) elements are set.

   like floydWarshall, the diagonal elements are set to Weight::one. but a
   future optimization may change this (don't count on it)

   if ZeroDroppedPaths, any time we don't find a path source->via worth
   continuing, we set source->via's distances entry to Weight::zero()

   i chose not to use path_traits because the semirings are used in their
   default plus/times accumulative sense. an optimization for semirings that are
   actually viterbi type could save a little time if we change away from matrix
   to adjacency list
*/

#define SDL_ALL_PAIRS_REQUIRES_SORTED_DAG \
  "FSM input (topologically sorted, nonlexical states first) with out-arcs"

struct KeepAll {
  template <class A>
  bool operator()(A const& a) const {
    return true;
  }
};

template <class InHypergraph, class KeepFn = KeepAll,
          class ArcWtFn = ArcWeight<typename InHypergraph::Arc::Weight>, bool ZeroDroppedPaths = true>
struct AllPairsSortedDag : private ArcWtFn, private KeepFn {  // empty base class opt.
  typedef InHypergraph InputHypergraph;
  typedef typename InputHypergraph::Arc Arc;
  typedef typename ArcWtFn::Weight Weight;
  typedef Util::Matrix<Weight> Distances;

  enum { kZeroDropped = ZeroDroppedPaths };

  InputHypergraph const& hg;
  Distances& distances;
  StateId nStates;  // # nonlex states (topo sorted => 0...nStates-1 are those)
  Weight const oneWeight;
  Weight const zeroWeight;

  KeepFn const& keep() const { return *this; }
  ArcWtFn const& arcWeight() const { return *this; }

  AllPairsSortedDag(InputHypergraph const& hg, Distances& distances, KeepFn keepFn = KeepFn(),
                    ArcWtFn arcWtFn = ArcWtFn(), Weight const& oneWeight = Weight::one(),
                    Weight const& zeroWeight = Weight::zero())
      : ArcWtFn(arcWtFn)
      , KeepFn(keepFn)
      , hg(hg)
      , distances(distances)
      , nStates((StateId)distances.getNumRows())
      , oneWeight(oneWeight)
      , zeroWeight(zeroWeight) {
    //    if (!hg.isFsm() || !hg.storesOutArcs())
    // SDL_THROW_LOG(Hypergraph, ConfigException,
    // "Current AllPairsSortedDag implementation needs " SDL_ALL_PAIRS_REQUIRES_SORTED_DAG);
    if (distances.getNumCols() != nStates)
      SDL_THROW_LOG(Hypergraph, IndexException,
                    "AllPairsSortedDag stores distances into a square matrix. yours wasn't");
    compute();
  }

 private:
  /**
     (maybe) improve destination cost from->to (weight=distanceTo)
     with partial path from->via (weight=viaWt), using arc via->to (weight=arcWt)
  */
  void improve(Weight& distanceTo, Weight const& viaWt, Weight const& arcWt) {
    Hypergraph::plusBy(times(viaWt, arcWt), distanceTo);
  }

  void checkTopoSort(StateId from, StateId to) {
    if (to <= from)
      SDL_THROW_LOG(Hypergraph, CycleException, "AllPairsSortedDag found cycle-causing arc "
                                                    << from << "->" << to
                                                    << " (requires " SDL_ALL_PAIRS_REQUIRES_SORTED_DAG ")");
  }

  void compute() {
    for (StateId from = 0; from < nStates; ++from) {  // ultimate source
      Weight* distancesFrom = distances.row(from);
      distancesFrom[from] = oneWeight;
      for (StateId via = from; via < nStates; ++via) {
        Weight& viaWt = distancesFrom[via];
        if (viaWt == zeroWeight) continue;
        // TODO: slight speedup: maintain reachability set (log n or bit vector, instead of n)

        if (!keep()(viaWt)) {
          if (kZeroDropped) viaWt = zeroWeight;
          continue;  // we already don't care for from->via. definitely don't continue it.
        }

        // for outarcs of via:
        ArcId nArcs = hg.numOutArcs(via);
        for (ArcId a = 0; a < nArcs; ++a) {
          Arc const& arc = *hg.outArc(via, a);
          ArcId to = arc.head();
          checkTopoSort(from, to);
          assert(to < distances.getNumCols());
          improve(distancesFrom[to], viaWt, ArcWtFn::operator()(&arc));
          // don't reject head yet (semiring may accumulate sums).
        }
      }
    }
  }
};

template <class InputHypergraph, class KeepFn, class ArcWtFn>
void allPairsSortedDag(InputHypergraph const& hg, Util::Matrix<typename ArcWtFn::Weight>& distances,
                       KeepFn const& keepFn, ArcWtFn const& arcWtFn, bool zeroDroppedPaths = true,
                       typename ArcWtFn::Weight const& oneWeight = ArcWtFn::Weight::one(),
                       typename ArcWtFn::Weight const& zeroWeight = ArcWtFn::Weight::zero()) {
  distances.fill(zeroWeight);
  if (zeroDroppedPaths)
    AllPairsSortedDag<InputHypergraph, KeepFn, ArcWtFn, true> compute(hg, distances, keepFn, arcWtFn,
                                                                      oneWeight, zeroWeight);
  else
    AllPairsSortedDag<InputHypergraph, KeepFn, ArcWtFn, false> compute(hg, distances, keepFn, arcWtFn,
                                                                       oneWeight, zeroWeight);
}


}}

#endif
