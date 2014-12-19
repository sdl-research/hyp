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
#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/Transform.hpp>

#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Forall.hpp>

namespace sdl {
namespace Hypergraph {

// TODO: This would be a good test case for a lazy (on-demand)
// operation.

namespace fs {

template <class Arc>
struct ArcReverser {
  IMutableHypergraph<Arc>* hg_;
  ArcReverser(IMutableHypergraph<Arc>* hg) : hg_(hg) {}
  void operator()(Arc const& arc) const {
    hg_->addArc(new Arc(Head(arc.getTail(0)), Tails(arc.head(), arc.getTail(1)), arc.weight()));
  }
  void operator()(Arc const* arc) const { (*this)(*arc); }
};

template <class Arc>
struct ArcReverserFsInPlace {
  IMutableHypergraph<Arc>* hg_;
  ArcReverserFsInPlace(IMutableHypergraph<Arc>* hg) : hg_(hg) {}
  void operator()(Arc& arc) const {
    typename Arc::StateIdContainer& tails = arc.tails();
    StateId h = arc.head();  // swap head, tail
    arc.setHead(tails[0]);
    tails[0] = h;
  }
  void operator()(Arc* arc) const { (*this)(*arc); }
};

template <class Arc>
void reverseFsm(const IHypergraph<Arc>& inhg, IMutableHypergraph<Arc>* result) {
  if (!inhg.isFsm()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "reverseFsm called on CFG");
  }
  forall (StateId sid, inhg.getStateIds()) {
    result->addStateId(sid, inhg.inputLabel(sid), inhg.outputLabel(sid));
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
struct ArcReverserCfgInPlace {
  IMutableHypergraph<Arc>* hg_;
  ArcReverserCfgInPlace(IMutableHypergraph<Arc>* hg) : hg_(hg) {}
  void operator()(Arc& arc) const {
    typename Arc::StateIdContainer& tails = arc.tails();
    std::reverse(tails.begin(), tails.end());
  }
  void operator()(Arc* arc) const { (*this)(*arc); }
};

template <class Arc>
void reverseCfg(IMutableHypergraph<Arc>& h) {
  if (h.prunedEmpty()) return;
  if (h.isFsm()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "reverseCfg called on FSM (reverseFsm instead)");
  }
  ArcReverserCfgInPlace<Arc> rev(&h);
  h.forArcs(rev);
}

template <class Arc>
void reverse(const IHypergraph<Arc>& inhg, IMutableHypergraph<Arc>* result) {
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

struct Reverse : TransformBase<Transform::Inplace> {
  template <class Config>
  void configure(Config& c) {
    c.is("Reverse");
    c("Reverse each path in a hypergraph");
  }

  static char const* name() { return "Reverse"; }

  template <class A>
  void inplace(IMutableHypergraph<A>& h) const {
    reverse(h);
  }
};


}}

#endif
