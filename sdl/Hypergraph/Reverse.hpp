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

   reverse yield of a CFG or FSM

   (Fsm still changes head/tail, so result is Fsm). Cfg reverses tails (reverses CFG rhs)

   proof sketch: no matter how you bracket a string abcdef into a tree, if you bottom-up reverse the children,
   you get the reverse of the original

   e.g. (ab)c(d(ef)) => [ba]c(d[fe]) -> [ba]c[fed] -> fedcba

   //TODO: fsm reverse in place - requires swapping inarcs, outarcs indices?. for now copies.
   */

#ifndef HYPERGRAPH_CFG_REVERSE_HPP
#define HYPERGRAPH_CFG_REVERSE_HPP
#pragma once

#include <algorithm>
#include <utility>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/DeferAddArcs.hpp>
#include <sdl/Hypergraph/Transform.hpp>

#include <sdl/SharedPtr.hpp>


namespace sdl {
namespace Hypergraph {

template <class Arc>
Arc* newGraphReversedArc(Arc* a) {
  StateIdContainer const& tails = a->tails_;
  return new Arc(a->head_, tails, tails[0], a->weight_);
}

template <class Arc>
void reverseGraph(IHypergraph<Arc> const& hg, IMutableHypergraph<Arc>* result) {
  if (!hg.isGraph()) SDL_THROW_LOG(Hypergraph, InvalidInputException, "reverseGraph called on CFG");
  for (StateId s : hg.getStateIds()) result->addStateId(s, hg.labelPair(s));
  result->setVocabulary(hg.getVocabulary());
  result->setStart(hg.final());
  result->setFinal(hg.start());
  hg.forArcs([&hg, result](Arc* arc) { result->addArc(newGraphReversedArc(arc)); });
}

template <class Arc>
void reverseGraph(IMutableHypergraph<Arc>& hg) {
  if (!hg.isGraph()) SDL_THROW_LOG(Hypergraph, InvalidInputException, "reverseGraph called on CFG");
  hg.forArcs([](Arc* arc) { std::swap(arc->tails_[0], arc->head_); });
  StateId final = hg.final();
  hg.setFinal(hg.start());
  hg.setStart(final);
  hg.rebuildArcAdjacencies();
}

template <class Arc>
struct ArcReverserCfgInplace {
  ArcReverserCfgInplace(IMutableHypergraph<Arc>*) {}
  void operator()(StateIdContainer& tails) const { std::reverse(tails.begin(), tails.end()); }
  void operator()(ArcBase* arc) const { (*this)(arc->tails_); }
};

template <class Arc>
void reverseCfg(IMutableHypergraph<Arc>& hg) {
  if (hg.prunedEmpty()) return;
  if (hg.isGraph()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "reverseCfg called on FSM (reverseGraph instead)");
  }
  forceInArcsOnly(hg);
  ArcReverserCfgInplace<Arc> rev(&hg);
  hg.forArcs(rev);
}

template <class Arc>
void reverse(IHypergraph<Arc> const& hg, IMutableHypergraph<Arc>* result) {
  if (hg.isGraph())
    reverseGraph(hg, result);
  else {
    copyHypergraph(hg, result);
    reverseCfg(*result);
  }
}

template <class Arc>
void reverse(IMutableHypergraph<Arc>& h) {
  if (h.isGraph())
    reverseGraph(h);
  else
    reverseCfg(h);
}


}}

#endif
