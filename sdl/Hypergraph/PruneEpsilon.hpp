/** \file

    remove any single-epsilon-outarc states from simple path (# outarcs <= 1 from start->final)
*/

#ifndef PRUNEEPSILON_JG_2015_03_17_HPP
#define PRUNEEPSILON_JG_2015_03_17_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>

namespace sdl {
namespace Hypergraph {

template <class Arc>
void pruneSimplePathGraphEpsilon(IMutableHypergraph<Arc>& hg, bool keepEpsilonWeights = true,
                                 bool pathPruneAllStates = false) {
  hg.forceFirstTailOutArcsOnly();
  SDL_DEBUG(Hypergraph.PruneEpsilon, "before pruneSimplePathGraphEpsilon properties: "
                                     << PrintProperties(hg.properties()) << ":\n" << hg);
  assert(hg.isGraph());
  StateId final = hg.final();
  if (final == kNoState) {
    hg.setEmpty();
    return;
  }
  unsigned const N = pathPruneAllStates ? hg.sizeForHeads() : 0;
  StateSet usefulState(N);
  StateId const sorig = hg.start();
  assert(sorig != kNoState);
  typedef typename IHypergraph<Arc>::ArcsContainer ArcsContainer;
  typedef typename Arc::Weight Weight;
  Weight initwt;
  Weight* prevwt = 0;
  Arc* lastnoneps = 0;
  bool useinitwt = false;
  for (StateId s = sorig, snext; s != final; s = snext) {
    ArcsContainer* outarcs = hg.maybeOutArcs(s);
    if (!outarcs) {
      hg.setEmpty();
      return;  // no paths
    }
    if (outarcs->size() != 1) {
      SDL_THROW_LOG(Hypergraph.PruneEpsilon, InvalidInputException, "input hg was not a simple path: state "
                                                                    << s << " had " << outarcs->size()
                                                                    << " out arcs instead of 0 or 1\n");
      return;  // not a path
    }
    Arc* a = outarcs->front();
    snext = a->head();
    if (snext == s) {
      SDL_WARN(Hypergraph.PruneEpsilon, "No path (because of arc " << *a << " self-loop leaving state " << s);
      hg.setEmpty();
      return;
    }
    if (isEpsilonLikeGraphArcAnyWeight(hg, *a)) {
      if (lastnoneps)
        lastnoneps->head() = snext;
      else
        hg.setStart(snext);
      Weight const& w = a->weight();
      if (keepEpsilonWeights && !isOne(w)) {
        if (prevwt) {
          timesBy(w, *prevwt);
        } else {
          useinitwt = true;
          initwt = w;
          prevwt = &initwt;
        }
      }
      delete a;
      outarcs->pop_back();
    } else {
      if (pathPruneAllStates) usefulState.set(s);
      prevwt = &a->weight();
      if (useinitwt) {
        timesBy(initwt, *prevwt);
        useinitwt = false;
      }
      lastnoneps = a;
    }
  }
  if (lastnoneps) {
    hg.setFinal(lastnoneps->head());
  } else if (useinitwt) {
    assert(hg.start() == final);
    hg.setStart(sorig);
    if (pathPruneAllStates) usefulState.set(sorig);
    hg.addArcGraphEpsilon(sorig, final, initwt);
  } else {
    hg.setFinal(hg.start());
  }

  if (pathPruneAllStates)
    for (StateId s = 0; s < N; ++s)
      if (!usefulState.test(s)) {
        SDL_DEBUG(Hypergraph.PruneEpsilon, "removing useless state " << s);
        ArcsContainer* outarcs = hg.maybeOutArcs(s);
        if (outarcs && outarcs->size()) {
          SDL_DEBUG(Hypergraph.PruneEpsilon, "removing useless state with outarcs: " << s);
          for (typename ArcsContainer::const_iterator i = outarcs->begin(), e = outarcs->end(); i != e; ++i)
            delete *i;
          outarcs->clear();
        }
      }
  SDL_DEBUG(Hypergraph.PruneEpsilon, "after pruneSimplePathGraphEpsilon:\n" << hg);
}

struct PruneEpsilonOptions : TransformBase<Transform::Inplace> {
  bool pruneEpsilon, keepEpsilonWeights, pathPruneAllStates;
  PruneEpsilonOptions(bool pruneEpsilon = false, bool keepEpsilonWeights = true, bool pathPruneAllStates = true)
      : pruneEpsilon(pruneEpsilon)
      , keepEpsilonWeights(keepEpsilonWeights)
      , pathPruneAllStates(pathPruneAllStates) {}
  template <class Config>
  void configure(Config& config) {
    config("path-prune-epsilon", &pruneEpsilon)(
        "remove epsilon-like arcs under the assumption that input hg is a simple path, i.e., has no "
        "branches from start-> (#outarcs is 0 or 1)");
    config("path-keep-epsilon-weights", &keepEpsilonWeights)(
        "when prune-epsilon, preserve any weights on the removed epsilon weights (shifting them left)");
    config("path-prune-all-states", &pathPruneAllStates)("remove arcs belonging to states not on the path");
  }
  bool enabled() const { return pruneEpsilon; }
  template <class Arc>
  bool needs(IMutableHypergraph<Arc>& h) const {
    return pruneEpsilon;
  }
  Properties inAddProps() const { return kStoreFirstTailOutArcs; }
  template <class Arc>
  void maybePruneEpsilon(IMutableHypergraph<Arc>& hg) const {
    if (pruneEpsilon) pruneSimplePathGraphEpsilon(hg, keepEpsilonWeights, pathPruneAllStates);
  }
  template <class Arc>
  struct TransformFor {
    typedef PruneEpsilonOptions type;
  };
  template <class Arc>
  void inplace(IMutableHypergraph<Arc>& hg) const {
    maybePruneEpsilon(hg);
  }
};

template <class Arc>
struct TransformFor<PruneEpsilonOptions, Arc> {
  typedef PruneEpsilonOptions type;
};


}}

#endif
