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
/* \file

   keep only part of an input hg (only some states, and only some arcs)

   this is a Transform: both inout and inplace

*/

#ifndef HYPERGRAPH_RESTRICT_HPP
#define HYPERGRAPH_RESTRICT_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/HypergraphCopy.hpp>
#include <sdl/Hypergraph/Transform.hpp>

namespace sdl {
namespace Hypergraph {

/// TODO: since a Restrict can only be used by one thread (it's more of a
/// temporary object) perhaps we should make it not a transform - as you can
/// see, the const-ness of Transform (intended to suggest putting mutable stuff
/// in per-thread objects or stack vars) is a sham here: everything is mutable
/// (note: we're careful not to use a multithreaded shared Restrict transform,
/// but it would be better to remove the Transform base entirely).
//: TransformBase<Transform::Inplace>
template <class A>
struct Restrict {
  enum { OptionalInplace = true };
  typedef IHypergraph<A> H;
  typedef A Arc;
  typedef typename A::ArcFilter Filter;

  mutable Filter keep;
  mutable bool clearRemap;
  Restrict() : clearRemap(true) {}

  explicit Restrict(Filter const& keep, bool clearRemap = true) : keep(keep), clearRemap(clearRemap) {}

  /// you must populate this if it's frozen, in which case it removes states not
  /// mapped. otherwise all states are mapped
  mutable StateIdTranslation stateRemap;

  bool trivial() const { return stateRemap.identity() && !keep; }

  void inout(IHypergraph<A> const& h, IMutableHypergraph<A>* o) const {
    assert(stateRemap);
    if (trivial())
      copyHypergraph(h, o);
    else {
      copyFilter(stateRemap, h, o, keep, kArcs, kStartFinal, kNoClear);
    }
  }
  void inplace(IMutableHypergraph<A>& m) const {
    assert(stateRemap);
    if (!trivial()) {
      m.restrict(stateRemap, keep);
    }
  }
  void complete() const {
    if (clearRemap) stateRemap.clear();
    keep = Filter();
  }
};


/// Prepare: CRTP inheritance
template <class Prepare, class A>
struct RestrictPrepare : Restrict<A> {
  bool samePropertiesOut;
  bool clearOut;
  RestrictPrepare() { samePropertiesOut = clearOut = true; }

  static inline bool isInplace(IHypergraph<A> const& h, IMutableHypergraph<A>& m) { return &h == &m; }

  /// CRTP inheritance of Prepare from this
  Prepare const* impl() const { return static_cast<Prepare const*>(this); }

  bool prepare(IHypergraph<A> const& h, IMutableHypergraph<A>& m) const {
    Prepare const* p = impl();
    if (clearOut && !isInplace(h, m)) {
      if (samePropertiesOut)
        m.clear(h.properties());
      else
        m.clear();
    }
    StateIdMapping* map = p->mapping(h, m);
    if (map) {
      this->stateRemap.resetNew(map);
      p->preparePost(h, m);
      return true;
    } else
      return false;
  }
  void preparePost(IHypergraph<A> const& h, IMutableHypergraph<A>& m) const {}
  StateIdMapping* mapping(IHypergraph<A> const& h, IMutableHypergraph<A>& m) const {
    assert(0);
    return 0;
  }
  void complete() const {
    Restrict<A>::complete();
    ((Prepare*)this)->completeImpl();
  }
  void completeImpl() {}
  enum { enableInplace = true };
  enum { Inplace = enableInplace, OptionalInplace = enableInplace };
  void inout(IHypergraph<A> const& h, IMutableHypergraph<A>* o) const {
    if (h.prunedEmpty()) return;
    prepare(h, *o);
    Restrict<A>::inout(h, o);
    complete();
  }
  bool needsRestrict(IHypergraph<A>& h) const { return true; }
  void inplace(IMutableHypergraph<A>& m) const {
    if (m.prunedEmpty()) return;
    if (impl()->needsRestrict(m)) {
      if (prepare(m, m)) {
        Restrict<A>::inplace(m);
        complete();
      } else
        m.setEmpty();
    }
  }
};


}}

#endif
