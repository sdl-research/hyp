/** \file

    'copy' arcs: take all the arcs from one hg and append them to another (used
    to concatenate two hgs), optionally through a stateid translation.

    also, more efficient fns that special-case an identity (or subset)
    StateIdTranslation
*/

#ifndef HYP__HYPERGRAPH_COPY_HPP
#define HYP__HYPERGRAPH_COPY_HPP
#pragma once


#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/StateIdTranslation.hpp>
#include <sdl/Hypergraph/Exception.hpp>
#include <sdl/Util/Once.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>


namespace sdl {
namespace Hypergraph {

VERBOSE_EXCEPTION_DECLARE(NonConstStateIdTranslation)

template <class A>
void copyStartFinal(StateIdTranslation &stateRemap, IHypergraph<A> const& from, IMutableHypergraph<A> *to, ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  if (clearAndSameProperties==kClearAndSameProperties)
    to->clear(from.properties());
  to->offerVocabulary(from);
  to->setStart(stateRemap.stateFor(from.start()));
  to->setFinal(stateRemap.stateFor(from.final()));
}

template <class A>
void copyStartFinalSubset(StateIdTranslation & stateRemap, IHypergraph<A> const& from, IMutableHypergraph<A> *to, ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  assert(stateRemap.frozen());
  if (clearAndSameProperties==kClearAndSameProperties)
    to->clear(from.properties());
  to->offerVocabulary(from);
  to->setStart(stateRemap.existingState(from.start()));
  to->setFinal(stateRemap.existingState(from.final()));
}


template <class A>
struct CopyTo {
  StateIdTranslation &stateRemap;
  IMutableHypergraph<A> *o;
  typedef void result_type;
  CopyTo(StateIdTranslation &stateRemap, IMutableHypergraph<A> *o) : stateRemap(stateRemap), o(o) {}
  result_type operator()(A const* a) const {
    A *c = stateRemap.copyArc(*a);
    if (c) o->addArc(c);
  }
};

// exactly like CopyToSubset but uses ArcFilter keep to rule out some edges
template <class A>
struct CopyRestrict {
  typedef typename A::ArcFilter Filter;
  StateIdTranslation &stateRemap;
  IMutableHypergraph<A> *o;
  Filter keep;
  CopyRestrict(StateIdTranslation &stateRemap, IMutableHypergraph<A> *o, Filter const& keep = A::filterTrue())
      : stateRemap(stateRemap), o(o), keep(keep) {
    assert(stateRemap.stateAdding());
  }
  typedef void result_type;
  A a;
  CopyRestrict(StateIdTranslation &stateRemap, IMutableHypergraph<A> *o) : stateRemap(stateRemap), o(o) {}
  result_type operator()(A * a) const {
    if (keep(a)) {
      A *c = stateRemap.copyArc(*a);
      if (c) o->addArc(c);
    }
  }
};

template <class A>
void copyArcs(StateIdTranslation &stateRemap, IHypergraph<A> const& i, IMutableHypergraph<A> *o, typename A::ArcFilter const&keep = 0) {
  if (keep)
    i.forArcs(CopyRestrict<A>(stateRemap, o, keep));
  else
    i.forArcs(CopyTo<A>(stateRemap, o));
}

enum CopyArcs { kNoArcs = 0, kArcs = 1 };
enum CopyStartFinal { kNoStartFinal = 0, kStartFinal = 1 };

template <class A>
void copySubset(StateIdTranslation &stateRemap, IHypergraph<A> const& i, IMutableHypergraph<A> *o, CopyArcs arcs = kArcs, CopyStartFinal startFinal = kStartFinal, ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  stateRemap.freeze();
  if (startFinal==kStartFinal)
    copyStartFinalSubset(stateRemap, i, o, clearAndSameProperties);
  if (arcs==kArcs)
    i.forArcs(CopyTo<A>(stateRemap, o));
  stateRemap.transferLabels(i, *o);
}

template <class A>
void copyFilter(StateIdTranslation &stateRemap, IHypergraph<A> const& i, IMutableHypergraph<A> *o, typename A::ArcFilter const& keep = 0, CopyArcs arcs = kArcs, CopyStartFinal startFinal = kStartFinal, ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  if (startFinal==kStartFinal)
    copyStartFinal(stateRemap, i, o, clearAndSameProperties);
  if (arcs==kArcs)
    copyArcs(stateRemap, i, o, keep);
  stateRemap.transferLabels(i, *o);
}

template <class A>
void copy(StateIdTranslation &stateRemap, IHypergraph<A> const& i, IMutableHypergraph<A> *o, CopyArcs arcs = kArcs, CopyStartFinal startFinal = kStartFinal) {
  copyFilter(stateRemap, i, o, 0, arcs, startFinal);
}


}}

#endif
