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
/** \file

    weighted pruning; only meaningful for Viterbi weight.

    an acyclic sum (LogWeight) pruning using InsideAlgorithm would be fine, but "within delta of best" won't mean what it does for viterbi
*/

#ifndef HYP__HYPERGRAPH_PRUNE_HPP
#define HYP__HYPERGRAPH_PRUNE_HPP
#pragma once

#include <sdl/Hypergraph/Restrict.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Util/FnReference.hpp>
#include <sdl/Hypergraph/Transform.hpp>

namespace sdl {
namespace Hypergraph {

template <class Arc>
struct PruneUnreachable;

struct PruneOptions : TransformOptionsBase
{
  template <class Arc>
  struct TransformFor
  {
    typedef PruneUnreachable<Arc> type;
  };
  bool packStates;
  PruneOptions()
  {
    defaults();
  }
  void defaults()
  {
    packStates=true;
  }
  static char const* caption()
  {
    return "Hypergraph pruning options";
  }
  static char const* name()
  {
    return "prune";
  }

  template <class Conf> void configure(Conf &c)
  {
    c.is("Prune");
    c("pack-states", &packStates)
      ("renumber remaining states to have contiguous ids");
  }

  bool validate() { return true; }
};

template <class Arc>
struct ArcInStateSet
{
  StateSet const& ss;

  explicit ArcInStateSet(StateSet const&s) : ss(s) {}

  bool operator()(Arc *a) const
  {
    if (!Util::test(ss, a->head())) return false;
    StateIdContainer const& tails = a->tails();
    for(StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i)
      if (!Util::test(ss, *i)) return false;
    return true;
  }
};

/**
   Prunes away unreachable states and arcs.

   uses arc filter when you don't want to pack states
 */
template <class Arc>
struct PruneUnreachable : public RestrictPrepare<PruneUnreachable<Arc>, Arc>
{
  PruneOptions opt;
  PruneUnreachable(PruneOptions const& opt=PruneOptions()) : opt(opt) {}
  typedef shared_ptr<StateSet> PUseful;
  PUseful pUseful;
  StateIdMapping *mapping(IHypergraph<Arc> const&h, IMutableHypergraph<Arc> &m)
  {
     // TODO: make Reach return shared_ptr to subset only
    Reach<Arc> r(h, true);
    pUseful = r.useful();
    ArcInStateSet<Arc> s(*pUseful);
    this->keep = s;
    if (opt.packStates) {
      return new StateAddMapping<Arc>(&m);
    } else { // preserve stateids
      return new IdentityIdMapping();
    }
  }
  void completeImpl()
  {
    pUseful.reset();
  }
};

template<class Arc>
void pruneUnreachable(IMutableHypergraph<Arc>* pHg) {
  PruneUnreachable<Arc> pruneFct;
  inplace(*pHg, pruneFct);
}

template<class Arc>
void pruneUnreachable(IMutableHypergraph<Arc>& hg) {
  pruneUnreachable(&hg);
}

template<class Arc>
void pruneUnreachable(IHypergraph<Arc> const& hgInput,
                      IMutableHypergraph<Arc>* pHgResult) {
  PruneUnreachable<Arc> pruneFct;
  inout(hgInput, pHgResult, pruneFct);
}


}}

#endif
