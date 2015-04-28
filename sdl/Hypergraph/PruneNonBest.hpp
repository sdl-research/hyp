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

    remove arcs not participating in a well-scoring derivation (for now, only
    the 1best is well-scoring).

    see also BestPath.hpp NbestHypergraphOptions which will save (without
    sharing) the nbest paths. this header intends to perform 'relatively
    useless' pruning keeping only the well-scoring parts

    if input is not a graph then you have to store in-arcs
*/

#ifndef PRUNENONBEST_LW2012517_HPP
#define PRUNENONBEST_LW2012517_HPP
#pragma once

#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/BestPath.hpp>
#include <sdl/Hypergraph/Prune.hpp>
#include <sdl/Util/PointerWithFlag.hpp>
#include <sdl/Hypergraph/PruneEpsilon.hpp>

namespace sdl {
namespace Hypergraph {

struct PruneToNbestOptions : PruneEpsilonOptions {
  void validate() {
    if (pruneToNbest > 1) {
      SDL_ERROR(Hypergraph.pruneToNbest, "prune-to-nbest " << pruneToNbest
                                                           << " > 1 is not supported yet; using 1");
      pruneToNbest = 1;
    }
  }
  friend inline void validate(PruneToNbestOptions& x) { x.validate(); }
  explicit PruneToNbestOptions(unsigned defaultNbest = 0)
      : pruneToNbest(defaultNbest), skipAlreadySingle(true) {}
  explicit PruneToNbestOptions(unsigned defaultNbest, PruneEpsilonOptions const& opt)
      : pruneToNbest(defaultNbest), skipAlreadySingle(true), PruneEpsilonOptions(opt) {}
  template <class Config>
  void configure(Config& config) {
    PruneEpsilonOptions::configure(config);
    config("prune-to-nbest", &pruneToNbest)
        .self_init()(
            "if >0, prune output so as to preserve just the [prune-to-nbest]-best paths (or you can use the "
            "separate PruneToBest module instead). only 1-best is supported so far.");
    config("skip-already-single", &skipAlreadySingle)
        .init(true)("don't modify hg if it already has only a single reachable derivation");
  }
  unsigned pruneToNbest;
  bool skipAlreadySingle;
  friend inline std::ostream& operator<<(std::ostream& out, PruneToNbestOptions const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { out << "prune-to-nbest=" << pruneToNbest; }

  template <class Arc>
  void maybePruneEpsilon(IMutableHypergraph<Arc>& hg) const {
    if (pruneEpsilon) pruneSimplePathGraphEpsilon(hg, keepEpsilonWeights, pathPruneAllStates);
  }

  template <class Arc>
  bool maybePruneSimplePath(IMutableHypergraph<Arc>& hg, bool checkEvenIfNoPrune = false) const {
    if ((checkEvenIfNoPrune || pruneEpsilon) && hasTrivialBestPath(hg, hg.final(), hg.start())) {
      maybePruneEpsilon(hg);
      return true;
    } else
      return false;
  }

  template <class Arc>
  bool skipSimplePath(IMutableHypergraph<Arc>& hg) const {
    return maybePruneSimplePath(hg, true);
  }
};

template <class Arc>
struct PruneNonBest;

struct PruneNonBestOptions : PruneToNbestOptions, PruneOptions, BestPathOptions {
 private:
  void init() { single = true; }

 public:
  PruneNonBestOptions(unsigned defaultNbest = 1) : PruneToNbestOptions(defaultNbest) { init(); }
  template <class Opt>
  PruneNonBestOptions(unsigned defaultNbest, Opt const& opt)
      : PruneToNbestOptions(defaultNbest, opt) {
    init();
  }
  template <class Arc>
  struct TransformFor {
    typedef PruneNonBest<Arc> type;
  };
  static char const* name() { return "prune-non-best"; }
  template <class Config>
  void configure(Config& config) {
    PruneToNbestOptions::configure(config);
    config("single", &prune).init(true).verbose()("perform pruning (if false, sets prune-to-nbest to 0)");
    PruneOptions::configure(config);  // could make hierarchical like best
    config("best", (BestPathOptions*)this);
    config("restrict hypergraph to states and arcs on or near the best scoring path");
    config.is("Prune non-best");
  }
  void validate() {
    PruneOptions::validate();
    BestPathOptions::validate();
    if (!single) pruneToNbest = 0;
  }
  friend void validate(PruneOptions& po) { po.validate(); }
  unsigned pruneToNbest;
  bool prune;
};

/// lightweight copy, expensive constructor (computes best path)
template <class A>
struct ArcInBest {
  typedef BestPath::Compute<A> ComputeBest;
  typedef typename ComputeBest::Pi Predecessor;  // pmap: best arc for head. returns ArcHandle for StateId.
  MutableHypergraph<A> const* phg;
  Predecessor pi;
  StateId start;
  bool inarcs;
  ArcInBest(Predecessor const& pi) : pi(pi) {}
  ArcInBest(ArcInBest const& o) : pi(o.pi) {}
  ArcInBest(IHypergraph<A> const& hg, PruneNonBestOptions const& opt)
      : phg(&dynamic_cast<MutableHypergraph<A> const&>(hg))
      , pi(ComputeBest(BestPathOptions(), hg).predecessors())
      , start(hg.start()) {
    init(opt, hg);
  }
  void init(PruneNonBestOptions const& opt, IHypergraph<A> const& hg)  // pi must be set already
  {
    inarcs = hg.storesInArcs();
    if (opt.single) justForGoal(hg.final(), hg.size());
  }
  void justForGoal(StateId goal, StateId nStates) {
    markFromGoal(goal);
    for (StateId s = 0; s < nStates; ++s) keepMarked(s, get(pi, s));
  }
  bool operator()(A* a) {
    StateId h = a->head();
    return get(pi, h) == (ArcHandle)a;
  }

