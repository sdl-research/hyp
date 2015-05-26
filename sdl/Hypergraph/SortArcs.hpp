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

    prepare arcs best-first for lazy compose.
*/


#ifndef HYPERGRAPH_SORTARCS_HPP
#define HYPERGRAPH_SORTARCS_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <boost/function.hpp>

#include <algorithm>

#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Forall.hpp>

namespace sdl {
namespace Hypergraph {

struct CmpByWeight {
  template <class X>
  bool operator()(X const* a, X const* b) const {
    return a->weight() < b->weight();  // TODO: use Hypergraph::less
  }
};

struct EqualByWeight {
  template <class X>
  bool operator()(X const* a, X const* b) const {
    return a->weight() == b->weight();
  }
};

template <class Arc>
struct CmpFsmArcInput {
  IHypergraph<Arc> const& fst;
  CmpFsmArcInput(IHypergraph<Arc> const& fst) : fst(fst) {}

  /**
     sort together arcs w/ same input label. among those, the lowest cost (highest prob) come first.
  */
  bool operator()(Arc const* a, Arc const* b) const {
    Sym labelA = fst.inputLabel(a->fsmSymbolState());
    Sym labelB = fst.inputLabel(b->fsmSymbolState());
    return labelA < labelB || (labelA == labelB && a->weight() < b->weight());
  }
};

/**
   for lazy compose, better to have ties broken by weight.
*/
template <class Arc>
struct CmpFsmArcInputBestFirst {
  IHypergraph<Arc> const& fst;
  CmpFsmArcInputBestFirst(IHypergraph<Arc> const& fst) : fst(fst) {}

  /**
     sort together arcs w/ same input label. among those, the lowest cost (highest prob) come first.
  */
  bool operator()(Arc const* a, Arc const* b) const {
    Sym labelA = fst.inputLabel(a->fsmSymbolState());
    Sym labelB = fst.inputLabel(b->fsmSymbolState());
    return labelA < labelB || (labelA == labelB && a->weight() < b->weight());
  }
};

template <class Arc>
struct CmpFsmArcHead {
  bool operator()(Arc const* a, Arc const* b) const { return a->head() < b->head(); }
};

template <class Arc>
struct IMutableHypergraph;

template <class Arc, class SortPolicy>
void sortArcsImpl(IMutableHypergraph<Arc>* hg, SortPolicy const& cmp) {
  if (!hg->storesOutArcs())
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "sortArcs is for Fsm with out-arcs index.");
  if (hg->hasSortedArcs()) return;
  for (StateId sid = 0, N = hg->size(); sid < N; ++sid) {
    typedef typename IMutableHypergraph<Arc>::ArcsContainer ArcsContainer;
    ArcsContainer* a = hg->maybeOutArcs(sid);
    if (a) std::sort(a->begin(), a->end(), cmp);
  }
}

template <class Arc>
void sortArcs(IMutableHypergraph<Arc>* pHg) {
  if (!pHg->storesOutArcs()) pHg->forceProperties(kStoreFirstTailOutArcs);
  CmpFsmArcInputBestFirst<Arc> cmp(*pHg);
  sortArcsImpl(pHg, cmp);
  pHg->addProperties(kSortedOutArcs);  // (by input label, then weight
}

template <class Arc, class SortPolicy>
void sortArcs(IMutableHypergraph<Arc>* pHg, SortPolicy const& cmp) {
  sortArcsImpl(pHg, cmp);
}

/// phrasedecoder rules may have constraint syms; single-source constraint-sub
/// rules get matched exhaustively (we don't try to fold them into sort. TODO:
/// if somebody has ensured that arcs w/ constraint sub start with that sym we
/// can include it in sort too and more uniformly match
template <class Arc>
struct FirstLexicalAscending {
  IMutableHypergraph<Arc> const& hg;
  explicit FirstLexicalAscending(IMutableHypergraph<Arc> const& hg) : hg(hg) {}
  bool operator()(Arc const* a, Arc const* b) const {
    return hg.firstLexicalInput(a) < hg.firstLexicalInput(b);
  }
  bool operator()(Sym a, Arc const* b) const { return a < hg.firstLexicalInput(b); }
  bool operator()(Arc const* a, Sym b) const { return hg.firstLexicalInput(a) < b; }
  Sym operator()(Arc const* a) const {
    return hg.firstLexicalInput(a);
  }
};

template <class Arc, class Less>
void sortInArcs(IMutableHypergraph<Arc> & hg, Less const& less) {
  hg.forceProperties(kStoreInArcs);
  for (StateId i = 0, N = hg.size(); i < N; ++i) {
    typename IMutableHypergraph<Arc>::ArcsContainer* a = hg.maybeInArcs(i);
    if (a) std::sort(a->begin(), a->end(), less);
  }
}
template <class Arc>
void sortInArcsFirstLexical(IMutableHypergraph<Arc> & hg) {
  sortInArcs(hg, FirstLexicalAscending<Arc>(hg));
}

template <class Arc>
std::pair<Arc const**, Arc const**> findLexSortedInArcs(IMutableHypergraph<Arc> const& hg, StateId head,
                                                      Sym firstLexical) {
  typename IMutableHypergraph<Arc>::ArcsContainer* a = hg.maybeInArcs(head);
  assert(a);
  return std::equal_range(a->begin(), a->end(), firstLexical, FirstLexicalAscending<Arc>(hg));
}

template <class Hg, class ArcsContainer>
typename ArcsContainer::const_iterator findNonlexSortedInArcsBegin(Hg const& hg, ArcsContainer const& a) {
  // FirstLexicalAscending puts @NoSymbol position (-1) so at end
  typename ArcsContainer::const_iterator b = a.begin(), e = a.end();
  if (b != e) {
    while (--e >= b)
      if (hg.firstLexicalInput(*e)) break;
    ++e;
  }
  return e;
}

template <class Arc>
std::pair<Arc const**, Arc const**> findNonlexSortedInArcs(IMutableHypergraph<Arc> const& hg, StateId head) {
  // FirstLexicalAscending puts @NoSymbol position (-1) so at end
  typename IMutableHypergraph<Arc>::ArcsContainer* a = hg.maybeInArcs(head);
  assert(a);
  return std::pair<Arc const**, Arc const**>(findNonlexSortedInArcsBegin(hg, *a), a->end());
}


}}

#endif
