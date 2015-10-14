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

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>

#include <sdl/Hypergraph/Transform.hpp>

#include <sdl/SharedPtr.hpp>


namespace sdl {
namespace Hypergraph {

// TODO: This would be a good test case for a lazy (on-demand)
// operation.

namespace fs {

template <class Arc>
struct ArcReverser {
  HypergraphBase* hg_;
  ArcReverser(HypergraphBase* hg) : hg_(hg) {}
  void operator()(Arc const& arc) const {
    StateIdContainer const& tails = arc.tails_;
    assert(tails.size() == 2);
    hg_->addArc(new Arc(HeadAndTail(), tails[0], arc.head_, tails[1], arc.weight_));
  }
  void operator()(Arc const* arc) const { (*this)(*(Arc*)arc); }
};

template <class Arc>
void reverseFsm(IHypergraph<Arc> const& inhg, IMutableHypergraph<Arc>* result) {
  if (!inhg.isFsm())
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "reverseFsm called on CFG");
  for (StateId s : inhg.getStateIds()) {
    result->addStateId(s, inhg.labelPair(s));
  }
  result->setVocabulary(inhg.getVocabulary());
  result->setStart(inhg.final());
  result->setFinal(inhg.start());
  ArcReverser<Arc> fct(result);
  inhg.forArcs(fct);
}

template <class Arc>
void reverseFsm(IMutableHypergraph<Arc>& h) {
  IMutableHypergraph<Arc>* i = h.clone();
  h.clear();
  reverseFsm(*i, &h);
  delete i;
}
}

template <class Arc>
struct ArcReverserCfgInplace {
  ArcReverserCfgInplace(IMutableHypergraph<Arc>*) {}
  void operator()(StateIdContainer& tails) const {
    std::reverse(tails.begin(), tails.end());
  }
  void operator()(ArcBase* arc) const { (*this)(arc->tails_); }
};

template <class Arc>
void reverseCfg(IMutableHypergraph<Arc>& hg) {
  if (hg.prunedEmpty()) return;
  if (hg.isFsm()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "reverseCfg called on FSM (reverseFsm instead)");
  }
  forceInArcsOnly(hg);
  ArcReverserCfgInplace<Arc> rev(&hg);
  hg.forArcs(rev);
}

template <class Arc>
void reverse(IHypergraph<Arc> const& inhg, IMutableHypergraph<Arc>* result) {
  if (inhg.isFsm())
    fs::reverseFsm(inhg, result);
  else {
    copyHypergraph(inhg, result);
    reverseCfg(*result);
  }
}

template <class Arc>
void reverse(IMutableHypergraph<Arc>& h) {
  if (h.isFsm())
    fs::reverseFsm(h);
  else
    reverseCfg(h);
}


}}

#endif
