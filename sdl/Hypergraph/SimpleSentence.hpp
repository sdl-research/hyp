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

    determine whether input graph is simple sentence (has only one derivation and
    no distracting arcs not in that derivation)
*/

#ifndef SIMPLESENTENCE_JG_2014_04_30_HPP
#define SIMPLESENTENCE_JG_2014_04_30_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Hypergraph {

struct CountStatesVisitor {
  HypergraphBase const& hg;
  CountStatesVisitor(HypergraphBase const& hg) : hg(hg) {}
  mutable StateId n = 0;
  mutable StateId neps = 0;
  void operator()(ArcBase const* a) const {
    ++n;
    if (!hg.firstLexicalInput(a)) ++neps;
  }
  void alternative(ArcBase const*) const {}
};

struct IgnoreArcVisitor {
  void operator()(ArcBase const*) const {}
  void alternative(ArcBase const*) const {}
};

inline bool leftToRightSimpleSentenceVisit(HypergraphBase const& hg) {
  return hg.storesOutArcs();
}

enum { kSinglePathOnly = 0, kAllowLabelAmbiguity = 1 };
/**
   \return 'hg is a graph with only a single path, or state sequence for all paths if allowSausage'

   call visit(a) for the first arc of path in either forward order (if outarcs) or reverse

   also call visit.alternative(a) for other arcs if allowSausage and >1 path

   if allowSausage, allow simple "sausage" ambiguity (multiple arcs from s->t
   but s->t is deterministic)
*/
template <class Visitor>
bool visitSimpleSentenceArcs(HypergraphBase const& hg, Visitor const& visit, bool allowSausage = kSinglePathOnly) {
  if (!hg.isGraph()) return false;
  if (hg.storesOutArcs()) {
    StateId end = hg.final(), s = hg.start();
    for (;;) {
      if (s == end) return true;
      ArcId n = hg.numOutArcs(s);
      if (!n || !allowSausage && n != 1) return false;
      ArcBase* a = hg.outArc(s, 0);
      StateId nexts = a->head_;
      visit(a);
      if (allowSausage)
        for (ArcId i = 1; i < n; ++i)
          if ((a = hg.outArc(s, i))->head_ != nexts)
            return false;
          else
            visit.alternative(a);
      s = nexts;
    }
  } else if (hg.storesInArcs()) {
    StateId s = hg.final(), end = hg.start();
    for (;;) {
      if (s == end) return true;
      ArcId n = hg.numInArcs(s);
      if (!n || !allowSausage && n != 1) return false;
      ArcBase* a = hg.inArc(s, 0);
      StateIdContainer const& tails = a->tails_;
      if (tails.empty()) return false;
      visit(a);
      if (allowSausage)
        for (ArcId i = 1; i < n; ++i) {
          StateIdContainer const& tails = (a = hg.inArc(s, i))->tails_;
          if (tails.empty() || tails[0] != s) return false;
          visit.alternative(a);
        }
      s = tails[0];
    }
  }
  return false;
}

struct AppendArcBase {
  std::vector<ArcBase*>& arcs;
  AppendArcBase(std::vector<ArcBase*>& arcs) : arcs(arcs) {}
  void operator()(ArcBase* a) const { arcs.push_back(a); }
  void alternative(ArcBase* a) const {}
};

template <class Visitor>
bool visitSimpleSentenceArcsLeftToRight(HypergraphBase const& hg, Visitor const& visit) {
  if (leftToRightSimpleSentenceVisit(hg))
    return visitSimpleSentenceArcs(hg, visit);
  else {
    std::vector<ArcBase*> arcs;
    bool ok = visitSimpleSentenceArcs(hg, AppendArcBase(arcs));
    if (!ok) return false;
    std::reverse(arcs.begin(), arcs.end());
    for (ArcBase* a : arcs) visit(a);
    return true;
  }
}

bool getSimpleSentenceWords(HypergraphBase const& hg, Syms& syms);

/// get arcs in left->right (start->final) order (whether in arcs or out arcs)
bool getSimpleSentenceArcs(std::vector<ArcBase*>& arcs, HypergraphBase const& hg);

/**
   \return length of the simple graph derivation in #arcs, else kNoState.
*/
StateId simpleSentenceLength(StateId& neps, HypergraphBase const& hg, bool allowSausage = kSinglePathOnly);
/// \return 'hg is a graph with only a single path, or state sequence for all paths if allowSausage'
bool isSimpleSentence(HypergraphBase const& hg, bool allowSausage = kSinglePathOnly);

/**
     given simpleSentenceLength != kNoState and sortStatesed-simple-sentence graph hg, make start state 0 -> 1
     -> ... max-nonlex(final)==simpleSentenceLength.

     \post hg is topo-sorted and first-tail outarcs only

     \return simpleSentenceLength(hg) (the new final state index)

     a version of this not requiring sorted input states would need to store a copy of outarcs for each state

  */
template <class Arc>
StateId compactSimpleSentence(IMutableHypergraph<Arc>& hg, StateIds& permutation, StateIds& inversePermutation) {
  assert(hg.isGraph());
  assert(hg.checkValid());
  assert(isSimpleSentence(hg, kAllowLabelAmbiguity));
  SDL_TRACE(Block.Hypergraph.SimpleSentence, "before first-tails->compact:\n" << hg);
  hg.forceFirstTailOutArcsOnly();
  assert(!hg.storesInArcs());
  SDL_TRACE(Block.Hypergraph.SimpleSentence, "after first-tails before compact:\n" << hg);
  StateId const end = hg.final();
  if (end == kNoState) return 0;
  inversePermutation.clear();
  inversePermutation.resize(end + 1, kNoState);
  StateId len = 0, s = hg.start();
  for (; s != end; ++len) {
    permutation.push_back(s);
    Util::atExpand(inversePermutation, s, kNoState) = len;
    ArcsContainer* outp = hg.maybeOutArcs(s);
    if (!outp || outp->empty() || (s = (*outp)[0]->head_) == kNoState) return 0;
  }
  assert(len == permutation.size());
  permutation.push_back(end);
  inversePermutation[end] = len;
  hg.orderGraphStates(permutation, inversePermutation);
  SDL_TRACE(Block.Hypergraph.SimpleSentence, "after compact:\n" << hg);
  hg.addUncomputedProperties(kAcyclic | kSortedStates);
  hg.setSortedStatesNumNotTerminal(len + 2);
  return len;
}

template <class Arc>
StateId compactSimpleSentence(IMutableHypergraph<Arc>& hg) {
  StateIds permutation, inversePermutation;
  return compactSimpleSentence(hg, permutation, inversePermutation);
}


}}

#endif
