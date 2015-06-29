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

    rearrange the states of a hypergraph into two parts, (terminal labeled, and
    not - nonterm meaning not terminal labeled, not meaning having a nonterminal
    label, or in topological (bottom up or top down) order
*/


#ifndef HYPERGRAPH_SORTSTATES_HPP
#define HYPERGRAPH_SORTSTATES_HPP
#pragma once

#include <sdl/Hypergraph/Restrict.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Hypergraph/StatesTraversal.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>
#include <sdl/Hypergraph/Visit.hpp>
#include <sdl/Config/Validate.hpp>

namespace sdl {
namespace Hypergraph {

enum SortOrder {
  // TODO: kNone for optional sorting?
  kTerminalFirst = 1,
  kTerminalLast,
  kTopSort,  // final state goes at end, after antecedents (tails) are fully processed. start state will be
  // at state 0 iff every state is reachable from start
  kSortOrderEnd,
  kSortOrderBegin = kTerminalFirst
};

// TODO: provide topo sort of reverse for Fsm - then we don't need in arcs
struct SortStatesOptions {
  bool topological() const { return sortOrder == kTopSort; }
  SortOrder sortOrder;
  bool canonicalLex;
  bool stable;
  /**
     because SortStates(&hg) makes a SortStatesOptions(), we can't defer initialization until configure.
  */
  SortStatesOptions(SortOrder sortOrder = kTopSort) : sortOrder(sortOrder) {
    stable = false;
    canonicalLex = false;
  }
#define kSortStatesOptionsSortOrderString                     \
  "1 = renumber states so lexical-labeled states are first, " \
  "2 = renumber states so lexical-labeled states are last, "  \
  "3 = topological sort (final state comes last, lexical-labeled states after that) "
  template <class Conf>
  void configure(Conf& c) {
    c.is("state sorting options");
    c("sort-order", &sortOrder)
        .self_init()(kSortStatesOptionsSortOrderString)
        .validate(Config::boundedRangeInclusive(1, 3, kSortStatesOptionsSortOrderString));
    c("canonical-lex", &canonicalLex)
        .self_init()(
            "for copying transform, ensure lexical-labeled states are reused (TODO: implement for in-place)");
    c("stable", &stable)
        .self_init()("don't sort if already sorted (ensures a saved sorted input will have stable state ids");
  }
  void validate() const { Config::boundedRangeInclusive(1, 4, kSortStatesOptionsSortOrderString)(sortOrder); }
  friend inline void validate(SortStatesOptions& x) { x.validate(); }
};


struct SortStatesMapping : public StateIdMapping {
  StateSet lexical;
  bool lexicalFirst;
  StateId nextLex, nextRest;
  template <class A>
  SortStatesMapping(IHypergraph<A> const& h, bool lexicalFirst)
      : lexical(h.size()), lexicalFirst(lexicalFirst) {
    StateId nlex = 0, N = lexical.size();
    assert(N == h.size());
    for (StateId s = 0; s < N; ++s)
      if (h.hasTerminalLabel(s)) {
        ++nlex;
        lexical.set(s);
      }
    assert(nlex == lexical.count());
    if (lexicalFirst) {
      nextLex = 0;
      nextRest = nlex;
    } else {
      nextLex = N - nlex;
      nextRest = 0;
    }
  }
  virtual StateId remap(StateId s) OVERRIDE { return remapImpl(s); }
  StateId remapImpl(StateId s) {
    assert(s < lexical.size());
    return Util::contains(lexical, s) ? nextLex++ : nextRest++;
  }
};


/**
   since topo sort .
*/

template <bool HeadAfterTails>
struct CheckTopo {
  CheckTopo(StateId endTailState) : endTailState(endTailState) {}
  StateId endTailState;

  /**
     \return whether Arc a obeys (reverse) topo sort

     for ignoring lexical tails, we only compare tails < endTailState.
  */
  template <class Arc>
  bool operator()(IHypergraph<Arc> const& hg, Arc* a) const {
    StateId head = a->head();
    StateIdContainer const& tails = a->tails();
    forall (StateId tail, tails) {
      if (tail < endTailState) {
        bool const violation = HeadAfterTails ? head <= tail : tail <= head;
        if (violation) return false;
      }
    }
    return true;
  }
};

template <bool HeadAfterTails>
struct CheckTopoSkipAxioms {
  /**
     \return whether Arc a obeys (reverse) topo sort

     for ignoring lexical tails, we check hg.isAxiom and skip those
  */
  template <class Arc>
  bool operator()(IHypergraph<Arc> const& hg, Arc* a) const {
    StateId head = a->head();
    StateIdContainer const& tails = a->tails();
    forall (StateId tail, tails) {
      if (!hg.isAxiom(tail)) {
        bool const violation = HeadAfterTails ? head <= tail : tail <= head;
        if (violation) return false;
      }
    }
    return true;
  }
};

/**
   \return whether reachable states from sid are already topo-sorted. (not necessarily nonterm-first)

   TODO (speed): early return manual enumeration of arcs for states, and perhaps combination with
   findBoundaryBetweenNotAndIsTerminal
*/
template <class Arc>
bool isTopoSort(IHypergraph<Arc> const& hg, bool requireHeadAfterTails = true, StateId nontermEnds = kNoState) {
  if (nontermEnds == kNoState) {
    if (requireHeadAfterTails)
      return visitArcsAtLeastOnce(hg, CheckTopoSkipAxioms<true>());
    else
      return visitArcsAtLeastOnce(hg, CheckTopoSkipAxioms<false>());
  } else if (requireHeadAfterTails)
    return visitArcsAtLeastOnce(hg, CheckTopo<true>(nontermEnds));
  else
    return visitArcsAtLeastOnce(hg, CheckTopo<false>(nontermEnds));
}


/**
   \return kNoState if states aren't split to nonterm first then lex, else return id such that [0, id) are
   nonterm and [id, size) are lex.
*/
inline StateId findBoundaryBetweenNotAndIsTerminal(IHypergraphStates const& hg) {
  StateId N = hg.size();
  IVocabularyPtr voc = hg.getVocabulary();
  for (StateId s = 0; s < N; ++s) {
    Sym i = hg.inputLabel(s);
    if (i.isTerminal()) {  // then s is the first lexical state
      for (StateId t = s + 1; t < N; ++t) {
        Sym i = hg.inputLabel(t);
        if (!i.isTerminal()) return kNoState;
      }
      return s;  // only if all the s...end are lexical
    }
  }
  return N;  // none were lexical
}

/**
   \return whether states aren't yet sorted per sortOrder. quick check for
   kTopSort, else only check if stable == true.

   \param[out] partBoundary - if states are already sorted then this is the
   boundary between the two types of states.
*/
template <class Arc>
inline bool unsortedStates(IHypergraph<Arc> const& hg, SortOrder sortOrder, StateId& partBoundary,
                           bool stable = false) {
  if (sortOrder == kTopSort && hasProperties(hg.properties(), kSortedStates)) {
    partBoundary = hg.numNotTerminalStates();
    return false;
  }
  if (stable)  // checking for already sorted is not worth the time, unless stable sorting was requested
    switch (sortOrder) {
      case kTerminalFirst:
        return (partBoundary = findBoundaryBetweenNotAndIsTerminal(hg)) == kNoState;
      case kTopSort:
        return !isTopoSort(hg) || (partBoundary = findBoundaryBetweenNotAndIsTerminal(hg)) == kNoState;
      default:
        partBoundary = kNoState;
    }
  // stable sort is only supported for topo sort and lexical-first
  return true;
}


/// in general, a partial map from old states to new states. puts nontermical states first in topo or reverse
/// topo order, then lexical states. may remove nonterm states that aren't connected to final, in which case
/// there will be a gap between lexical and nontermical states, or (if reverse) state ids will start at k>0.
template <class A>
struct TopNontermOrder : IStatesVisitor {
  std::vector<StateId> unmappedLexStates;
  StateIdTranslation* pDestState;  // either adds states (inout) or reassign states (inplace)
  IHypergraph<A> const* ph;
  TopNontermOrder() {}
  TopNontermOrder(IHypergraph<A> const& hg, StateIdTranslation* pDestState_, bool reverseTopSort,
                  bool canonLex) {
    set(hg, pDestState_, reverseTopSort);
  }

  StateId firstLexSt;
  // returns first lexical state id
  StateId set(IHypergraph<A> const& hg, StateIdTranslation* pDestState_, bool reverseTopSort, bool canonLex) {
    firstLexSt = 0;
    pDestState = pDestState_;
    ph = &hg;
    StateId N = hg.size();
    unmappedLexStates.clear();
    unmappedLexStates.reserve(N);
    if (reverseTopSort) {
      ReverseTopsortStatesTraversal<A>(hg, this);
    } else {
      TopsortStatesTraversal<A>(hg, this);
    }
    for (std::vector<StateId>::const_iterator i = unmappedLexStates.begin(), e = unmappedLexStates.end();
         i != e; ++i) {
      StateId lexst = *i;
      if (canonLex)
        pDestState->addState(*i, ph->labelPair(lexst));
      else
        pDestState->stateFor(lexst);
    }
    return firstLexSt;
  }
  virtual void visit(StateId sid) OVERRIDE {
    if (ph->hasTerminalLabel(sid))
      unmappedLexStates.push_back(sid);
    else {
      ++firstLexSt;
      pDestState->stateFor(sid);
    }
  }
};

// RestrictPrepare(restrict.hpp) is a Transform template that helps fill out and use a
// StateIdTranslation and offer both inout and inplace.
template <class A>
struct SortStates : public RestrictPrepare<SortStates<A>, A> {
  static char const* type() { return "SortStates"; }
  SortStatesOptions opt;
  SortStates(SortStatesOptions const& opt = SortStatesOptions(), StateId partBoundaryFromNeeds = kNoState)
      : opt(opt), partBoundary(partBoundaryFromNeeds) {
    opt.validate();
    this->clearOut = false;
  }
  mutable StateId partBoundary;

  void preparePost(IHypergraph<A> const& h, IMutableHypergraph<A>& m) const {
    bool inplace = this->isInplace(h, m);
    bool canonLex = !inplace && opt.canonicalLex;  // TODO: support for inplace also
    if (canonLex) m.forceCanonicalLex();
    StateIdTranslation& x = this->stateRemap;
    if (opt.topological()) {
      TopNontermOrder<A> order;
      partBoundary = order.set(h, &x, false, canonLex);
      m.setSortedStatesNumNotTerminal(partBoundary);
      m.addProperties(kSortedStates);
    } else {
      StateId N = h.size();
      if (canonLex) {  // todo: support for inplace also
        if (opt.sortOrder == kTerminalFirst) {
          std::vector<StateId> lexs(N);
          StateId nlex = 0;
          for (StateId s = 0; s != N; ++s)
            if (h.hasTerminalLabel(s))
              lexs[nlex++] = s;
            else
              x.stateFor(s);
          partBoundary = N - nlex;
          assert(partBoundary == x.size());
          for (StateId i = 0; i < nlex; ++i) {
            StateId s = lexs[i];
            x.addState(s, h.labelPair(s));
          }
        } else if (opt.sortOrder == kTerminalLast) {
          for (StateId s = 0; s != N; ++s)
            if (h.hasTerminalLabel(s)) x.addState(s, h.labelPair(s));
          partBoundary = m.size();
          for (StateId s = 0; s != N; ++s) x.stateFor(s);
        }
      } else {
        for (StateId s = 0; s != N; ++s)
          if (h.hasTerminalLabel(s)) {
            if (opt.sortOrder == kTerminalFirst) x.stateFor(s);
          } else if (opt.sortOrder == kTerminalLast)
            x.stateFor(s);
        partBoundary = (StateId)x.size();
        for (StateId s = 0; s != N; ++s) x.stateFor(s);
      }
    }
    x.freeze();
  }

  StateIdMapping* mapping(IHypergraph<A> const& h, IMutableHypergraph<A>& m) const {
    if (this->isInplace(h, m))
      return new ConsecutiveIdMapping();
    else {
      m.clear(h.properties());
      return new StateAddMapping<A>(&m);
    }
  }
};

/**
   sorts states in topological order by default

   \return The StateId for which [0, index) is terminal or (not terminal)
   only.
   *
   */
template <class A>
StateId sortStates(IMutableHypergraph<A>& h, SortStatesOptions const& opt = SortStatesOptions()) {
  StateId boundary;
  bool needs = unsortedStates(h, opt.sortOrder, boundary, opt.stable);
  if (needs) {
    SortStates<A> ss(opt, needs);
    ss.inplace(h);
    return ss.partBoundary;
  } else
    return boundary;
}

/**
   sorts states in topological order by default

   \return The StateId for which [0, index) is terminal or (not terminal)
   only.
   *
   */
template <class A>
StateId sortStates(IHypergraph<A> const& h, IMutableHypergraph<A>* out,
                   SortStatesOptions const& opt = SortStatesOptions()) {
  SortStates<A> ss(opt, kNoState);
  ss.inout(h, out);
  return ss.partBoundary;
}

struct SortStatesTransform : SortStatesOptions, TransformBase<Transform::Inplace> {
  static char const* type() { return "SortStates"; }

  enum { OptionalInplace = true };

  bool needs(IHypergraphStates const& hg) const {
    return !(sortOrder == kTopSort && hasProperties(hg.properties(), kSortedStates));
  }
  template <class Arc>
  bool needs(IHypergraph<Arc> const& hg) const {
    return needs((IHypergraphStates const&)hg);
  }

  SortStatesTransform(SortStatesOptions const& opt = SortStatesOptions()) : SortStatesOptions(opt) {}
  template <class Arc>
  void inout(IHypergraph<Arc> const& h, IMutableHypergraph<Arc>* o) const {
    sortStates(h, o, *this);
  }
  template <class Arc>
  void inplace(IMutableHypergraph<Arc>& m) const {
    sortStates(m, *this);
  }
};


/**
   \return whether this is an acyclic fsm

   TODO: much more efficient algorithm is possible.
*/
template <class Arc>
bool isAcyclicBySortStates(IHypergraph<Arc> const& hg) {
  if (hasProperties(hg.properties(), kSortedStates))
    return isTopoSort(hg);
  else {
    MutableHypergraph<Arc> sorted((kStoreInArcs));
    SortStates<Arc> sorter;
    sorter.inout(hg, &sorted);
    return isTopoSort(sorted, true, sorter.partBoundary);
  }
}


}}

#endif
