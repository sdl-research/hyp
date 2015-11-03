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

    similar to hg.forArcs but possibly ending early

    TODO: hg.forArcs could also be free fns like these.

    convenience functions for visiting the specified arcs or states. since the
    visitor functor is templated, it's not possible to offer more efficient
    virtual overrides for e.g. IMutableHypergraph - everything goes through the
    public interface

    if fsm stores out arcs only, and you don't mind visiting the same arc
    repeatedly (your visitor is fast), that will save tracking the set of
    visited arcs.

    if your fsm stores in arcs, this gives visit-each-arc-once easily.

    all visitors return true (meaning continue visiting more) or false, and the
    visiting functions provided here return \forall (true if all visits returned
    true). since the visitor code is template-based, the compiler should be able
    to optimize away an always-true return value, so there's no point providing
    a void-returning-visitor version.

    A visitor is V const& v, so you can put mutable members in a visitor to
    return additional (non-bool) values, and a visitor won't be copied.

    arcs are visited as arcvisitor(hg, Arc *arc) - the tail or head state id
    that triggered the visit isn't passed on

    states are visited as statevisitor(hg, StateId s)

    the hypergraph argument is passed as a courtesy since you'll want it in many
    cases. if your visitor doesn't care, it can ignore it. in the interest of
    not repeating ourselves, there aren't versions taking a non-const ref to hg,
    but it's of course safe to cast away const-ness appropriately
    e.g. (IMutableHypergraph projectOutput which modifies state labels)

    arc visiting methods allowing duplicates have name ending in 'AtLeastOnce'

*/

#ifndef VISITHYPERGRAPH_JG_2013_06_12_HPP
#define VISITHYPERGRAPH_JG_2013_06_12_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/Once.hpp>

namespace sdl {
namespace Hypergraph {

template <class Arc>
struct IHypergraph;

/**
   state visitors:

  bool stateVisit(IHypergraph<Arc> const& hg, StateId s) const;
*/

template <class Arc, class V>
bool visitStates(IHypergraph<Arc> const& hg, V const& v) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (!v(hg, s)) return false;
  return true;
}

template <class Arc, class V>
bool visitAxiomStates(IHypergraph<Arc> const& hg, V const& v) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (hg.isAxiom(s) && !v(hg, s)) return false;
  return true;
}

template <class Arc, class V>
bool visitTerminalStates(IHypergraph<Arc> const& hg, V const& v) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (hg.hasTerminalLabel(s) && !v(hg, s)) return false;
  return true;
}

template <class Arc, class V>
bool visitNotTerminalStates(IHypergraph<Arc> const& hg, V const& v) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (!hg.hasTerminalLabel(s) && !v(hg, s)) return false;
  return true;
}

template <class Arc, class V>
bool visitLexicalStates(IHypergraph<Arc> const& hg, V const& v) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (hg.hasLexicalLabel(s) && !v(hg, s)) return false;
  return true;
}

template <class Arc, class V>
bool visitLabeledStates(IHypergraph<Arc> const& hg, V const& v) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (hg.hasLabel(s) && !v(hg, s)) return false;
  return true;
}


/**
   arc visitors:

  bool arcVisit(IHypergraph<Arc> const& hg, Arc *arc) const;

   as soon as a visitor returns false, we stop.

   TODO: optimized (MutableHypergraph) versions?
*/

template <class Arc, class V>
bool visitArcsOut(IHypergraph<Arc> const& hg, StateId s, V const& v) {
  for (ArcId a = 0, N = hg.numOutArcs(s); a < N; ++a)
    if (!v(hg, hg.outArc(s, a))) return false;
  return true;
}

template <class Arc, class V>
bool visitArcsIn(IHypergraph<Arc> const& hg, StateId s, V const& v) {
  for (ArcId a = 0, N = hg.numInArcs(s); a < N; ++a)
    if (!v(hg, hg.inArc(s, a))) return false;
  return true;
}

