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

    used by CharDetokModule-modify lexical labeled states' tokens
*/

#ifndef HYP__SPLITSTATE_HPP
#define HYP__SPLITSTATE_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/InArcs.hpp>
#include <sdl/Hypergraph/StatesTraversal.hpp>
#include <sdl/Vocabulary/Glue.hpp>
#include <sdl/Util/String.hpp>
#include <sdl/Util/Utf8Pred.hpp>
#include <sdl/Sym.hpp>
#include <boost/array.hpp>

namespace sdl {
namespace Hypergraph {

/*
  split states based on its glue symbols(__LW_AT__).
*/
const unsigned glue_len = (unsigned)Vocabulary::kGlueAffix.length();

enum StateType {
  kStart,
  kNoGlueNoSpace,
  kNoGlueSpace,
  kGlueLeft,
  kGlueRight,
  kGlueBoth,
  kSpecial,
  knStateType
};

/**
   add space to the beginning of the symbol if no glue indicator
*/
inline Sym detokSymbol(StateType frontType, StateType currentType, Sym const& origSym, IVocabularyPtr pVoc,
                       bool spaceBetween) {
  std::string sym = pVoc->str(origSym);
  if (currentType == kGlueLeft || currentType == kGlueBoth) {
    sym.erase(sym.begin(), sym.begin() + glue_len);
  }
  if (currentType == kGlueRight || currentType == kGlueBoth) {
    sym.erase(sym.end() - glue_len, sym.end());
  }
  if (frontType != kStart) {
    if (!(frontType == kGlueRight || frontType == kGlueBoth || currentType == kGlueLeft
          || currentType == kGlueBoth || (frontType == kNoGlueNoSpace && currentType == kNoGlueNoSpace)
          || (!spaceBetween && (frontType == kNoGlueNoSpace || currentType == kNoGlueNoSpace)))) {
      sym = " " + sym;
    }
  }
  return pVoc->add(sym, kTerminal);
}

inline StateType detectSymType(Sym const& symId, Util::Utf8RangePred const& pred, IVocabularyPtr pVoc) {
  if (symId.isSpecial()) return kSpecial;
  std::string const& sym = pVoc->str(symId);
  if (sym.length() < glue_len) return pred(sym) ? kNoGlueNoSpace : kNoGlueSpace;
  if (Util::startsWith(sym, Vocabulary::kGlueAffix))
    return Util::endsWith(sym, Vocabulary::kGlueAffix) ? kGlueBoth : kGlueLeft;
  else if (Util::endsWith(sym, Vocabulary::kGlueAffix))
    return kGlueRight;
  else
    return pred(sym) ? kNoGlueNoSpace : kNoGlueSpace;
}


// TODO: merge into SplitStateVisitor? there's no other policy
template <class Arc>
struct TokenSplitPolicy {

  typedef boost::array<StateId, knStateType> StateSplits;
  TokenSplitPolicy(IHypergraph<Arc> const& hg, Util::Utf8RangePred pred, bool spaceBetween)
      : hg_(hg), pVoc_(hg.getVocabulary()), pred_(pred), spaceBetween_(spaceBetween) {
    assert(hg.isGraph());
    assert(hg.hasAtMostOneLexicalTail());
  }
  /*
    detect the type of the arc's head state

    Start : start state

    NoGlueNoSpace : no __LW_AT__ , no spaces between tokens

    NoGlueSpace : no __LW_AT__ , insert space at left and right

    GlueLeft : __LW_AT__ at the left

    GlueRight : __LW_AT__ at the right

    GlueBoth : __LW_AT__ at both sides

    Special : special tokens (should be treated similar to start, probably, but
    for status quo, seems to insert a space unlike Start)
  */

  StateType headType(Sym firstLexical) {
    return firstLexical ? detectSymType(firstLexical, pred_, pVoc_) : kSpecial;
  }

  Sym resultingSymbol(Arc* a, Sym firstLexical, StateType type1, StateType type2) {
    if (firstLexical)
      return detokSymbol(type1, type2, firstLexical, pVoc_, spaceBetween_);
    else {
      StateIdContainer const& tails = a->tails();
      if (tails.size() != 2)
        return EPSILON::ID;
      else {
        Sym const osym = hg_.outputLabel(tails[1]);
        return osym ? osym : EPSILON::ID;
      }
    }
  }

