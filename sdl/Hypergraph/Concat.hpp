// Copyright 2014 SDL plc
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
   concatenate yields of two forests
*/

#ifndef HYP__CONCATLW20111227_HPP
#define HYP__CONCATLW20111227_HPP
#pragma once

#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Hypergraph/Adjacency.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Hypergraph/Union.hpp> // for merging two hgs' states
#include <sdl/Hypergraph/HypergraphCopy.hpp>
#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Hypergraph {

struct ConcatOptions {
  bool mergeFsmFinalStart;
  bool cfgFinalLabel;
  bool checkEmptyRight;
  std::string defaultFinalLabel;
  ConcatOptions()
      : mergeFsmFinalStart(true)
      , cfgFinalLabel(true)
      , checkEmptyRight(false)
      , defaultFinalLabel("ConcatFinal")
  // TODO: kCanonicalLex - requires modifying StateIdTranslation
  {}

  static char const* caption() { return "Concatenate (CFG or FSM)"; }

  template <class Conf>
  void configure(Conf const& c) {
    c.is("Concatenate Hypergraphs");
    c("merge-fsm-states",
      &mergeFsmFinalStart)('m')("merge FSM start2/final1 states if possible (label from start2)");
    c("cfg-final-label", &mergeFsmFinalStart)('c')(
        "build label for new cfg final state as input from cfg1-input-label, output from cfg2-input-label");
    c("default-final-label", &defaultFinalLabel)("use this label for new cfg final state if -c 0");
    c("check-empty-rhs", &checkEmptyRight)(
        "perform linear-time empty check on rhs to set resulting crossproduct explicitly to empty");
  }
};

// concat of yields of crossprod A x B - note: if either empty, so is result
template <class A>
struct Concat : TransformBase<Transform::Inplace, 0> {
  static char const* name() { return "Concat"; }
  typedef A Arc;
  typedef IHypergraph<A> HG;
  typedef shared_ptr<HG const> PHG;
  PHG rh;
  ConcatOptions opt;
  explicit Concat(PHG const& rh, ConcatOptions const& opt = ConcatOptions()) : rh(rh), opt(opt) {}
  bool mustCopy(HG const& h) const {
    // can't update h with r if h is same as r
    copyIfSame(rh, h);
    return false;
  }
  // both hg must have same vocab
  void inplace(IMutableHypergraph<A>& l) const {
    IHypergraph<A> const& r = *rh;
    assert(l.getVocabulary() == r.getVocabulary());
    if (pruneEmpty(l)) {
      SDL_DEBUG(Hypergraph.Concat, "concat(l, r): l empty -> result empty");
      return;
    }
    if (r.prunedEmpty() || (opt.checkEmptyRight && empty(r))) {
      SDL_DEBUG(Hypergraph.Concat, "concat(l, r): r empty -> result empty");
      l.setEmpty();
      return;
    }
    StateIdTranslation x(new StateAddMapping<A>(&l));
    StateId ls = l.start(), lf = l.final();
    StateId rs = r.start(), rf = r.final();
    assert(rf != Hypergraph::kNoState && lf != Hypergraph::kNoState);
    if (l.isFsm() && r.isFsm()) {  // for FSM, add epsilon between final1 and start2
      if (rs == Hypergraph::kNoState || ls == Hypergraph::kNoState)
        SDL_THROW3(FsmNoStartException, "concat(l, r) both fsm - both should have start states", ls, rs);
      if (opt.mergeFsmFinalStart && !(countOutArcs(l, lf) && countInArcs(r, rs)))  // can merge states if
        // don't have both out and
        // in arcs meeting on
        // shared final state:
        x.add(rs, lf);
      else
        l.addArcFsa(lf, x.stateFor(rs));  // epsilon from lfinal to rstart
      l.setFinal(x.stateFor(rf));
      copyArcs(x, r, &l);
      x.transferLabels(r, l);  // transferLabelsPartial ?
    } else {  // for CFG, add S->S1 S2. note: since only one axiom is allowed, share (remap) start states
      if (rs != Hypergraph::kNoState) {
        if (ls != Hypergraph::kNoState)
          x.add(rs, ls);
        else
          l.setStart(x.stateFor(rs));
      }
      copyArcs(x, r, &l);
      x.transferLabels(r, l);
      StateIdContainer ff;
      ff.push_back(lf);
      assert(lf != Hypergraph::kNoState);
      ff.push_back(x.stateFor(rf));
      assert(ff.back() != Hypergraph::kNoState);
      StateId newf = l.addState();
      if (opt.cfgFinalLabel)
        l.setLabelPair(newf, LabelPair(l.inputLabel(lf), r.inputLabel(rf)));
      else
        l.setInputLabel(newf, l.getVocabulary()->add(opt.defaultFinalLabel, sdl::kNonterminal));
      l.setFinal(newf);
      A* ss = new A(newf, ff);
      l.addArc(ss);  // S->Sl Sr
    }
  }
  void operator()(IHypergraph<A> const& h, IMutableHypergraph<A>* o) const {
    SDL_THROW_LOG(Hypergraph, UnimplementedException, "unimplemented in->out transform");
  }
};

template <class Arc>
void hgConcat(IMutableHypergraph<Arc>* pLeftHg, const IHypergraph<Arc>& rightHg, ConcatOptions const& opt) {
  Concat<Arc> x(ptrNoDelete(rightHg), opt);
  inplace(*pLeftHg, x);
}

template <class Arc>
void hgConcat(const IHypergraph<Arc>& leftHg, const IHypergraph<Arc>& rightHg,
              IMutableHypergraph<Arc>* result, ConcatOptions const& opt) {
  Concat<Arc> x(ptrNoDelete(rightHg), opt);
  inout(leftHg, result, x);
}


}}

#endif
