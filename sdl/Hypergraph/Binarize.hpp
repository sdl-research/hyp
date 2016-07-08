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

    binarize a CFG/hg so each arc has at most 2 tails.
*/


#ifndef HYPERGRAPH_BINARIZE_HPP
#define HYPERGRAPH_BINARIZE_HPP
#pragma once

#include <sdl/Hypergraph/DeferAddArcs.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/RestoreArcProperties.hpp>
#include <sdl/HypergraphExt/AlignmentFeatures.hpp>
#include <sdl/Util/Enum.hpp>

namespace sdl {
namespace Hypergraph {

SDL_ENUM(BinarizeWhen, 3, (Never, Always, One_Lexical))

struct BinarizeOptions {
  BinarizeOptions() { Config::inits(this); }
  BinarizeWhen binarizeWhen;
  template <class Config>
  void configure(Config& config) {
    config("binarize-when", &binarizeWhen)
        .init(kAlways)(
            "split arcs with many tail: never, always (#tails becomes at most 2), one-lexical (at most one "
            "lexical per arc - maybe more than 2 tails)");
  }
};

/**
   split is called for i=[1...#tails) and if you return true it means start a new arc at that position
*/
struct BinarizeAlways {
  void startTails() const {}
  bool split(TailId i, Sym inputLabel) { return i > 2; }
  enum { kAddEpsilonToOneTailArcs = false };
};

/// add <eps> labeled tail to one-tail arcs if kAddEpsilonToOneTailArcs
struct BinarizeAlwaysToFsm : BinarizeAlways {
  enum { kAddEpsilonToOneTailArcs = true };
};

struct BinarizeOneLexicalPerArc {
  enum { kAddEpsilonToOneTailArcs = false };
  bool anyLex;
  void startTails() { anyLex = false; }
  bool split(TailId i, Sym inputLabel) {
    if (anyLex) {
      if (inputLabel.isLexical()) return true;
    } else
      anyLex = inputLabel.isLexical();
    return false;
  }
};


/**
   binarizes arc's tails (a b c d) -> (((a b) c) d)

   where weight is only on first arc (a b)

   OK if first-tail outarcs only; otherwise use deferred-add version which can
   handle in-arcs as well
*/
template <class When, class Arc>
void binarizeSingleArc(IMutableHypergraph<Arc>& hg, Arc* arc, When& when, bool cloneAlignments = true) {
  StateIdContainer& tails = arc->tails_;
  TailId const tail0 = tails[0];
  TailId N = (TailId)tails.size();
  assert(hg.firstTailOnly());
  StateIdContainer oldTails(1, tail0);
  tails.swap(oldTails);
  when.startTails();
  typedef typename Arc::Weight Weight;
  Weight const& inWeight = arc->weight_;
  Arc* lastArc = arc;
  StateId const dstState = arc->head_;
  for (TailId i = 1; i < N; ++i) {
    TailId const tail = oldTails[i];
    Sym const inputLabel = hg.inputLabel(tail);
    if (when.split(i, inputLabel)) {
      StateId const lastHead = hg.addState();
      lastArc->head_ = lastHead;
      lastArc = new Arc(lastHead, tail);
      hg.addArc(lastArc);
      if (cloneAlignments) copyAlignmentFeatures(inWeight, lastArc->weight_);
    } else
      lastArc->tails_.push_back(tail);
  }
  lastArc->head_ = dstState;
}

template <class When, class Arc>
void binarizeSingleArcDeferAdditional(IMutableHypergraph<Arc>& hg, Arc* arc, When& when,
                                      DeferAddArcs<Arc>& addArcsLater) {
  StateIdContainer& tails = arc->tails_;
  TailId const tail0 = tails[0];
  TailId N = (TailId)tails.size();
  StateIdContainer oldTails(1, tail0);
  tails.swap(oldTails);
  when.startTails();
  Arc* lastArc = arc;
  StateId const dstState = arc->head_;
  for (TailId i = 1; i < N; ++i) {
    TailId const tail = oldTails[i];
    Sym const inputLabel = hg.inputLabel(tail);
    if (when.split(i, inputLabel)) {
      StateId const lastHead = hg.addState();
      lastArc->head_ = lastHead;
      lastArc = new Arc(lastHead, tail);
      addArcsLater.push_back(lastArc);
    } else
      lastArc->tails_.push_back(tail);
  }
  lastArc->head_ = dstState;
}

/// owns arc (via addArcsLater) and possibly creates some new ones
template <class When, class Arc>
void binarizeSingleArcDefer(IMutableHypergraph<Arc>& hg, Arc* arc, When& when, DeferAddArcs<Arc>& addArcsLater) {
  addArcsLater.push_back(arc);
  binarizeSingleArcDeferAdditional(hg, arc, when, addArcsLater);
}

/**
   binarizes arcs' tails (a b c d) -> (((a b) c) d)

   where weight is only on first arc (a b)

   TODO: option to clone alignment features so they appear on every arc

   \param add <eps> labeled tail to one-tail arcs if kAddEpsilonToOneTailArcs if
   When::kAddEpsilonToOneTailArcs
  */
template <class When, class Arc>
void leftBranchingBinarize(IMutableHypergraph<Arc>& hg) {
  DeferAddArcs<Arc> addArcsLater(hg);  // these get added later so we can iterate safely over arcs
  addArcsLater.reserve(hg.estimatedNumEdges());
  StateId epsState = kNoState;
  When when;
  hg.forArcs([&](Arc* arc) {
    StateIdContainer& tails = arc->tails_;
    addArcsLater.push_back(arc);
    if (When::kAddEpsilonToOneTailArcs && tails.size() == 1) {
      if (epsState == kNoState) epsState = hg.addState(kEpsilonLabelPair);
      tails.push_back(epsState);
    } else
      binarizeSingleArcDeferAdditional(hg, arc, when, addArcsLater);
  });
  hg.releaseArcs();  // we defer-added original arcs already
}

template <class Arc>
void binarizeAlways(IMutableHypergraph<Arc>* hg) {
  leftBranchingBinarize<BinarizeAlways>(*hg);
}

template <class Arc>
void binarize(IMutableHypergraph<Arc>* hg) {
  if (!hg->isFsm()) leftBranchingBinarize<BinarizeAlways>(*hg);
}

template <class Arc>
void ensureFsm(shared_ptr<IHypergraph<Arc> const>& hg) {
  if (hg->isFsm()) return;
  if (!hg->isGraph()) {
    SDL_THROW_LOG(Hypergraph.Binarize, ConfigException,
                  "hypergraph is not a graph, so can't ensureGraphIsFsm");
  } else {
    shared_ptr<IHypergraph<Arc> const> oldHg(hg);
    typedef MutableHypergraph<Arc> Hg;
    Hg* r = copyHypergraphNew(*hg, hg, kStoreFirstTailOutArcs);
    leftBranchingBinarize<BinarizeAlwaysToFsm>(*r, RestoreArcPropertiesOptions());
  }
}

template <class Arc>
void binarizeOneLexicalPerArc(IMutableHypergraph<Arc>* hg) {
  if (!hg->hasAtMostOneLexicalTail()) leftBranchingBinarize<BinarizeOneLexicalPerArc>(*hg);
}

template <class Arc>
void ensureOneLexicalPerArc(shared_ptr<IHypergraph<Arc> const>& hg) {
  if (hg->isFsm()) return;
  if (!hg->isGraph()) {
    SDL_THROW_LOG(Hypergraph.Binarize, ConfigException,
                  "hypergraph is not a graph, so can't ensureGraphIsFsm");
  } else {
    if (hg->hasAtMostOneLexicalTail()) return;
    SDL_INFO(Hypergraph.ensureOneLexicalPerArc,
             "binarizing graph to ensure at most one lexical label per arc");
    shared_ptr<IHypergraph<Arc> const> oldHg(hg);
    typedef MutableHypergraph<Arc> Hg;
    Hg* r = copyHypergraphNew(*hg, hg, kStoreFirstTailOutArcs);
    leftBranchingBinarize<BinarizeOneLexicalPerArc>(*r);
  }
}

template <class Arc>
void binarize(IMutableHypergraph<Arc>& hg, BinarizeOptions const& options) {
  switch (options.binarizeWhen) {
    case kAlways: return binarize(&hg);
    case kOne_Lexical: return binarizeOneLexicalPerArc(&hg);
    default:;
  }
}


}}

#endif
