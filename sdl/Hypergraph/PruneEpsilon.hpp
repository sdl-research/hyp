/** \file

    remove any single-epsilon-outarc states from simple path
*/

#ifndef PRUNEEPSILON_JG_2015_03_17_HPP
#define PRUNEEPSILON_JG_2015_03_17_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>

namespace sdl { namespace Hypergraph {

template <class Arc>
void pruneSimplePathGraphEpsilon(IMutableHypergraph<Arc> &hg, bool keepEpsWeight = true) {
  assert(hg.isGraph());
  SDL_DEBUG(Hypergraph.PruneEpsilon, "before pruneSimplePathGraphEpsilon:\n"<<hg);
  hg.forceFirstTailOutArcsOnly();
  StateId final = hg.final();
  if (final == kNoState) {
    hg.setEmpty();
    return;
  }
  unsigned N = hg.sizeForHeads();
  StateId const sorig = hg.start();
  StateId s = sorig;
  assert(s != kNoState);
  typedef typename IHypergraph<Arc>::ArcsContainer ArcsContainer;
  typedef typename Arc::Weight Weight;
  Weight initwt;
  Weight *prevwt = 0;
  Arc *lastnoneps = 0;
  while (s != final) {
    ArcsContainer *outarcs = hg.maybeOutArcs(s);
    if (!outarcs) return; // no paths so don't care
    if (outarcs->size() != 1) return; // not a path
    Arc *a = outarcs->front();
    s = a->head();
    if (isEpsilonLikeGraphArcAnyWeight(hg, *a)) {
      if (lastnoneps)
        lastnoneps->head() = s;
      else
        hg.setStart(s);
      Weight const& w = a->weight();
      if (keepEpsWeight && !isOne(w)) {
        if (lastnoneps) {
          initwt = w;
          prevwt = &initwt;
        } else
          timesBy(w, *prevwt);
      }
      delete a;
      outarcs->pop_back();
    } else {
      prevwt = &a->weight();
      lastnoneps = a;
      if (!lastnoneps) {
        timesBy(initwt, *prevwt);
      }
    }
  }
  assert(s == final);
  if (keepEpsWeight && !lastnoneps && prevwt) {
    assert(hg.start() == final);
    hg.setStart(sorig);
    hg.addArcGraphEpsilon(sorig, final, initwt);
  } else if (lastnoneps)
    hg.setFinal(lastnoneps->head());
  else
    hg.setFinal(hg.start());
  SDL_DEBUG(Hypergraph.PruneEpsilon, "before pruneSimplePathGraphEpsilon:\n"<<hg);
}


}}

#endif
