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
#include <sdl/Hypergraph/Assert.hpp>

namespace sdl {
namespace Hypergraph {


template <class Arc>
void addGraphEpsilon(IMutableHypergraph<Arc>& hg, StateId epsLabelState, bool graphOutput, StateId from,
                     StateId to) {
  if (graphOutput)
    hg.addArcGraphEpsilon(from, to);
  else
    hg.addArcFsm(from, to, epsLabelState);
}

namespace UnionHelper {
/**
   Copies an arc, adding all involved states to the resulting
   Hypergraph (unless already there).
*/
template <class Arc>
struct ArcCopyFct {

  typedef unordered_map<StateId, StateId> StatePairMap;

  ArcCopyFct(IHypergraph<Arc> const& source, IMutableHypergraph<Arc>* pTarget)
      : source_(source), pTarget_(pTarget) {}

  void operator()(ArcBase* pArc) const {
    Arc const& from = *(Arc*)pArc;
    Arc* copied = new Arc(from, stateId(from.head_));
    for (unsigned i = 0, N = from.tails_.size(); i < N; ++i) copied->tails_[i] = stateId(from.tails_[i]);
    pTarget_->addArc(copied);
  }

  StateId stateId(StateId oldState) const {
    StateId* pNewState;
    if (Util::update(old2new_, oldState, pNewState))
      return (*pNewState = pTarget_->addState(source_.labelPair(oldState)));
    return *pNewState;
  }

  StateId existingStateId(StateId s) const { return s == kNoState ? s : old2new_[s]; }

  /**
     call only after visiting all arcs.
  */
  StateId start() { return existingStateId(source_.start()); }
  StateId final() { return existingStateId(source_.final()); }

  IHypergraph<Arc> const& source_;
  IMutableHypergraph<Arc>* pTarget_;
  mutable StatePairMap old2new_;  // this is ok only because forArcs passes visitors by const ref.
};
}

/**
   Adds the sourceFst to the pTargetFst.
*/
template <class Arc>
void fstUnion(IHypergraph<Arc> const& sourceFst, IMutableHypergraph<Arc>* pTargetFst) {
  SDL_DEBUG(Hypergraph.Union, "Enter fstUnion");
  ASSERT_VALID_HG(sourceFst);
  if (empty(sourceFst)) {  // FIXME: this is not a cheap operation. prunedEmpty is, though.
    SDL_DEBUG(Hypergraph.Union, "union(target)+=source: source is empty. returning w/ no change.\n");
    return;
  }

  StateId origStartState = pTargetFst->start();
  StateId origFinalState = pTargetFst->final();

  // Write sourceFst into pTargetFst
  UnionHelper::ArcCopyFct<Arc> fct(sourceFst, pTargetFst);
  sourceFst.forArcs(fct);  // must store arcs

  StateId epsLabelState = pTargetFst->addState(EPSILON::ID);

  // Add common new start state
  StateId unionStartState = pTargetFst->addState();
  pTargetFst->addArc(new Arc(HeadAndTail(), origStartState, unionStartState, epsLabelState));
  pTargetFst->addArc(new Arc(HeadAndTail(), fct.start(), unionStartState, epsLabelState));
  pTargetFst->setStart(unionStartState);

  // Add common new final state
  StateId unionFinalState = pTargetFst->addState();
  pTargetFst->addArc(new Arc(HeadAndTail(), unionFinalState, origFinalState, epsLabelState));
  pTargetFst->addArc(new Arc(HeadAndTail(), unionFinalState, fct.final(), epsLabelState));
  pTargetFst->setFinal(unionFinalState);
  ASSERT_VALID_HG(*pTargetFst);
}


template <class Arc>
void hgUnion(IHypergraph<Arc> const& sourceHg, IMutableHypergraph<Arc>* pTargetHg) {
  if (sourceHg.isFsm() && pTargetHg->isFsm()) {
    fstUnion(sourceHg, pTargetHg);
  } else {
    UnionHelper::ArcCopyFct<Arc> copier(sourceHg, pTargetHg);
    sourceHg.forArcs(copier);

    StateId epsLabelState = pTargetHg->addState(EPSILON::ID);

    // Add common new final state
    StateId superfinal = pTargetHg->addState();
    pTargetHg->addArc(new Arc(HeadAndTail(), superfinal, pTargetHg->final(), epsLabelState));
    pTargetHg->addArc(new Arc(HeadAndTail(), superfinal, copier.final(), epsLabelState));
    pTargetHg->setFinal(superfinal);
  }
}

/**
   n-way union construction (instead of binary fstUnion list/tree). updates
   fst[0] in-place, so faster if fsts[0] is the largest

   if !graphOut, add explicit <eps>
 */
template <class Arc>
shared_ptr<Hypergraph::IMutableHypergraph<Arc>>
graphMultiUnion(std::vector<shared_ptr<IMutableHypergraph<Arc>>>& graphs, bool graphOutput = true) {
  typedef IMutableHypergraph<Arc> Hg;
  typedef shared_ptr<Hg> HgPtr;

  if (graphs.empty()) return HgPtr();

  /// the resulting union of graphs[0...]
  HgPtr const& union0ptr = graphs[0];
  Hg& union0 = *union0ptr;
  StateId start0 = union0.start();
  StateId final0 = union0.final();
  StateId epsLabelState = graphOutput ? kNoState : union0.addState(EPSILON::ID);
  StateId start = union0.addState();
  StateId final = union0.addState();
  union0.setStart(start);
  union0.setFinal(final);

  addGraphEpsilon(union0, epsLabelState, graphOutput, start, start0);
  addGraphEpsilon(union0, epsLabelState, graphOutput, final0, final);

  for (typename std::vector<HgPtr>::const_iterator i = graphs.begin() + 1, e = graphs.end(); i != e; ++i) {
    IHypergraph<Arc> const& graph = **i;
    // copy g arcs into pTargetGraph (creating new states)
    UnionHelper::ArcCopyFct<Arc> copied(graph, &union0);
    graph.forArcs(copied);
    addGraphEpsilon(union0, epsLabelState, graphOutput, start, copied.start());
    addGraphEpsilon(union0, epsLabelState, graphOutput, copied.final(), final);
  }
  return union0ptr;
}


}}

#endif