 private:
  bool marked(StateId s) const { return Util::lsb(get(pi, s)); }
  void markFromGoal(StateId s) {
    ArcHandle ah = get(pi, s);
    if (ah) {
      if (Util::lsb(ah)) return;  // already marked subtree
      put(pi, s, Util::withLsb(ah));
      A* a = (A*)ah;
      forall (StateId tail, a->tails()) { markFromGoal(tail); }
    } else if (inarcs && phg->hasLexicalLabel(s)) {
      forall (ArcId aid, phg->inArcIds(s)) {
        A* arc = phg->inArc(s, aid);
        assert(!arc->tails().empty());
        if (arc->getTail(0) == start) put(pi, s, Util::withLsb((ArcHandle)arc));
      }
    }
  }
  void keepMarked(StateId s, ArcHandle a) { put(pi, s, Util::lsb(a) ? Util::withoutLsb(a) : (ArcHandle)0); }
};

template <class A>
struct PruneNonBest : RestrictPrepare<PruneNonBest<A>, A> {
  typedef RestrictPrepare<PruneNonBest<A>, A> Base;
  void inplace(IMutableHypergraph<A>& hg) {
    Base::inplace(hg);
    PruneEpsilon pruneEpsilon(opt);
    pruneEpsilon.inplace(hg);
  }
  PruneNonBestOptions opt;
  PruneNonBest(PruneNonBestOptions const& opt = PruneNonBestOptions()) : opt(opt) {}
  template <class Opt>
  PruneNonBest(unsigned defaultNbest, Opt const& opt)
      : opt(defaultNbest, opt) {}
  template <class Opt>
  explicit PruneNonBest(Opt const& opt)
      : opt(1, opt) {}
  typedef shared_ptr<StateSet> PUseful;
  PUseful pUseful;

  bool needsRestrict(IHypergraph<A>& hg) {
    if (opt.skipAlreadySingle && hg.isMutable() && hasTrivialBestPath(hg, hg.final(), hg.start())) {
      PruneEpsilon pruneEpsilon(opt);
      pruneEpsilon.maybePruneEpsilon(static_cast<IMutableHypergraph<A>&>(hg));
      return false;
    } else
      return true;
  }

  StateIdMapping* mapping(IHypergraph<A> const& hg, IMutableHypergraph<A>& outHg) {
    try {
      this->keep = ArcInBest<A>(hg, opt);
    } catch (EmptySetException&) {
      this->keep = A::filterFalse();  // no best path
    }
    if (opt.packStates) {
      return new StateAddMapping<A>(&outHg);
    } else {  // preserve stateids - means we need to define an edge filter
      return new IdentityIdMapping();
    }
  }
  void completeImpl() {}
};

/// if leaveSimplePathAlone, don't bother removing arcs as long as there's just 1 inarc from final
template <class Arc>
void justBest(IMutableHypergraph<Arc>& hg, bool leaveSimplePathAlone = true) {
  PruneNonBest<Arc> prune;
  prune.opt.skipAlreadySingle = leaveSimplePathAlone;
  prune.inplace(hg);
}

/// Options is either PruneNonBestOptions or PruneEpsilonOptions
template <class Arc, class Options>
void justBest(unsigned nbest, IMutableHypergraph<Arc>& hg, Options const& pruneNonBestOrEpsilonOptions,
              bool leaveSimplePathAlone = true) {
  if (nbest) {
    PruneNonBest<Arc> prune(nbest, pruneNonBestOrEpsilonOptions);
    prune.opt.skipAlreadySingle = leaveSimplePathAlone;
    prune.inplace(hg);
  } else if (leaveSimplePathAlone)
    pruneNonBestOrEpsilonOptions.maybePruneSimplePath(hg);
}

template <class Arc>
void justBest(IMutableHypergraph<Arc>& hg, PruneToNbestOptions const& pruneNonBestOrEpsilonOptions,
              bool leaveSimplePathAlone = true) {
  if (pruneNonBestOrEpsilonOptions.pruneToNbest) {
    PruneNonBest<Arc> prune(pruneNonBestOrEpsilonOptions);
    prune.opt.skipAlreadySingle = leaveSimplePathAlone;
    prune.inplace(hg);
  } else if (leaveSimplePathAlone)
    pruneNonBestOrEpsilonOptions.maybePruneSimplePath(hg);
}


}}

#endif
