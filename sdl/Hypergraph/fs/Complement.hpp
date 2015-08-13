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

    complement: for a vocabulary, include all strings not in the input hg

   construction: roughly speaking, take a determinized transducer with a
   transition for every symbol, then invert final(x)

   input is determinized w/o epsilons (so has 0 or 1 accepting paths for all
   strings). so the first divergence from that accepting path means the string
   isn't in input set, so is in complement. furthermore, any path that reaches
   end-of-string early, and wasn't final in input, should be final in complement.

   if input doesn't contain empty string, then simply add a new sink "failure"
   state with rho self-loop, then from every state, enter that with a rho (else),
   unless that state already has a sigma or rho. if empty string was in original,
   then ensure that we don't have it in complement.

*/

#ifndef HYPERGRAPH_COMPLEMENT_HPP
#define HYPERGRAPH_COMPLEMENT_HPP
#pragma once

#include <sdl/Util/Forall.hpp>
#include <graehl/shared/os.hpp>

#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Determinize.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Hypergraph/Assert.hpp>

#include <algorithm>

#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Hypergraph {
namespace fs {

template <class SpecialSymbol, class Arc>
struct NonSpecialInput {
  typedef IHypergraph<Arc> H;
  NonSpecialInput(H const& h) : h(h) {}
  H const& h;
  bool operator()(Arc const& a) const { return h.getFsmInput(a) != SpecialSymbol::ID; }
};

template <class Arc>
void complement(IHypergraph<Arc> const& inhg, IMutableHypergraph<Arc>* result) {
  ASSERT_VALID_HG(inhg);
  const Sym complementSigma = RHO::ID;  // FIXME: actual SIGMA may fail in compose; check. try RHO instead?
  result->offerVocabulary(inhg);
  if (empty(inhg)) {
    // result is sigma*
    result->setAllStrings(complementSigma);
    return;
  }

  // deteriminize:
  typedef IHypergraph<Arc> H;
  shared_ptr<H const> det;  // = ptr(inhg); inplace(inhg, Determinize(DETERMINIZE_INPUT, kStoreOutArcs));
  // determinize only if needed:
  H const& hg = determinized(inhg, det, DETERMINIZE_INPUT);

  // store arcs:
  result->forceProperties(kStoreFirstTailOutArcs | kFsm | kGraph, true);

  /*

    the construction is: from every state, including the old final state, add a
    rho (else) transition to a new final state, where once you get there, you
    can stay there with any symbol. obviously if there already was a wildcard
    (sigma) or else (rho), then there are no "else" remaining

    if original didn't contain empty string, we need to add it to the result
    set. but we can't go to the "stay there" final state with epsilon, or
    we'd accept everything. so we then need two final states; for that, we
    need a new final state with epsilons from both start (for the empty
    string) and the old sink state

  */

  bool hadempty = containsEmptyStringDet(hg);

  copyHypergraphPartial(hg, result, NonSpecialInput<EPSILON, Arc>(hg));
  // TODO: complain if any epsilons except those needed to simulate multiple
  // final states

  StateId startState = result->start();
  StateId rhoLabelState = result->addState(RHO::ID);
  StateId sigmaLabelState = result->addState(complementSigma);

  StateId newFinal = result->nextStateId();

  IVocabularyPtr pVoc = result->getVocabulary();
  assert(pVoc != 0);
  bool reachedNewFinal = false;
  StateIdRange states = result->getStateIds();
  forall (StateId sid, states) {
    const bool isLexicalState = result->inputLabel(sid).isTerminal();
    // if ((!isLexicalState && result->numOutArcs(sid)) || sid == newFinal) {
    typedef typename Arc::Weight Weight;
    if (!isLexicalState && sid != newFinal && !hg.hasAllOut(sid)) {
      // ONLY IF there isn't already a sigma or rho, and state wasn't final (epsilon to oldFinal)
      result->addArcFsm(sid, newFinal, rhoLabelState);
      reachedNewFinal = true;
    }
  }
  // stay in final state on any symbol
  if (reachedNewFinal) {
    result->addStateId(newFinal);
    result->setFinal(newFinal);
    result->addArcFsm(newFinal, newFinal, sigmaLabelState);
  }
  // can't direct empty string to "stay in with any symbol", so need new final state that both start and
  // newfinal point at
  if (!hadempty) {  // then complement must contain empty.
    StateId epsilonState = result->addState(EPSILON::ID);
    StateId emptyFinal;
    result->setFinal(emptyFinal = result->addState());
    result->addArcFsm(startState, emptyFinal, epsilonState);
    if (reachedNewFinal) result->addArcFsm(newFinal, emptyFinal, epsilonState);
  } else if (!reachedNewFinal)
    result->setEmpty();

  ASSERT_VALID_HG(*result);
}

/**
   null options class allowing us to use
   Hypergraph::inplace(hg, ComplementOptions()), xmt
   REGISTER_TRANSFORM_AS_MODULE, etc
*/
struct Complement : SimpleTransform<Complement, Transform::Inout>, TransformOptionsBase {
  Complement() {}
  explicit Complement(TransformOptionsBase const& base) {}
  static char const* type() { return "Complement"; }
  std::string checkInputsHelp() { return "must be FSA"; }
  template <class A>
  bool checkInputs(IHypergraph<A> const& h) const {
    return h.isFsm();
  }
  template <class Arc>
  void inout(IHypergraph<Arc> const& hg, IMutableHypergraph<Arc>* pResultHg) const {
    complement(hg, pResultHg);
  }
};


}}}

#endif