/// once may be pre-filled to exclude some arcs.
template <class Arc, class V>
bool visitArcsOutOnce(IHypergraph<Arc> const& hg, StateId s, V const& v, Util::Once& once) {
  for (ArcId a = 0, N = hg.numOutArcs(s); a < N; ++a) {
    Arc* arc = hg.outArc(s, a);
    if (once.first(arc) && !v(hg, arc)) return false;
  }
  return true;
}

/// once may be pre-filled to exclude some arcs.
template <class Arc, class V>
bool visitArcsInOnce(IHypergraph<Arc> const& hg, StateId s, V const& v, Util::Once& once) {
  for (ArcId a = 0, N = hg.numInArcs(s); a < N; ++a) {
    Arc* arc = hg.inArc(s, a);
    if (once.first(arc) && !v(hg, arc)) return false;
  }
  return true;
}

/// once may be pre-filled to exclude some arcs.
template <class Arc, class V>
bool visitArcsOutOnce(IHypergraph<Arc> const& hg, V const& v, Util::Once& once) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (!visitArcsOutOnce(hg, s, v, once)) return false;
  return true;
}

/// once may be pre-filled to exclude some arcs.
template <class Arc, class V>
bool visitArcsInOnce(IHypergraph<Arc> const& hg, V const& v, Util::Once& once) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (!visitArcsInOnce(hg, s, v, once)) return false;
  return true;
}

/// visits each arc as many times as it has tails if kStoreOutArcs, or just once if kStoreFirstTailOutArcs
template <class Arc, class V>
bool visitArcsOutAtLeastOnce(IHypergraph<Arc> const& hg, V const& v) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (!visitArcsOut(hg, s, v)) return false;
  return true;
}

template <class Arc, class V>
bool visitArcsOut(IHypergraph<Arc> const& hg, V const& v) {
  Util::Once once;
  return visitArcsOutOnce(hg, v, once);
}

template <class Arc, class V>
bool visitArcsIn(IHypergraph<Arc> const& hg, V const& v) {
  for (StateId s = 0, N = hg.size(); s < N; ++s)
    if (!visitArcsIn(hg, s, v)) return false;
  return true;
}

/// we don't care about repeated visits to same arc (which will happen if we
/// store out arcs and not in); calls v(arc) until false is returned
template <class Arc, class V>
bool visitArcsAtLeastOnce(IHypergraph<Arc> const& hg, V const& v) {
  return hg.storesInArcs() ? visitArcsIn(hg, v) : visitArcsOutAtLeastOnce(hg, v);
}


template <class Arc, class V>
bool visitArcs(IHypergraph<Arc> const& hg, V const& v) {
  Properties p = hg.properties();
  return hg.forArcs(v);
  (p & kStoreInArcs) ? visitArcsIn(hg, v) : (p & kStoreFirstTailOutArcs) ? visitArcsOutAtLeastOnce(hg, v)
                                                                         : visitArcsOut(hg, v);
}

template <class V>
struct AcceptArc {
  V const* pvisit;
  AcceptArc() : pvisit() {}
  AcceptArc(V const* pvisit) : pvisit(pvisit) {}
  template <class Arc>
  bool operator()(IHypergraph<Arc> const&, Arc* a) const {
    (*pvisit)(a);
    return true;
  }
};

template <class V>
struct MutableAcceptArc {
  V* pvisit;
  MutableAcceptArc() : pvisit() {}
  MutableAcceptArc(V& pvisit) : pvisit(&pvisit) {}
  MutableAcceptArc(V* pvisit) : pvisit(pvisit) {}
  template <class Arc>
  bool operator()(IHypergraph<Arc> const&, Arc* a) const {
    (*pvisit)(a);
    return true;
  }
};

/**
  void visitArc(Arc * arc) adapter for visitArcs
*/
template <class V>
AcceptArc<V> acceptArc(V const* visitArc) {
  return AcceptArc<V>(visitArc);
}

/**
  void visitArc(Arc * arc) adapter for visitArcs
*/
template <class V>
MutableAcceptArc<V> mutableAcceptArc(V& visitArc) {
  return MutableAcceptArc<V>(visitArc);
}


}}

#endif