  /*
    connect s1 and s2 with the arc

    first get rid of "__LW_AT__" on the original symbol from arc, then
    based on s1's type, type1, and s2's type, type2,
    an additionaly space may or may not be added to the begining of the symbol.
  */
  void connect(Arc* arc, Sym firstLexical, StateType type1, StateId s1, StateType type2, StateId s2,
               IMutableHypergraph<Arc>* pHgResult) {
    // pHgResult->addStateId(s2);
    StateId const labelSt = pHgResult->addState(resultingSymbol(arc, firstLexical, type1, type2));
    pHgResult->addArc(new Arc(s1, labelSt, arc->weight(), s2));
  }
  IHypergraph<Arc> const& hg_;
  IVocabularyPtr pVoc_;
  Util::Utf8RangePred pred_;
  bool spaceBetween_;
};

/*
  visitor class that is intended be used with topological traversal.

  it will split a state based on its incomming arcs.
  one state from the input lattice could have multiple incoming arcs, therefore multiple
  types. The policy class decides the types for each state and connects them with
  arcs.

  used by CharDetokModule
  */
template <class Arc>
struct SplitStateVisitor : IStatesVisitor, TokenSplitPolicy<Arc> {
  typedef TokenSplitPolicy<Arc> SplitPolicy;
  typedef typename SplitPolicy::StateSplits StateSplits;

  enum { kUnsplitState = (StateId)-1 };
  SplitStateVisitor(IHypergraph<Arc> const& hg, IMutableHypergraph<Arc>* pHgResult, Util::Utf8RangePred pred,
                    bool spaceBetween)
      : SplitPolicy(hg, pred, spaceBetween)
      , hg_(hg)
      , outHg_(pHgResult)
      , final_(kNoState)
      , maxNotTerminalState_(hg.maxNotTerminalState())
      , startStates_(maxNotTerminalState_ + 1) {
    startStates_.set(hg_.start());
    inputFinal_ = hg_.final();
    SDL_DEBUG(Hypergraph.SplitState, "hg #states=" << hg.size() << " max not-terminal state id="
                                                   << hg.maxNotTerminalState() << " final=" << inputFinal_);
    outHg_->setStart(start_ = outHg_->addState());
    StateSplits splits;
    splits.assign(kUnsplitState);
    stateSplitsFor_.resize(maxNotTerminalState_ + 1, splits);
    hgAsMutable_ = hg_.isMutable() ? static_cast<IMutableHypergraph<Arc> const*>(&hg_) : 0;
  }
  IMutableHypergraph<Arc> const* hgAsMutable_;


  void visit(StateId s) {
    if (s > maxNotTerminalState_) return;
    isStart_ = startStates_.test(s);
    if (!isStart_ && hg_.isAxiom(s)) return;
    visitOutArcs(*this, s, hg_, hgAsMutable_);
  }

  void acceptOut(ArcBase* a, StateId s) {
    Arc* arc = (Arc*)a;
    Sym const arcSym = hg_.firstLexicalOutput(arc);
    bool const extendStart = isStart_ && !arcSym;
    StateType const headType = extendStart ? kStart : this->headType(arcSym);
    StateId head = arc->head();
    if (extendStart) startStates_.set(head);
    assert(!hg_.hasTerminalLabel(head));
    if (head > maxNotTerminalState_) {
      assert(head != inputFinal_);
      return;
    }
    assert(head < stateSplitsFor_.size());
    StateSplits& headSplits = stateSplitsFor_[head];
    StateId& headSplit = headSplits[headType];
    assert(headType < knStateType);
    if (isStart_) {
      if (head == inputFinal_) {
        if (final_ == kNoState) final_ = outHg_->addState();
        this->connect(arc, arcSym, kStart, start_, headType, final_, outHg_);
      } else {
        if (headSplit == kUnsplitState) headSplit = outHg_->addState();
        this->connect(arc, arcSym, kStart, start_, headType, headSplit, outHg_);
      }
    } else {
      if (headSplit == kUnsplitState) {
        headSplit = outHg_->addState();
        if (head == inputFinal_) {
          for (std::size_t i = 0; i < StateSplits::static_size; ++i) headSplits[i] = headSplit;
          final_ = headSplit;
        }
      }
      StateSplits const& sSplits = stateSplitsFor_[s];
      for (std::size_t i = 0; i < StateSplits::static_size; ++i)
        if (sSplits[i] != kUnsplitState)
          this->connect(arc, arcSym, (StateType)i, sSplits[i], headType, headSplit, outHg_);
    }
  }

  StateId finishFinal() {
    if (final_ != kNoState) outHg_->setFinal(final_);
    return final_;
  }

  IHypergraph<Arc> const& hg_;
  IMutableHypergraph<Arc>* outHg_;
  std::vector<StateSplits> stateSplitsFor_;
  StateId start_, final_;
  StateId inputFinal_, maxNotTerminalState_;

  StateSet startStates_;
  // a set because we consider special-symbol-reachable-from-start to also count
  // as kStart type (don't add space before)

  bool isStart_;
};


}}

#endif
