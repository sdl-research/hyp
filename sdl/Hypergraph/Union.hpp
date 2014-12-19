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

/** \file

    union of two hypergraphs: union of the weighted trees in them

    TODO: optimize for (no inarcs to start of either A or B) instead of always
    adding new epsilon arcs
*/

#ifndef HYP__HYPERGRAPH_UNION_HPP
#define HYP__HYPERGRAPH_UNION_HPP
#pragma once

// TODO: use HypergraphCopy and StateIdTranslation

#include <map>

#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>

#include <sdl/Util/Map.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Unordered.hpp>

namespace sdl {
namespace Hypergraph {

namespace UnionHelper {
/**
   Copies an arc, adding all involved states to the resulting
   Hypergraph (unless already there).
*/
template <class Arc>
struct ArcCopyFct {

  typedef unordered_map<StateId, StateId> StatePairMap;

  ArcCopyFct(const IHypergraph<Arc>& source, IMutableHypergraph<Arc>* pTarget)
      : source_(source), pTarget_(pTarget) {}

  void operator()(Arc* pArc) const {
    Arc* newArc = new Arc();
    newArc->setHead(getNewStateId(pArc->head()));
    for (TailId i = 0, e = (TailId)pArc->getNumTails(); i < e; ++i) {
      newArc->addTail(getNewStateId(pArc->getTail(i)));
    }
    newArc->setWeight(pArc->weight());
    pTarget_->addArc(newArc);
  }

  StateId getNewStateId(StateId oldSid) const {
    StateId* pNewState;
    if (Util::update(old2new_, oldSid, pNewState))
      return (*pNewState = pTarget_->addState(source_.labelPair(oldSid)));
    return *pNewState;
  }

  /**
     call only after visiting all arcs.
  */
  StateId getNewStart() { return old2new_[source_.start()]; }

  StateId getNewFinal() { return old2new_[source_.final()]; }

  IHypergraph<Arc> const& source_;
  IMutableHypergraph<Arc>* pTarget_;
  mutable StatePairMap old2new_;  // this is ok only because forArcs passes visitors by const ref.
};
}

/**
   Adds the sourceFst to the pTargetFst.
*/
template <class Arc>
void fstUnion(const IHypergraph<Arc>& sourceFst, IMutableHypergraph<Arc>* pTargetFst) {
  SDL_DEBUG(Hypergraph.Union, "Enter fstUnion");
  ASSERT_VALID_HG(sourceFst);
  if (empty(sourceFst)) {  // FIXME: this is not a cheap operation. prunedEmpty is, though.
    SDL_DEBUG(Hypergraph.Union, "union(target)+=source: source is empty. returning w/ no change.\n");
    return;
  }

  StateId origStartSid = pTargetFst->start();
  StateId origFinalSid = pTargetFst->final();

  // Write sourceFst into pTargetFst
  UnionHelper::ArcCopyFct<Arc> fct(sourceFst, pTargetFst);
  sourceFst.forArcs(fct);  // must store arcs

  StateId epsLabelState = pTargetFst->addState(EPSILON::ID);

  // Add common new start state
  StateId unionStartSid = pTargetFst->addState();
  pTargetFst->addArc(new Arc(Head(origStartSid), Tails(unionStartSid, epsLabelState)));
  pTargetFst->addArc(new Arc(Head(fct.getNewStart()), Tails(unionStartSid, epsLabelState)));
  pTargetFst->setStart(unionStartSid);

  // Add common new final state
  StateId unionFinalSid = pTargetFst->addState();
  pTargetFst->addArc(new Arc(Head(unionFinalSid), Tails(origFinalSid, epsLabelState)));
  pTargetFst->addArc(new Arc(Head(unionFinalSid), Tails(fct.getNewFinal(), epsLabelState)));
  pTargetFst->setFinal(unionFinalSid);
  ASSERT_VALID_HG(*pTargetFst);
}

template <class Arc>
void hgUnion(const IHypergraph<Arc>& sourceHg, IMutableHypergraph<Arc>* pTargetHg) {
  if (sourceHg.isFsm() && pTargetHg->isFsm()) { fstUnion(sourceHg, pTargetHg); } else {
    UnionHelper::ArcCopyFct<Arc> copier(sourceHg, pTargetHg);
    sourceHg.forArcs(copier);

    StateId epsLabelStateId = pTargetHg->addState(EPSILON::ID);

    // Add common new final state
    StateId superfinal = pTargetHg->addState();
    pTargetHg->addArc(new Arc(Head(superfinal), Tails(pTargetHg->final(), epsLabelStateId)));
    pTargetHg->addArc(new Arc(Head(superfinal), Tails(copier.getNewFinal(), epsLabelStateId)));
    pTargetHg->setFinal(superfinal);
  }
}


}}

#endif
