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
    return a->weight() < b->weight(); // TODO: use Hypergraph::less
  }
};

struct EqualByWeight {
  template <class X>
  bool operator()(X const* a, X const* b) const {
    return a->weight() == b->weight();
  }
};

template<class Arc>
struct CmpFsmArcInput {
  const IHypergraph<Arc>& fst;
  CmpFsmArcInput(const IHypergraph<Arc>& fst)
      : fst(fst) {}

  /**
     sort together arcs w/ same input label. among those, the lowest cost (highest prob) come first.
  */
  bool operator()(Arc const* a, Arc const* b) const {
    Sym labelA = fst.inputLabel(a->fsmSymbolState());
    Sym labelB = fst.inputLabel(b->fsmSymbolState());
    return labelA<labelB || (labelA == labelB && a->weight() < b->weight());
  }
};

/**
   for lazy compose, better to have ties broken by weight.
*/
template<class Arc>
struct CmpFsmArcInputBestFirst {
  const IHypergraph<Arc>& fst;
  CmpFsmArcInputBestFirst(const IHypergraph<Arc>& fst)
      : fst(fst) {}

  /**
     sort together arcs w/ same input label. among those, the lowest cost (highest prob) come first.
  */
  bool operator()(Arc const* a, Arc const* b) const {
    Sym labelA = fst.inputLabel(a->fsmSymbolState());
    Sym labelB = fst.inputLabel(b->fsmSymbolState());
    return labelA<labelB || (labelA == labelB && a->weight() < b->weight());
  }
};

template<class Arc>
struct CmpFsmArcHead {
  bool operator()(Arc const* a, Arc const* b) const{
    return a->head() < b->head();
  }
};

template<class Arc>
struct IMutableHypergraph;

template<class Arc, class SortPolicy>
void sortArcsImpl(IMutableHypergraph<Arc>* hg, SortPolicy const& cmp){
  if (!hg->storesOutArcs()) SDL_THROW_LOG(Hypergraph, InvalidInputException, "sortArcs is for Fsm with out-arcs index.");
  if (hg->hasSortedArcs()) return;
  for(StateId sid=0, N=hg->size();sid<N;++sid) {
    typedef typename IMutableHypergraph<Arc>::ArcsContainer ArcsContainer;
    ArcsContainer *a = hg->maybeOutArcs(sid);
    if (a)
      std::sort(a->begin(), a->end(), cmp);
  }
}

template<class Arc>
void sortArcs(IMutableHypergraph<Arc>* pHg){
  if (!pHg->storesOutArcs())
    pHg->forceProperties(kStoreFirstTailOutArcs);
  CmpFsmArcInputBestFirst<Arc> cmp(*pHg);
  sortArcsImpl(pHg, cmp);
  pHg->addProperties(kSortedOutArcs); // (by input label, then weight
}

template<class Arc, class SortPolicy>
void sortArcs(IMutableHypergraph<Arc>* pHg, SortPolicy const& cmp){
  sortArcsImpl(pHg, cmp);
}

}}

#endif
