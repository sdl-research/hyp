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

    weighted pruning; only meaningful for Viterbi weight.

    an acyclic sum (LogWeight) pruning using InsideAlgorithm would be fine, but "within delta of best" won't
   mean what it does for viterbi
*/

#ifndef HYP__HYPERGRAPH_PRUNE_HPP
#define HYP__HYPERGRAPH_PRUNE_HPP
#pragma once

#include <sdl/Hypergraph/Restrict.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Hypergraph/Transform.hpp>

namespace sdl {
namespace Hypergraph {

struct PruneTransform;

struct PruneOptions : TransformOptionsBase {
  template <class Arc>
  struct TransformFor {
    typedef PruneTransform type;
  };
  bool packStates;
  PruneOptions() { defaults(); }
  void defaults() { packStates = true; }
  static char const* caption() { return "Hypergraph pruning options"; }
  static char const* type() { return "Prune"; }

  template <class Conf>
  void configure(Conf& c) {
    c.is("Prune");
    c("pack-states", &packStates).self_init()("renumber remaining states to have contiguous ids");
  }
};

struct ArcInStateSet {
  StateSet const& ss;

  explicit ArcInStateSet(StateSet const& s) : ss(s) {}

  bool operator()(ArcBase* a) const {
    if (!Util::test(ss, a->head())) return false;
    StateIdContainer const& tails = a->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i)
      if (!Util::test(ss, *i)) return false;
    return true;
  }
};

/**
   Prunes away unreachable states and arcs.

   uses arc filter when you don't want to pack states
 */
template <class Arc>
struct PruneUnreachable : RestrictPrepare<PruneUnreachable<Arc>, Arc>, PruneOptions {
  static char const* type() { return "Prune"; }
  typedef RestrictPrepare<PruneUnreachable<Arc>, Arc> Base;
  typedef PruneOptions Config;
  PruneUnreachable(PruneOptions const& opt = PruneOptions()) : PruneOptions(opt) {}
  typedef shared_ptr<StateSet> PUseful;
  mutable PUseful pUseful;
  StateIdMapping* mapping(IHypergraph<Arc> const& h, IMutableHypergraph<Arc>& m) const {
    // TODO: make Reach return shared_ptr to subset only
    Reach r(h, true);
    pUseful = r.useful();
    ArcInStateSet s(*pUseful);
    this->keep = s;
    if (packStates) {
      return new StateAddMapping<Arc>(&m);
    } else {  // preserve stateids
      return new IdentityIdMapping();
    }
  }
  void completeImpl() const { pUseful.reset(); }
};

template <class Arc>
void pruneUnreachable(IMutableHypergraph<Arc>* pHg, PruneOptions const& opt = PruneOptions()) {
  PruneUnreachable<Arc> p(opt);
  p.inplace(*pHg);
}

template <class Arc>
void pruneUnreachable(IMutableHypergraph<Arc>& hg, PruneOptions const& opt = PruneOptions()) {
  pruneUnreachable(&hg, opt);
}

template <class Arc>
void pruneUnreachable(IHypergraph<Arc> const& hgInput, IMutableHypergraph<Arc>* pHgResult,
                      PruneOptions const& opt = PruneOptions()) {
  PruneUnreachable<Arc> p(opt);
  p.inout(hgInput, pHgResult);
}

struct PruneTransform : TransformBase<Transform::Inout>, PruneOptions {
  PruneTransform(PruneOptions const& o = PruneOptions()) : PruneOptions(o) {}
  enum { OptionalInplace = true };
  template <class Arc>
  void inout(IHypergraph<Arc> const& h, IMutableHypergraph<Arc>* o) const {
    pruneUnreachable(h, o, *this);
  }
  template <class Arc>
  void inplace(IMutableHypergraph<Arc>& m) const {
    pruneUnreachable(m, *this);
  }
};


}}

#endif
