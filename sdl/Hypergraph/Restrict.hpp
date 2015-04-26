// Copyright 2014 SDL plc
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

template <class A>
struct Restrict : TransformBase<Transform::Inplace>
{
  typedef IHypergraph<A> H;
  typedef A Arc;
  typedef typename A::ArcFilter Filter;

  Filter keep;
  bool clearRemap;
  Restrict() : clearRemap(true) { }

  explicit Restrict(Filter const& keep, bool clearRemap = true)
      : keep(keep)
      , clearRemap(clearRemap)
  {}

  /// you must populate this if it's frozen, in which case it removes states not
  /// mapped. otherwise all states are mapped
  StateIdTranslation stateRemap;

  bool trivial() const
  {
    return stateRemap.identity() && !keep;
  }

  enum { Inplace = true, OptionalInplace = true };
  void inout(IHypergraph<A> const &h, IMutableHypergraph<A> *o)
  {
    assert(stateRemap);
    if (trivial())
      copyHypergraph(h, o);
    else {
      copyFilter(stateRemap, h, o, keep, kArcs, kStartFinal, kNoClear);
    }
  }
  void inplace(IMutableHypergraph<A> &m) {
    assert(stateRemap);
    if (!trivial()) {
      m.restrict(stateRemap, keep);
    }
  }
  void complete()
  {
    if (clearRemap)
      stateRemap.clear();
    keep = Filter();
  }
};


template <class Prepare, class A>
struct RestrictPrepare : Restrict<A>
{
  bool samePropertiesOut;
  bool clearOut;
  RestrictPrepare() {
    samePropertiesOut=clearOut=true;
  }
  static inline bool isInplace(IHypergraph<A> const&h, IMutableHypergraph<A> &m) {
    return &h==&m;
  }
  void prepare(IHypergraph<A> const &h, IMutableHypergraph<A> &m)
  {
    Prepare *p=(Prepare *)this;
    if (clearOut&&!isInplace(h, m)) {
      if (samePropertiesOut)
        m.clear(h.properties());
      else
        m.clear();
    }
    this->stateRemap.resetNew(p->mapping(h, m));
    p->preparePost(h, m);
  }
  void preparePost(IHypergraph<A> const &h, IMutableHypergraph<A> &m)
  {
  }
  StateIdMapping *mapping(IHypergraph<A> const &h, IMutableHypergraph<A> &m)
  {
    assert(0);
    return 0;
  }
  void complete()
  {
    Restrict<A>::complete();
    ((Prepare *)this)->completeImpl();
  }
  void completeImpl() {
  }
  enum { enableInplace=true };
  enum { Inplace=enableInplace, OptionalInplace=enableInplace };
  void inout(IHypergraph<A> const &h, IMutableHypergraph<A> *o)
  {
    if (h.prunedEmpty()) return;
    prepare(h,*o);
    Restrict<A>::inout(h, o);
    complete();
  }
  void inplace(IMutableHypergraph<A> &m) {
    if (m.prunedEmpty()) return;
    prepare(m, m);
    Restrict<A>::inplace(m);
    complete();
  }
};


}}

#endif
