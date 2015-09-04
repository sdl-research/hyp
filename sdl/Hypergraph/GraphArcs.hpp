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

    facilitate algorithms that traverse forward (out) or reverse (in) arc
    adjacencies of a graph (hg that .isgraph())

    GraphFirstTailOutArcsCached will work whether or not the hg stores outarcs; the
    others work only for that arc storage type
*/

#ifndef GRAPHARCS_JG_2014_05_27_HPP
#define GRAPHARCS_JG_2014_05_27_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/HypergraphBase.hpp>

namespace sdl { namespace Hypergraph {

struct GraphInArcs {
  ArcBase *getArc(HypergraphBase const& hg, StateId s, ArcId a) const {
    return hg.inArc(s, a);
  }
  template <class Arc>
  Arc *getArc(IHypergraph<Arc> const& hg, StateId s, ArcId a) const {
    return hg.inArc(s, a);
  }
  ArcId getNumArcs(HypergraphBase const& hg, StateId s) const {
    return hg.numInArcs(s);
  }
  template <class Arc>
  static inline StateId from(Arc const* a) {
    return a->head();
  }
  template <class Arc>
  static inline StateId to(Arc const* a) {
    assert(a->tails().size());
    return a->tails()[0];
  }
  static inline StateId from(HypergraphBase const& hg) {
    return hg.final();
  }
  static inline StateId to(HypergraphBase const& hg) {
    return hg.start();
  }
};

struct GraphOutArcs {
  ArcBase *getArc(HypergraphBase const& hg, StateId s, ArcId a) const {
    return hg.outArc(s, a);
  }
  template <class Arc>
  Arc *getArc(IHypergraph<Arc> const& hg, StateId s, ArcId a) const {
    return hg.outArc(s, a);
  }
  ArcId getNumArcs(HypergraphBase const& hg, StateId s) const {
    return hg.numOutArcs(s);
  }
  template <class Arc>
  static inline StateId from(Arc const* a) {
    assert(a->tails().size());
    return a->tails()[0];
  }
  template <class Arc>
  static inline StateId to(Arc const* a) {
    return a->head();
  }
  static inline StateId from(HypergraphBase const& hg) {
    return hg.start();
  }
  static inline StateId to(HypergraphBase const& hg) {
    return hg.final();
  }
};

struct GraphFirstTailOutArcsCached : GraphOutArcs {
  typedef HypergraphBase::Adjs Adjs;
  typedef HypergraphBase::AdjsPtr AdjsPtr;
  AdjsPtr adjs;
  StateId const nadjs;
  GraphFirstTailOutArcsCached(HypergraphBase const& hg)
      : adjs(hg.getFirstTailOutArcs())
      , nadjs(adjs->size())
  {}
  ArcBase *getArc(HypergraphBase const& hg, StateId s, ArcId a) const {
    return (*adjs)[s][a];
  }
  ArcId getNumArcs(HypergraphBase const& hg, StateId s) const {
    return s < nadjs ? (*adjs)[s].size() : 0;
  }
};

inline Sym firstGraphLabel(HypergraphBase const& hg, ArcBase *a) {
  StateIdContainer const& tails = a->tails_;
  return tails.size() > 1 ? hg.inputLabel(tails[1]) : NoSymbol;
}


}}

#endif
