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


#include <sdl/Hypergraph/BestPath.hpp>
#include <sdl/Hypergraph/Prune.hpp>
#include <sdl/Util/PointerWithFlag.hpp>
#include <sdl/Hypergraph/PruneEpsilon.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Util/Delete.hpp>
#include <sdl/Hypergraph/OutsideCosts.hpp>
#include <sdl/Util/TopK.hpp>

namespace sdl {
namespace Hypergraph {

struct PruneToNbestOptions : PruneEpsilonOptions {
  bool pruning() const { return pruneToNbest; }
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
        .defaulted()(
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

struct PruneToBestOptions : PruneToNbestOptions, PruneOptions, BestPathOptions {
  typedef OptionalInplaceTransform<PruneToBestOptions> PruneToBestTransform;
  template <class Arc>
  struct TransformFor {
    typedef PruneToBestTransform type;
  };
  static char const* type() { return "PruneToBest"; }
  PruneToBestOptions(unsigned defaultNbest = 1) : PruneToNbestOptions(defaultNbest) { init(); }
  template <class Opt>
  PruneToBestOptions(unsigned defaultNbest, Opt const& opt)
      : PruneToNbestOptions(defaultNbest, opt) {
    init();
  }
  template <class Opt>
  PruneToBestOptions(Opt const& opt, bool, bool)
      : PruneToNbestOptions(opt) {
    init();
  }

  SdlFloat beam, beamEpsilon;
  StateId beamPlusStates;
  bool single;
  bool beaming() const {
    assert(!single || is_null(beam));
    return !is_null(beam);
  }
  bool pruning() const { return !is_null(beam) || single; }
  template <class Config>
  void configure(Config& config) {
    PruneToNbestOptions::configure(config);
    config("beam-epsilon", &beamEpsilon)
        .defaulted()(
            "extra margin added to beam-width to allow '0' to be safely used w/ floating point rounding "
            "differences");
    config("beam-width", &beam)
        .defaulted()(
            "cost difference from best path to preserve (used instead of prune-to-nbest if specified");
    config("beam-plus-states", &beamPlusStates)
        .defaulted()(
            "after beam-width, keep an additional number of states (meaning: make the beam just wide enough "
            "to include those extra states");
    config("single", &single)
        .defaulted()
        .verbose()(
            "prune to exactly a single derivation (if false, no pruning except if beam-width is set >= 0)");
    PruneOptions::configure(config);  // could make hierarchical like best
    config("best", (BestPathOptions*)this);
    config("restrict hypergraph to states and arcs on or near the best scoring path");
    config.is("Prune to (within beam-width of) best derivation");
  }
  void validate() {
    PruneOptions::validate();
    BestPathOptions::validate();
    PruneToNbestOptions::validate();
    if (pruneToNbest == 1) single = true;
    if (!is_null(beam)) {
      if (beam < 0) {
        SDL_WARN(PruneToBest, "beam-width < 0; disabling beam pruning");
        beam = NAN;
      } else if (single) {
        SDL_DEBUG(PruneToBest, "not pruning to single best derivation, because beam-width was set to " << beam);
        single = false;
      }
    }
    if (!single) pruneToNbest = 0;
    SDL_DEBUG(PruneToBest, "validated: " << *this);
  }
  friend void validate(PruneToBestOptions& po) { po.validate(); }
  friend inline std::ostream& operator<<(std::ostream& out, PruneToBestOptions const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    out << "single=" << single << " beam-width=" << beam << " beam-plus-states=" << beamPlusStates
        << " pruning=" << pruning();
  }

  template <class Arc>
  void inout(IHypergraph<Arc> const& h, IMutableHypergraph<Arc>* o) const;
  template <class Arc>
  void inplace(IMutableHypergraph<Arc>& m) const;

 private:
  void init() {
    single = true;
    beam = NAN;
    beamEpsilon = 1e-5;
    beamPlusStates = 0;
  }
};

typedef PruneToBestOptions::PruneToBestTransform PruneToBestTransform;

/// lightweight copy, expensive constructor (computes best path)
template <class A>
struct ArcInBest {
  typedef BestPath::Compute<A> ComputeBest;
  typedef typename ComputeBest::Pi Predecessor;  // pmap: best arc for head. returns ArcHandle for StateId.
  typedef typename ComputeBest::Mu Inside;
  MutableHypergraph<A> const* phg;
  bool beaming;
  bool inarcs;
  bool single;
  bool empty;
  Predecessor pi;
  Inside inside;
  SdlFloat worstAllowedCost;
  typedef Util::AutoDeleteArray<SdlFloat> Outside;
  shared_ptr<Outside> outside;
  SdlFloat* outside0;
  StateId start, N;
  ArcInBest(IHypergraph<A> const& hg, PruneToBestOptions const& opt)
      : phg(&dynamic_cast<MutableHypergraph<A> const&>(hg))
      , beaming(opt.beaming())
      , single(opt.single)
      , worstAllowedCost(HUGE_VAL)
      , start(hg.start()) {
    SDL_DEBUG(PruneToBest, "ArcInBest from " << opt);
    if (!beaming && !single) return;
    if (single && beaming) beaming = false;
    N = hg.sizeForHeads();
    empty = !N;
    if (empty) return;
    ComputeBest best(opt, hg);
    if (beaming) empty = !best.best(false, single);
    if (empty) return;
    inarcs = hg.storesInArcs();
    StateId final = hg.final();
    if (single) {
      pi = best.predecessors();
      justForGoal(final, N);
    } else {
      assert(beaming);
      inside = best.mu;
      // TODO: separate impl class for efficiency? we don't care about pi ...
      pi = Predecessor();
      worstAllowedCost = inside[final] + opt.beam;
      SDL_TRACE(PruneToBest, "worst allowed (before beam-plus-states, epsilon): "
                                 << worstAllowedCost << " final=" << final << " start=" << start);
      outside.reset(new Outside(N, (float)HUGE_VAL));
      outsideCosts(hg, (outside0 = *outside), inside, N,
                   opt.beamPlusStates ? HUGE_VAL : opt.beamEpsilon + worstAllowedCost);
      if (opt.beamPlusStates) {
        typedef std::pair<SdlFloat, StateId> S;

        std::vector<S> topk;
        for (unsigned i = 0; i < N; ++i) {
          SdlFloat c = outside0[i];
          if (hg.isAxiom(i)) inside[i] = 0;
          c += inside[i];
          if (!is_null(c) && c > worstAllowedCost) topk.emplace_back(c, i);
        }
        Util::topk(topk, opt.beamPlusStates);
        if (!topk.empty()) worstAllowedCost = topk.back().first;
      }
      worstAllowedCost += opt.beamEpsilon;
      SDL_DEBUG(PruneToBest, "worst allowed: " << worstAllowedCost);
      SDL_TRACE(PruneToBest, "inside:  " << Util::arrayPrintable(&inside[0], N, true));
      SDL_TRACE(PruneToBest, "outside: " << Util::arrayPrintable(outside0, N, true));
    }
  }
  void justForGoal(StateId goal, StateId nStates) {
    markFromGoal(goal);
    for (StateId s = 0; s < nStates; ++s) keepMarked(s, get(pi, s));
  }
  SdlFloat edgeCost(A* a) const {
    assert(beaming);
    assert(inside);
    StateId h = a->head_;
    assert(h < N);
    SdlFloat c = outside0[h] + a->weight_.value_;
    for (StateIdContainer::const_iterator i = a->tails_.begin(), e = a->tails_.end(); i != e; ++i) {
      StateId t = *i;
      if (t < N) c += inside[t];
    }
    return c;
  }

  bool operator()(A* a) const {
    assert(!empty);
    // if (empty) return false;
    StateId h = a->head_;
    if (single) return get(pi, h) == (ArcHandle)a;
    if (beaming) return edgeCost(a) <= worstAllowedCost;
    return true;
  }

 private:
  bool marked(StateId s) const { return Util::lsb(get(pi, s)); }
  void markFromGoal(StateId s) {
    ArcHandle ah = get(pi, s);
    if (ah) {
      if (Util::lsb(ah)) return;  // already marked subtree
      put(pi, s, Util::withLsb(ah));
      A* a = (A*)ah;
      for (StateId tail : a->tails()) {
        markFromGoal(tail);
      }
    } else if (inarcs && phg->hasLexicalLabel(s)) {
      for (ArcId aid : phg->inArcIds(s)) {
        A* arc = phg->inArc(s, aid);
        assert(!arc->tails().empty());
        if (arc->getTail(0) == start) put(pi, s, Util::withLsb((ArcHandle)arc));
      }
    }
  }
  void keepMarked(StateId s, ArcHandle a) { put(pi, s, Util::lsb(a) ? Util::withoutLsb(a) : (ArcHandle)0); }
};

template <class A>
struct PruneToBest : RestrictPrepare<PruneToBest<A>, A> {
  PruneToBestOptions const& opt;
  typedef RestrictPrepare<PruneToBest<A>, A> Base;
  enum { Inplace = true, OptionalInplace = false };
  using Base::inout;
  void inplace(IMutableHypergraph<A>& hg) {
    SDL_DEBUG(PruneToBest, "pruning " << hg);
    if (opt.pruning()) Base::inplace(hg);
    PruneEpsilon pruneEpsilon(opt);
    pruneEpsilon.inplace(hg);
  }

  template <class Opt>
  PruneToBest(Opt const& opt)
      : opt(opt) {}
  PruneToBest(PruneToBest const& o) = delete;

  bool needs(IHypergraph<A>& hg) const { return needsRestrict(hg); }

  bool needsRestrict(IHypergraph<A>& hg) const {
    if (opt.skipAlreadySingle && hg.isMutable() && hasTrivialBestPath(hg, hg.final(), hg.start())) {
      PruneEpsilon pruneEpsilon(opt);
      pruneEpsilon.maybePruneEpsilon(static_cast<IMutableHypergraph<A>&>(hg));
      return false;
    } else
      return true;
  }

  StateIdMapping* mapping(IHypergraph<A> const& hg, IMutableHypergraph<A>& outHg) const {
    try {
      SDL_DEBUG(PruneToBest, "creating mapping from " << opt);
      ArcInBest<A> b(hg, opt);
      if (b.empty) return 0;
      this->keep = b;
    } catch (EmptySetException&) {
      return 0;
    }
    if (opt.packStates) {
      return new StateAddMapping<A>(&outHg);
    } else {  // preserve stateids - means we need to define an edge filter
      return new IdentityIdMapping();
    }
  }
};

template <class Arc>
void PruneToBestOptions::inout(IHypergraph<Arc> const& h, IMutableHypergraph<Arc>* o) const {
  PruneToBest<Arc> p(*this);
  p.inout(h, o);
}

template <class Arc>
void PruneToBestOptions::inplace(IMutableHypergraph<Arc>& m) const {
  PruneToBest<Arc> p(*this);
  p.inplace(m);
}

/// if leaveSimplePathAlone, don't bother removing arcs as long as there's just 1 inarc from final
template <class Arc>
void justBest(IMutableHypergraph<Arc>& hg, bool leaveSimplePathAlone = true) {
  PruneToBestOptions opt;
  opt.skipAlreadySingle = leaveSimplePathAlone;
  inplace(hg, PruneToBestTransform(opt));
}

template <class Arc>
void justBest(IHypergraph<Arc> const& hg, IMutableHypergraph<Arc>& out, bool leaveSimplePathAlone = true) {
  PruneToBestOptions opt;
  opt.skipAlreadySingle = leaveSimplePathAlone;
  opt.inout(hg, &out);
}

/// Options is either PruneToBestOptions or PruneEpsilonOptions
template <class Arc, class Options>
void justBest(unsigned nbest, IMutableHypergraph<Arc>& hg, Options const& pruneNonBestOrEpsilonOptions,
              bool leaveSimplePathAlone = true) {
  if (nbest) {
    PruneToBestOptions opt(nbest, pruneNonBestOrEpsilonOptions);
    PruneToBest<Arc> prune(opt);
    prune.skipAlreadySingle = leaveSimplePathAlone;
    inplace(hg, PruneToBestTransform(prune));
  } else if (!leaveSimplePathAlone)
    pruneNonBestOrEpsilonOptions.maybePruneSimplePath(hg);
}

template <class Arc, class Options>
void justBest(IMutableHypergraph<Arc>& hg, Options const& opt, bool leaveSimplePathAlone = true) {
  if (opt.pruning()) {
    PruneToBestOptions o(opt, false, false);
    o.skipAlreadySingle = leaveSimplePathAlone;
    inplace(hg, PruneToBestTransform(o));
  } else if (!leaveSimplePathAlone)
    opt.maybePruneSimplePath(hg);
}


}}

#endif
