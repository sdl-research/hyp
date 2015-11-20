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

    actual hypergraph storing its data (in/out arcs, labels) in vectors.

    TODO: we lose a lot of cache-efficiency by having Arc * allocated in
    different places. we should try switching to holding Arc by value (perhaps
    using Util/StableVector), or at least provide a
    preallocate-chunk-of-contiguous Arc* facility when we can estimate the
    number of arcs needed in advance
*/

#ifndef HYP__HYPERGRAPH_MUTABLEHYPERGRAPH_HPP
#define HYP__HYPERGRAPH_MUTABLEHYPERGRAPH_HPP
#pragma once


#include <cassert>
#include <vector>
#include <algorithm>
#include <sdl/SharedPtr.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/StateIdTranslation.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Util/Latch.hpp>

#include <sdl/Util/ShrinkVector.hpp>
#include <sdl/Util/Interested.hpp>
#include <sdl/Hypergraph/src/IsGraphArc.ipp>
#include <sdl/Util/MinMax.hpp>

namespace sdl {
namespace Hypergraph {

/// common impl for IMutableHypergraph<Arc>
struct MutableHypergraphLabels {
  typedef std::vector<Sym> LabelForState;

  Sym inputLabelImpl(StateId s) const { return Util::getOrElse(iLabelForState, s, NoSymbol); }

  Sym outputLabelImpl(StateId s) const {
    Sym r = Util::getOrElse(oLabelForState, s, NoSymbol);
    return r ? r : inputLabelImpl(s);
  }

  bool hasLexicalLabelImpl(StateId s) const {
    return s < iLabelForState.size()
           && (iLabelForState[s].isLexical() || s < oLabelForState.size() && oLabelForState[s].isLexical());
  }

  bool hasLexicalInputLabelImpl(StateId s) const {
    return s < iLabelForState.size() && iLabelForState[s].isLexical();
  }

  bool elseHasLexicalOutputLabelImpl(StateId s) const {
    return s < oLabelForState.size() && oLabelForState[s].isLexical();
  }

 protected:
  typedef unordered_map<LabelPair, StateId> LabelPairState;
  LabelPairState lstate;  // for kCanonicalLex

  IVocabularyPtr pVocab_;
  StateId numStates_;

  LabelForState iLabelForState;
  LabelForState oLabelForState;

  /**
     sets the input label (and if outputLabelFollowsInput(s), the output label as well)

     //TODO: an explicit call for only modifying the input label no matter whether outputLabelFollowsInput ?
     */
  void setInputLabelImpl(StateId s, Sym label) {
    assert(s != kNoState);
    setLabel(iLabelForState, s, label);
  }

  void setOutputLabelImpl(StateId s, Sym label, Properties& hgProperties) {
    assert(s != kNoState);
    setOutputLabelImpl(s, label);
    if (!label) return;  // this means if you set NoSymbol for output, it means "copy input"
    hgProperties |= kHasOutputLabels;
  }

  void setOutputLabelImpl(StateId s, Sym label) {
    // assumes that kHasOutputLabels will be/has already been set to true
    if (s >= oLabelForState.size()) {
      if (!label) return;  // this means if you set NoSymbol for output, it means "copy input"
      oLabelForState.resize(s + 1, NoSymbol);
    }
    oLabelForState[s] = label;
  }

  StateId size() const {
    assert(oLabelForState.size() <= iLabelForState.size());
    return (StateId)iLabelForState.size();
  }

  bool hasLabelImpl(StateId state) const {
    return state < iLabelForState.size() && iLabelForState[state] != NoSymbol;
  }
  // does not clear kCanonicalLex
  void setNewStateLabels(StateId s, Sym inputLabel, Sym outputLabel, Properties& hgProperties) {
    setInputLabelImpl(s, inputLabel);
    assert(!outputLabelBare(s));  // because it's a new state.
    if (inputLabel != outputLabel && outputLabel != NoSymbol) {
      // TODO: ideally let's make Hg arc parser code ask for state with
      // outputLabel NoSymbol if it wasn't explicitly specified, but for now
      // check if inputLabel == outputLabel and treat it as same as no output
      // label.
      setOutputLabelImpl(s, outputLabel);
      hgProperties |= kHasOutputLabels;
    }
  }

  Sym outputLabelBare(StateId s) const { return Util::getOrElse(oLabelForState, s, NoSymbol); }

  ///*Impl: avoid virtual fn overhead, equiv. to IHypergraph virtual method *


  Sym outputLabelOrElse(StateId s, Sym inLabel) const {
    Sym r = Util::getOrElse(oLabelForState, s, NoSymbol);
    return r ? r : inLabel;
  }

  // purely to avoid virtual fn calls; same as IHypergraph isLexical
  bool isLexicalStateImpl(StateId s) const { return inputLabelImpl(s).isLexical(); }
  static inline void updateState(StateId& s, StateIdTranslation& stateRemap) {
    if (s != kNoState) s = stateRemap.stateFor(s);
  }

  static inline void transferLabels(StateIdTranslation const& stateRemap, LabelForState const& lin,
                                    LabelForState& lout)  //, StateId mastateRemapState
  {
    for (StateIdMap::value_type const& io : stateRemap.cache) {
      if (io.first < lin.size()) setLabel(lout, io.second, lin[io.first]);
    }
  }

  static inline void setLabel(LabelForState& l, StateId s, Sym sym) {
    if (s >= l.size()) {
      if (!sym) return;  // this means if you set NoSymbol for output, it means "copy input"
      l.resize(s + 1, NoSymbol);
    }
    l[s] = sym;
  }

  static inline void clearLabel(LabelForState& l, StateId state) {
    if (l.size() > state) l[state] = NoSymbol;
  }

  bool forceCanonicalLexImpl() {
    bool nodup = true;
    for (StateId s = 0, e = size(); s < e; ++s) {
      Sym i = iLabelForState[s];
      if (i.isTerminal())
        nodup &= lstate.insert(LabelPairState::value_type(LabelPair(i, outputLabelOrElse(s, i)), s)).second;
    }
    return (canonicalLexIsCanonical_ = nodup);
  }

  StateId hgSizeForLabels() const { return std::max(iLabelForState.size(), oLabelForState.size()); }

  MutableHypergraphLabels() : canonicalLexIsCanonical_(true) {}

  bool canonicalLexIsCanonical_;
};

typedef std::vector<StateId> InterestedStates;

template <class A>
struct MutableHypergraph final : IMutableHypergraph<A>, private MutableHypergraphLabels {
 private:
  typedef std::vector<ArcsContainer> AdjacentArcs;
  static inline void transform(StateIds const& permutation, StateId& s) {
    assert(s < permutation.size());
    s = permutation[s];
  }
  static inline void maybeTransform(StateIds const& permutation, StateId& s) {
    s = s < permutation.size() ? permutation[s] : kNoState;
  }
  void orderGraphStatesImpl(StateIds const& permutation, StateIds const& inversePermutation, AdjacentArcs& adj) {
    assert(permutation.size() <= numStates_);
    StateId N = std::min(adj.size(), permutation.size());
    AdjacentArcs outAdj(N);
    for (Position i = 0; i < N; ++i) {
      Position const i2 = permutation[i];
      ArcsContainer& out = outAdj[i];
      if (i2 == kNoState) {
        deleteAdjacentArcs(out);
      } else {
        assert(i2 < adj.size());
        out = std::move(adj[i2]);
        for (ArcsContainer::const_iterator i = out.begin(), e = out.end(); i != e; ++i) {
          Arc* a = (Arc*)*i;
          assert(!a->tails_.empty());  // because isGraph
          transform(inversePermutation, a->tails_[0]);
          transform(inversePermutation, a->head_);
        }
      }
    }
    adj = std::move(outAdj);
  }

 public:
  void orderGraphStates(StateIds const& permutation, StateIds const& inversePermutation) override {
    assert(this->isGraph());
    maybeTransform(inversePermutation, this->start_);
    maybeTransform(inversePermutation, this->final_);
    if (this->start_ == kNoState || this->final_ == kNoState) return;
    if (this->properties_ & kStoreFirstTailOutArcs) {
      this->removeInArcs();
      goto FirstTail;
    }
    if (this->properties_ & kStoreInArcs) {
      this->removeOutArcs();
      orderGraphStatesImpl(permutation, inversePermutation, this->inArcsPerState_);
    } else {
      this->forceFirstTailOutArcs();
    FirstTail:
      assert(!(this->properties_ & kStoreOutArcs));
      orderGraphStatesImpl(permutation, inversePermutation, this->outArcsPerState_);
    }
  }

  StateIdInterval possiblyInputTerminalLabeledStatesImpl() const {
    return StateIdInterval(this->properties_ & kSortedStates ? this->sortedStatesNumNotTerminal_ : 0,
                           iLabelForState.size());
  }

  StateIdInterval possiblyInputTerminalLabeledStates() const override {
    return possiblyInputTerminalLabeledStatesImpl();
  }

  // TODO: could do truth-maintenance on this like the kFsmProperties or watching
  // modification of our private iLabelForState
  WhichFstComposeSpecials whichInputFstComposeSpecials() const override {
    WhichFstComposeSpecials r;
    for (StateIdInterval states(possiblyInputTerminalLabeledStatesImpl()); states.first < states.second;
         ++states.first)
      r.check(inputLabel(states.first));
    return r;
  }

  typedef A Arc;
  typedef IHypergraph<A> Base;
  typedef IMutableHypergraph<A> MutableBase;
  typedef ArcsContainer::const_iterator ArcIter;
  typedef shared_ptr<MutableHypergraph<Arc>> Ptr;
  typedef typename A::Weight Weight;
  typedef typename A::ArcFilter ArcFilter;
  typedef std::vector<ArcsContainer> Adjs;
  typedef typename A::ArcVisitor ArcVisitor;

  /// size may be less than #states
  LabelForState const& inputLabels() const { return iLabelForState; }
  /// size may be than #states. missing entry = output same as input
  LabelForState const& outputLabels() const { return oLabelForState; }

  HypergraphBase::MaybeLabels maybeLabels() const override {
    return HypergraphBase::MaybeLabels(&iLabelForState, &oLabelForState);
  }

  using MutableHypergraphLabels::inputLabelImpl;
  using MutableHypergraphLabels::elseHasLexicalOutputLabelImpl;


  typedef StateId const* StateIdPtr;
  TailId endAnnotations(StateIdPtr tails, TailId n) const override {
    for (TailId i = 1; i < n; ++i)
      if (!Vocabulary::isAnnotation(inputLabelImpl(tails[i]))) return i;
    return n;
  }

  StateId sizeForLabels() const override { return hgSizeForLabels(); }

  StateId sizeForInputLabels() const override { return iLabelForState.size(); }

  StateId canonicalExistingStateForLabelPair(LabelPair io) override {
    assert(this->properties_ & kCanonicalLex);
    return Util::getOrElse(lstate, io, kNoState);
  }

  /** \return true iff outputLabel is NoSymbol, and so tracks the input
      label. This can save redundantly transforming the output
      label. (outputLabel returns inputLabel in that case). */
  bool outputLabelFollowsInput(StateId state) const override { return outputLabelBare(state) == NoSymbol; }

  /** \return true if for all state, outputLabelFollowsInput(state). O(1), may return false if not sure */
  bool outputLabelFollowsInput() const override { return oLabelForState.empty(); }

  void removeDeletedArcs(Util::PointerSet const& deletedArcPointers) override {
    removeDeletedArcs(deletedArcPointers, inArcsPerState_);
    removeDeletedArcs(deletedArcPointers, outArcsPerState_);
  }

  void addedTail(Arc* a, StateId tail) override {
    if (this->properties_ & kStoreOutArcs) Util::atExpand(outArcsPerState_, tail).push_back(a);
  }

  LabelPair labelPairOptionalOutput(StateId state) const override {
    return LabelPair(inputLabelImpl(state), Util::getOrElse(oLabelForState, state, NoSymbol));
  }

  void setFsmGraphOneLexical(bool fsm, bool graph, bool one) override {
    this->setUncomputedPropertiesAt(kGraph, graph);
    this->setUncomputedPropertiesAt(kFsm, fsm);
    this->setUncomputedPropertiesAt(kOneLexical, one);
    fsmChecked = true;
  }

 protected:
  LabelPair labelPairImpl(StateId state) const {
    Sym const in = inputLabelImpl(state);
    return LabelPair(in, outputLabelOrElse(state, in));
  }

  Properties uncomputedProperties() const override { return this->properties_; }

 public:
  LabelPair labelPair(StateId state) const override { return labelPairImpl(state); }

  /**
     virtual version of outputLabelFollowsInput.
  */
  bool emptyOutputLabels() const override { return oLabelForState.empty(); }

  virtual void deleteState(StateId s) override {
    if (storesOutArcs()) {
      assert(!storesInArcs());
      deleteOutArcsImpl(s);
    } else
      deleteInArcsImpl(s);
    if (s == numStates_-1) numStates_ = s;
  }

  enum { kAdjLinearSearchLimit = 16 };

  void deleteInArcsImpl(StateId state) override {
    SDL_TRACE(PhraseBased.Hypergraph.deleteInArcsImpl, gseq << " clearing state " << state
                                                            << " #inarcs: " << numInArcs(state));
    if (state >= inArcsPerState_.size()) return;
    ArcsContainer& arcs = inArcsPerState_[state];
    if (storesOutArcsImpl()) {
      if (this->properties_ & kStoreFirstTailOutArcs) {
        Util::Once deleted;
        interested_.reserve(numStates_);
        for (ArcIter i = arcs.begin(), e = arcs.end(); i != e; ++i) {
          Arc* a = (Arc*)*i;
          StateIdContainer const& tails = a->tails();
          if (!tails.empty()) {
            StateId t = tails[0];
            if (t < outArcsPerState_.size()) {
              ArcsContainer& adj = outArcsPerState_[t];
              if (adj.size() <= kAdjLinearSearchLimit)
                removeDeletedArc(a, adj);
              else {
                interested_.add(t);
                deleted.insert((intptr_t)a);
              }
            }
          }
          delete a;
        }
        removeDeletedArcsFromInterested(deleted, outArcsPerState_);
        interested_.clear();
      } else {
        Util::Once deleted;
        for (ArcIter i = arcs.begin(), e = arcs.end(); i != e; ++i) {
          Arc* a = (Arc*)*i;
          deleted.insert((intptr_t)a);
          delete a;
        }
        removeDeletedArcs(deleted, outArcsPerState_);
      }
    } else
      for (ArcIter i = arcs.begin(), e = arcs.end(); i != e; ++i) delete (Arc*)*i;
    arcs.clear();
  }

  void deleteOutArcsExceptImpl(StateId state, Arc* keepArc) override {
    ArcsContainer& arcs = outArcsPerState_[state];
    ArcsContainer::iterator i = std::find(arcs.begin(), arcs.end(), keepArc);
    if (i != arcs.end()) {
      arcs.erase(i);
    }
    deleteOutArcsImpl(state);
  }

  void deleteOutArcsImpl(StateId state) override {
    if (state >= outArcsPerState_.size()) return;
    ArcsContainer& arcs = outArcsPerState_[state];
    if (storesInArcs()) {
      Util::Once deleted;
      interested_.reserve(numStates_);
      for (ArcIter i = arcs.begin(), e = arcs.end(); i != e; ++i) {
        Arc* a = (Arc*)*i;
        StateId t = a->head();
        if (t < inArcsPerState_.size()) {
          ArcsContainer& adj = inArcsPerState_[t];
          if (adj.size() <= kAdjLinearSearchLimit)
            removeDeletedArc(a, adj);
          else {
            interested_.add(t);
            deleted.insert((intptr_t)a);
          }
        }
        delete a;
      }
      removeDeletedArcsFromInterested(deleted, inArcsPerState_);
      interested_.clear();
    } else
      for (ArcIter i = arcs.begin(), e = arcs.end(); i != e; ++i) delete (Arc*)*i;
    arcs.clear();
  }

  // for each Arc *arc, if keep(arc), then remap arc through x and place it in the in/out arcs index. similar
  // to HypergraphCopy.hpp but in-place. note: if x.frozen, then keep is implicitly heads+tails-in-x(arc) AND
  // keep(arc)
  // axiomsFirst does nothing at the moment - not sure I want it to, either; SortStates uses a
  // non-state-adding mapping for inplace now
  std::size_t restrict(StateIdTranslation& x, ArcFilter const& keep) override {
    ArcFilter keepa = keep ? keep : Arc::filterTrue();
    if (x.identity()) return restrict(keep);
    bool adding = x.stateAdding();
    // TODO: special case !x.frozen - faster than checking when translating
    AllArcs arcs;
    this->fetchArcs(arcs);
    clearArcsPer(this->numStates_);
    this->numStates_ = 0;  // this is necessary! x.relabelArc(*this, *a) actually calls addState
    LabelForState in, out;
    this->iLabelForState.swap(in);
    this->oLabelForState.swap(out);

    /// kCanonicalLex for lexical labels (adding states)
    lstate.clear();  // clear while saving arcs/labels

    std::size_t ndel = 0;
    for (Arc* a : arcs) {
      if (keepa(a) && x.relabelArc(*a)) {
        if (adding)
          addArc(a);
        else
          addArcResize(a);  // resize adjs later
      } else {
        ++ndel;
        delete a;
      }
    }
    updateState(this->start_, x);
    updateState(this->final_, x);
    //    x.freeze(); // add no more states
    transferLabels(x, in, this->iLabelForState);
    transferLabels(x, out, this->oLabelForState);
    notifyLabelImpl();

    if (!x.stateAdding()) {
      setNumStates();  // makes this a valid hg
      compressNumStates();
    }
    finishDeleted(ndel);

    return ndel;
  }

  typedef ArcsContainer::iterator ArcsIter;

  // as above but leaves stateids intact. returns # deleted.
  std::size_t restrict(ArcFilter const& keep) override {
    std::size_t ndel = 0;
    if (!keep) return ndel;
    if (this->properties_ & kStoreInArcs) {
      outArcsPerState_.clear();
      StateId ns = (StateId)inArcsPerState_.size();
      for (StateId s = 0; s < ns; ++s) {
        ArcsContainer& ias = inArcsPerState_[s];
        ArcsIter o = ias.begin();
        ArcsIter i = o, e = ias.end();
        for (; i != e; ++i) {
          Arc* a = (Arc*)*i;
          if (keep(a)) {
            *o = a;
            ++o;
          } else {
            ++ndel;
            delete a;
          }
        }
        ias.erase(o, ias.end());
      }
      rebuildOutArcs();
    } else {
      bool addFirst = this->properties_ & kStoreFirstTailOutArcs;
      AllArcs arcs;
      this->fetchArcs(arcs);
      StateId ns = (StateId)outArcsPerState_.size();
      Util::reinit(outArcsPerState_, ns);
      for (Arc* a : arcs) {
        if (keep(a)) {
          if (addFirst)
            addArcFirstTailOut(a);
          else
            addArcOut(a);
        } else
          ++ndel;
      }
    }
    finishDeleted(ndel);
    return ndel;
  }

  TailId firstLexicalTail(StateIdPtr tails, TailId n) const override {
    for (TailId i = 0; i < n; ++i) {
      if (MutableHypergraphLabels::hasLexicalLabelImpl(tails[i])) return i;
    }
    return (TailId)-1;
  }

  /// \return NoSymbol if none
  Sym firstLexicalInput(ArcBase const* a) const override {
    StateIdContainer const& tails = a->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      Sym s = inputLabelImpl(*i);
      if (s.isLexical()) return s;
    }
    return NoSymbol;
  }

  Sym firstLexicalOutput(ArcBase const* a) const override {
    StateIdContainer const& tails = a->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      Sym s = outputLabelImpl(*i);
      if (s.isLexical()) return s;
    }
    return NoSymbol;
  }

  LabelPair firstLexicalLabelPair(ArcBase const* a) const override {
    StateIdContainer const& tails = a->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      StateId const t = *i;
      LabelPair const p = labelPairImpl(*i);
      if (p.first.isLexical() || p.second.isLexical()) return p;
    }
    return kNullLabelPair;
  }

  LabelPair firstLexicalLabelPairOrEps(ArcBase const* a) const override {
    StateIdContainer const& tails = a->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      StateId const t = *i;
      LabelPair const p = labelPairImpl(*i);
      if (p.first.isLexical() || p.second.isLexical()) return p;
    }
    return getEpsilonLabelPair();
  }

  LabelPair firstLexicalInputLabelPair(ArcBase const* a) const override {
    StateIdContainer const& tails = a->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      StateId const t = *i;
      LabelPair const p = labelPairImpl(*i);
      if (input(p).isLexical()) return p;
    }
    return kNullLabelPair;
  }

  LabelPair firstLexicalOutputLabelPair(ArcBase const* a) const override {
    StateIdContainer const& tails = a->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      LabelPair const p = labelPairImpl(*i);
      if (output(p).isLexical()) return p;
    }
    return kNullLabelPair;
  }

 private:
  /// remove (already deleted) arc pointers
  static void removeDeletedArcs(Util::PointerSet const& deleted, Adjs& adjs) {
    if (deleted.empty()) return;
    for (typename Adjs::iterator i = adjs.begin(), e = adjs.end(); i != e; ++i)
      removeDeletedArcs(deleted, *i);
  }

  void removeDeletedArcsFromInterested(Util::PointerSet const& deleted, Adjs& adjs) {
    if (deleted.empty()) return;
    for (InterestedStates::const_iterator i = interestedStates_.begin(), e = interestedStates_.end(); i != e;
         ++i) {
      StateId s = *i;
      if (s < adjs.size()) removeDeletedArcs(deleted, adjs[s]);
    }
  }

  static void removeDeletedArcs(Util::PointerSet const& deleted, ArcsContainer& arcs) {
    ArcsContainer::iterator i = arcs.begin(), e = arcs.end(), o, b = i;
    for (; i != e; ++i)
      if (Util::contains(deleted, (intptr_t)*i)) {
        o = i;
        ++i;
        for (; i != e; ++i)
          if (!Util::contains(deleted, (intptr_t)*i)) {
            *o = *i;
            ++o;
          }
        arcs.resize(o - b);
        break;
      }
  }

  static void removeDeletedArc(Arc* a, ArcsContainer& arcs) {
    for (ArcsContainer::iterator i = arcs.begin(), e = arcs.end(); i != e; ++i)
      if (*i == a) {
        arcs.erase(i);
        break;
      }
  }

  mutable bool fsmChecked;  // deferring checking of arcs (not as they're added for two reasons: most
  // importantly, in case you add arcs before setting labels. secondly, it's faster
  // this way especially if you never ask. properties will have to compute it, of
  // course

  bool isFsmCheck() const override {
    computeFsmGraphProperty();
    return this->properties_ & kFsm;
  }
  bool isGraphCheck(bool& fsm, bool& oneLexical) const override {
    computeFsmGraphProperty();
    fsm = this->properties_ & kFsm;
    oneLexical = this->properties_ & kOneLexical;
    return this->properties_ & kGraph;
  }

  virtual bool checkGraph() override {
    fsmChecked = false;
    computeFsmGraphProperty();
    return this->properties_ & kGraph;
  }

  void computeFsmGraphProperty() const {
    if (fsmChecked) return;
    this->properties_ &= ~(kFsmProperties);  // need to do this up front because isFsmCheck itself may check
    // properties via prunedEmpty
    fsmChecked = true;
    bool f, one;
    bool g = IHypergraph<Arc>::isGraphCheck(f, one);
    if (f) this->properties_ |= kFsm;
    if (g) this->properties_ |= kGraph;
    if (one) this->properties_ |= kOneLexical;
  }

  void notifyArcsModified() override {
    notifyArcImpl();
    this->properties_ &= ~kSortedOutArcs;
    this->properties_ &= ~kAcyclic;
    assert(checkValid());
  }

  void finishDeleted(std::size_t ndel) {
    if (ndel) {
      this->properties_ &= ~kSortedOutArcs;
      if (!this->isFsm()) fsmChecked = false;  // we may have deleted all the non-fsm arcs
    }
  }

  void setNumStates(StateId N) {
    if (this->properties_ & kStoreInArcs) inArcsPerState_.resize(N);
    if (storesOutArcsImpl()) outArcsPerState_.resize(N);
    numStates_ = N;
  }

  void compressNumStates() {
    StateId m = maxState(*this);  // maxState calls size.
    setNumStates((m == kNoState) ? 0 : m + 1);
  }

  // makes this a valid hg for in-place non-adding transform, where arcs were added before states were
  void setNumStates() {
    StateId m = 0;
    m = std::max(m, (StateId)inArcsPerState_.size());
    m = std::max(m, (StateId)outArcsPerState_.size());
    m = std::max(m, (StateId)iLabelForState.size());
    m = std::max(m, (StateId)oLabelForState.size());
    StateId n = maxState(this->start_, this->final_);
    if (n != kNoState) m = std::max(m, n + 1);
    setNumStates(m);
    numStates_ = m;
  }

  // TODO: perhaps allow lazy growing of inArcsPerState_ and outArcsPerState_? or covering lexical states
  // only? would save some space
  void reserve(StateId n) override {
    if (this->properties_ & kStoreInArcs) inArcsPerState_.reserve(n);
    if (this->properties_ & kStoreAnyOutArcs) outArcsPerState_.reserve(n);
  }

  void rebuildArcAdjacencies() override {
    AllArcs arcs;
    fetchArcs(arcs);
    releaseArcs();
    addArcs(arcs);
  }


  void rebuildOutArcs() {
    if (this->properties_ & kStoreInArcs) {
      if (this->properties_ & kStoreFirstTailOutArcs) {
        this->properties_ &= ~(kStoreFirstTailOutArcs | kStoreOutArcs);
        forceFirstTailOutArcs();
      } else if (this->properties_ & kStoreOutArcs) {
        this->properties_ &= ~kStoreOutArcs;
        forceOutArcs();
      }
    }
  }
  void finishAddArcResize(StateId N) {
    numStates_ = N;
    finishArcsPer(inArcsPerState_, N);
    finishArcsPer(outArcsPerState_, N);
  }
  static inline void finishArcsPer(Adjs& adj, StateId N) { adj.resize(N); }

  void addArcResize(Arc* arc) {
    if (this->properties_ & kStoreInArcs) Util::atExpand(inArcsPerState_, arc->head()).push_back(arc);
    StateIdContainer const& tails = arc->tails();
    if (this->properties_ & kStoreFirstTailOutArcs) {
      if (!tails.empty()) Util::atExpand(outArcsPerState_, tails.front()).push_back(arc);
    } else if (this->properties_ & kStoreOutArcs) {
      for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i)
        Util::atExpand(outArcsPerState_, *i).push_back(arc);
    }
  }

  static void resetArcs(Adjs& adj, StateId N) {
    if (!N) {
      Adjs().swap(adj);
    } else {
      adj.clear();
      adj.reserve(N);
    }
  }

  void clearArcsPer(StateId N) {
    resetArcs(inArcsPerState_, this->properties_ & kStoreInArcs ? N : 0);
    resetArcs(outArcsPerState_, storesOutArcsImpl() ? N : 0);
  }

  void setLabelPairImpl(StateId state, LabelPair io) {
    this->properties_ &= ~kCanonicalLex;
    setLabel(iLabelForState, state, input(io));
    setLabel(oLabelForState, state, output(io));
    if (io.second != NoSymbol) this->properties_ |= kHasOutputLabels;
  }

 public:
  /** an output symbol of NoSymbol means the output changes when you
      setInputLabel later; an explicit same-symbol will fix the output label
      against that eventuality. */
  void setLabelPair(StateId state, LabelPair io) override {
    setLabelPairImpl(state, io);
    notifyLabelImpl();
  }

 protected:
  void init(Properties props) {
    this->typename_ = "sdl::MutableHypergraph";
    this->properties_ = props | kFsmProperties;
    clearStates();
    if (!storesArcs())
      SDL_THROW_LOG(Hypergraph, ConfigException,
                    "Hypergraph can't store arcs - user properties didn't specify in-arcs, out-arcs, or "
                    "first-tail-out-arcs");
  }

  void clearStates() {
    numStates_ = 0;
    this->start_ = Hypergraph::kNoState;
    this->final_ = Hypergraph::kNoState;
    fsmChecked = true;
  }

 public:
  typedef MutableHypergraph Self;
  // improved: default props get set no matter what you pass in.
  MutableHypergraph(Properties props = kDefaultStoreArcsPerState) { init(props); }

  MutableHypergraph<Arc>* clone() const override {
    MutableHypergraph* pHg = new MutableHypergraph<A>(this->properties_);
    pHg->setVocabulary(pVocab_);
    copyHypergraph(*this, pHg);
    return pHg;
  }

  ~MutableHypergraph() {
    assert(checkValid());
    this->deleteArcs();  // can't go in parent classes
  }

  StateId ensureStart() override {
    if (this->start_ == kNoState) {
      this->start_ = numStates_;
      addStateImpl();
    }
    return this->start_;
  }

  StateId size() const override { return numStates_; }

  void clearImpl(Properties prop) override {
    this->deleteArcs();
    clearArcsPer(numStates_);
    clearLabelImpl(prop);
    clearStates();
  }

  ArcId numInArcs(StateId from) const override {
    assert(storesInArcs());
    return from < inArcsPerState_.size() ? inArcsPerState_[from].size() : 0;
  }
  ArcId numOutArcs(StateId from) const override {
    assert(storesOutArcsImpl());
    return from < outArcsPerState_.size() ? outArcsPerState_[from].size() : 0;
  }

  Arc* inArc(StateId from, ArcId aid) const override {
    assert(storesInArcs());
    assert(from < inArcsPerState_.size());
    assert(aid < inArcsPerState_[from].size());
    assert(from == inArcsPerState_[from][aid]->head_);
    return (Arc*)inArcsPerState_[from][aid];
  }

  Arc* outArc(StateId from, ArcId aid) const override {
    assert(storesOutArcsImpl());
    assert(from < outArcsPerState_.size());
    assert(aid < outArcsPerState_[from].size());
    assert(!(this->properties_ & kStoreFirstTailOutArcs) || from == outArcsPerState_[from][aid]->tails_[0]);
    return (Arc*)outArcsPerState_[from][aid];
  }

  void replaceFirstTailOutArcsMove(StateId from, ArcsContainer&& with) override {
    assert(this->properties_ & kStoreFirstTailOutArcs);
    assert(from < outArcsPerState_.size());
    outArcsPerState_[from] = with;
  }

  bool storesInArcs() const { return this->properties_ & kStoreInArcs; }
  bool storesOutArcs() const { return storesOutArcsImpl(); }

  typedef typename IHypergraph<Arc>::AdjsPtr AdjsPtr;

  /**
     may be invalidated by IMutableHypergraph addArc operations.
  */
  AdjsPtr inArcs() const override {
    if (storesInArcs()) {
      return AdjsPtr(inArcsPerState_);
    } else {
      Adjs* p = new Adjs;
      AdjsPtr r(p);
      IHypergraph<Arc>::inArcs(*p);
      return r;
    }
  }

  /**
     may be invalidated by IMutableHypergraph addArc operations.
  */
  AdjsPtr getFirstTailOutArcs(bool allowNonFirstTail) const override {
    if (this->properties_ & kStoreFirstTailOutArcs || (allowNonFirstTail && this->properties_ & kStoreOutArcs))
      return AdjsPtr(outArcsPerState_);
    else {
      Adjs* p = new Adjs;
      AdjsPtr r(p);
      HypergraphBase::getFirstTailOutArcs(*p, allowNonFirstTail);
      return r;
    }
  }


  InterestedStates interestedStates_;
  Util::BitSet inInterestedStates_;
  void clearInterested() {
    for (InterestedStates::const_iterator i = interestedStates_.begin(), e = interestedStates_.end(); i != e;
         ++i) {
      inInterestedStates_.reset(*i);
    }
    interestedStates_.clear();
  }
  void resizeInterested(StateId n) {
    assertEmptyInterested();
    inInterestedStates_.resize(n);
  }
  void interested(StateId s) {
    if (!inInterestedStates_.test_set(s)) interestedStates_.push_back(s);
  }
  void assertEmptyInterested() {
    assert(interestedStates_.empty());
    assert(inInterestedStates_.count() == 0);
  }

  void prepareAddArcsSize(StateId size) override { resizeArcsForStates(size); }

  void prepareAddArcs() override { resizeArcsForStates(this->sizeForHeads()); }

  bool firstTailOnly() const override {
    return (this->properties_ & kStoreFirstTailOutArcs) && !(this->properties_ & kStoreInArcs);
  }

 private:
  /// don't call if storing both in and out arcs
  static void deleteAdjacentArcs(ArcsContainer& arcs) {
    for (ArcIter i = arcs.begin(), e = arcs.end(); i != e; ++i) delete (Arc*)*i;
    arcs.clear();
  }

  /// no resize needed / removes ability to modify hg while holding maybeOutArcs
  /// ref (see addArc-resizes indices as needed)
  void resizeArcsForStates(StateId size) {
    if (this->properties_ & kStoreInArcs) inArcsPerState_.resize(size);
    if (storesOutArcsImpl()) outArcsPerState_.resize(size);
  }

  void addStateImpl() {
    ++numStates_;
    notifyLabelImpl();
  }

  StateId addStateImpl(Sym inputLabel) {
    StateId state = numStates_;
    setInputLabelImpl(state, inputLabel);
    addStateImpl();
    return state;
  }

  void addStateIdImpl(StateId state) {
    if (state >= numStates_) {
      numStates_ = state + 1;
      // resizeArcsForStates();
    }
  }

 public:
  void addStateId(StateId state, LabelPair l) override {
    addStateIdImpl(state);
    setLabelPairImpl(state, l);
  }

  void removeLastState() override {
    --numStates_;
    // resizeArcsForStates();
  }

  StateId addState() override {
    StateId state = numStates_;
    addStateImpl();
    return state;
  }

  /// If kCanonicalLex, returns known StateId for Sym, if available.
  StateId addState(Sym input) override {
    if (!(this->properties_ & kCanonicalLex) || !input.isTerminal()) return addStateImpl(input);
    StateId* s;
    if (Util::update(lstate, LabelPair(input, NoSymbol), s))
      return (*s = addStateImpl(input));
    else
      return *s;
  }

  StateId addStateNoCanonical(Sym input, Sym output) {
    StateId state = numStates_;
    addStateImpl();
    setNewStateLabels(state, input, output, this->properties_);
    return state;
  }

  StateId addState(LabelPair const& io) { return addState(input(io), output(io)); }

  /// If kCanonicalLex is set, returns known StateId for Sym, if
  /// available.  Call only for axioms (terminal labels).
  StateId addState(Sym input, Sym output) override {
    assert(output == NoSymbol || input == output || input.isTerminal() && output.isTerminal());
    if (!(this->properties_ & kCanonicalLex)) return addStateNoCanonical(input, output);
    StateId* s;
    if (Util::update(lstate, LabelPair(input, output), s))
      return (*s = addStateNoCanonical(input, output));
    else
      return *s;
  }

  StateId nextStateId() const override { return numStates_; }

  // addStateId may be existing state. //FIXME: is this wrong? should we assert that the state is new?
  StateId addStateId(StateId state) override {
    addStateIdImpl(state);
    return state;
  }

  StateId addStateId(StateId state, Sym inputLabel, Sym outputLabel) override {
    addStateIdImpl(state);
    setNewStateLabels(state, inputLabel, outputLabel, this->properties_);
    return state;
  }

  bool hasLexicalInputLabel(StateId state) const override {
    return MutableHypergraphLabels::hasLexicalInputLabelImpl(state);
  }

  bool hasLexicalLabel(StateId state) const override {
    return MutableHypergraphLabels::hasLexicalLabelImpl(state);
  }

  bool hasTerminalLabel(StateId state) const override { return inputLabelImpl(state).isTerminal(); }

  bool hasTerminalLabelImpl(StateId state) const { return inputLabelImpl(state).isTerminal(); }

  bool isFsmArc(ArcBase const& a) const override {
    StateIdContainer const& tails = a.tails();
    return tails.size() == 2 && !hasTerminalLabelImpl(tails[0]) && hasTerminalLabelImpl(tails[1]);
  }

  Sym inputLabel(StateId state) const override { return inputLabelImpl(state); }

  bool isGraphArc(ArcBase const& a, bool& fsm, bool& oneLexical) const override {
    return detail::isGraphArcImpl(*this, a, fsm, oneLexical);
  }

  /**
     Returns output label; by convention, whenever an output
     label is not given it is identical to the input label.
  */
  Sym outputLabel(StateId state) const override { return outputLabelImpl(state); }

  // unlike IHypergraph's impl, this actually shrinks the vec to the least necessary size. it's also faster.
  // this is conceptually const only - not bothering to add const and non-const versions to IHypergraph
  StateId maxLabeledState(LabelType labelType) const override {
    if (labelType == kInput_Output_Label)
      return std::max(maxLabeledState(kInput), maxLabeledState(kOutput));
    else {
      LabelForState& syms = const_cast<LabelForState&>(labelType == kOutput ? oLabelForState : iLabelForState);
      StateId m = maxLabeledState(syms);
      StateId s = (m == kNoState) ? 0 : m + 1;
      assert(s <= syms.size());
      syms.resize(s);
      return m;
    }
  }

  // in case we ever want higher performance, add this to IHypergraph interface too (now is free fn in
  // SortStates)
  /*
    StateId countLabeledStates() const
    {
    return countLabeledStates(iLabelForState);
    }

    StateId countLabeledStates(LabelForState const& syms) const
    {
    StateId nLabeled = 0;
    StateId nst = std::max(numStates_, syms.size()); // could std::count_if not_equal(NoSymbol)
    for (StateId st = 0; st<nst; ++st)
    if (syms[st]!=NoSymbol)
    ++nLabeled;
    return nLabeled;
    }*/
  StateId maxLabeledState(LabelForState const& syms) const {
    StateId s = std::min(numStates_, (StateId)syms.size());
    while (s) {
      --s;
      if (syms[s] != NoSymbol) return s;
    }
    return kNoState;
  }

  /** on a new state s, or one whose output label was never set, or was set
      explicitly to NoSymbol, this also effectively sets the output label.

      setLabelPair(s, LabelPair(1, 1))
      setInputLabel(s, 2)
      The answer is labelPair()== (2, 1).

      If, in that example, we currently don’t store the output label 1 explicitly then
      we have a bug. The correct behavior of setLabelPair(s, LabelPair(1, 1)) is to
      store 1 as input label and 1 (not NoSymbol) as output label.

      And if the output label is never explicitly set, we have this:

      setInputLabel(s, 1)
      setInputLabel(s, 2)
      labelPair()== (2, 2)

      if you intend to modify the input but leave the output unmodified, first
      outputLabel, then modify the input label, then setOutputLabel to
      restore it - you can check outputLabelFollowsInput(state); only if that's
      true do you need to save/restore the output

      TODO: for clarity, we may want to have a new method setInputOrFsaLabel
      that does what the current setInputLabel does, and a new setInputLabel
      that only changes the result of inputLabel and not that of
      outputLabel (regardless of whether outputLabelFollowsInput, which would
      only be relevant for the new setInputOrFsaLabel)
  */
  void setInputLabel(StateId state, Sym label) override {
    this->properties_ &= ~kCanonicalLex;
    setInputLabelImpl(state, label);
    notifyLabelImpl();
  }

  void setOutputLabel(StateId state, Sym label) override {
    this->properties_ &= ~kCanonicalLex;
    setOutputLabelImpl(state, label, this->properties_);
    notifyLabelImpl();
  }

  void addProperties(Properties p) { this->properties_ |= p; }
  void clearProperties(Properties p) { this->properties_ &= ~p; }

  void removeOutArcs() override {
    Util::clearVector(outArcsPerState_);
    this->clearProperties(kStoreOutArcs);
    this->clearProperties(kStoreFirstTailOutArcs);
  }

  void removeInArcs() override {
    Util::clearVector(inArcsPerState_);
    this->clearProperties(kStoreInArcs);
  }

  void releaseArcs() override {
    Util::clearVector(inArcsPerState_);
    assert(inArcsPerState_.empty());
    Util::clearVector(outArcsPerState_);
    assert(outArcsPerState_.empty());
  }

  void removeOutputLabels() override { Util::clearVector(oLabelForState); }

  bool checkValid() const override {
    if (!HypergraphBase::checkValid()) return false;
    if (this->prunedEmpty()) return true;
    if (this->isFsm() && !this->isFsmCheck()) {
      SDL_WARN(Hypergraph.checkValid, "ERROR: set as Fsm but checking found a non-fsm arc!\n");
      return false;
    }
    return true;
  }

 private:
  Util::Interested<StateId> interested_;

  void addArcIn(Arc* arc) {
    StateId const headId = arc->head();
    Util::atExpand(inArcsPerState_, headId).push_back(arc);
  }

  void addArcFirstTailOut(Arc* arc) {
    this->properties_ &= ~kSortedOutArcs;  // TODO: check if inserted in sort-preserving (ascending) order
    StateIdContainer const& tails = arc->tails();
    if (tails.empty()) return;
    StateId const tailState = tails.front();
    Util::atExpand(outArcsPerState_, tailState).push_back(arc);
  }

  void addArcOut(Arc* arc) {
    this->properties_ &= ~kSortedOutArcs;  // TODO: check if inserted in sort-preserving (ascending) order
    StateIdContainer const& tails = arc->tails();
    StateId N = (StateId)outArcsPerState_.size();
    for (StateId tailState : tails) {
      Util::atExpand(outArcsPerState_, tailState).push_back(arc);
    }
  }

  struct AddIn {
    Self& hg;
    AddIn(Self& hg) : hg(hg) {}
    void operator()(Arc* arc) const { hg.addArcIn(arc); }
  };

  struct AddFirstTailOut {
    enum { addProperties = kStoreFirstTailOutArcs, removeProperties = kStoreOutArcs };
    Self& hg;
    AddFirstTailOut(Self& hg) : hg(hg) {}
    void operator()(Arc* arc) const { hg.addArcFirstTailOut(arc); }
  };

  struct AddOut {
    enum { addProperties = kStoreOutArcs, removeProperties = kStoreFirstTailOutArcs };
    Self& hg;
    AddOut(Self& hg) : hg(hg) {}
    void operator()(Arc* arc) const { hg.addArcOut(arc); }
  };

  template <class Adder>
  void buildOut() {
    assert(this->storesArcs());
    Adder adder(*this);
    if (this->properties_ & kStoreInArcs) {
      Util::reinit(outArcsPerState_, inArcsPerState_.size());
      this->forArcsImpl(adder);
    } else {
      AllArcs arcs;
      this->fetchArcs(arcs);
      Util::reinit(outArcsPerState_, numStates_);
      for (Arc* arc : arcs) adder(arc);
    }
    this->properties_ |= Adder::addProperties;
    this->properties_ &= ~Adder::removeProperties;
  }

  bool storesOutArcsImpl() const { return this->properties_ & kStoreAnyOutArcs; }

  using typename Base::AllArcs;

  void addArcs(AllArcs const& arcs) override {
    for (Arc* arc : arcs) addArc(arc);
  }

  void fetchArcs(AllArcs& arcs) const {
    arcs.reserve(this->estimatedNumEdges());
    forArcsImpl([&arcs](Arc* arc) { arcs.push_back(arc); });
  }

  /// mere optimization over IHypergraph::visitArcs (also, you can trust it uses
  /// inarcs iff possible - see buildOut)
  template <class Visitor>
  void forArcsImpl(Visitor const& v) const {
    // using stateid s so you can add states in visitor
    if (this->properties_ & kStoreInArcs) {
      for (StateId s = 0, n = (StateId)inArcsPerState_.size(); s < n; ++s)
        for (ArcBase* a : inArcsPerState_[s]) v((Arc*)a);
    } else if (this->properties_ & kStoreFirstTailOutArcs) {
      for (StateId s = 0, n = (StateId)outArcsPerState_.size(); s < n; ++s)
        for (ArcBase* a : outArcsPerState_[s]) v((Arc*)a);
    } else
      // will also use stateid s.
      IHypergraph<Arc>::forArcs(v);
  }

  virtual void visitArcs(ArcVisitor const& v) const override { forArcsImpl(v); }

 public:
  /// the new arc might not be graph/fsm/one-lexical. we defer checking until
  /// someone cares to ask. we don't mark arcs unsorted since you might
  /// intentionally add them in sorted order (call notifyArcsModified if you
  /// want to tell us that the arc sorting might have been lost)
  void notifyArcImpl() { fsmChecked = false; }
  /// changing a label or adding a state might drop sorted-states property (e.g. not-terminal first)
  void notifyLabelImpl() { this->properties_ &= ~kSortedStates; }

  /**
     Depending on the properties, we may store the arc as
     incoming in the head state or as outgoing in the tail states or
     both.
  */

  bool hasLabel(StateId state) const override { return hasLabelImpl(state); }

  bool storesArcs() const { return this->properties_ & kStoreAnyArcs; }

  void addArc(ArcBase* arc) override {
    assert(this->storesArcs());
    notifyArcImpl();
    if (this->properties_ & kStoreInArcs) Util::atExpand(inArcsPerState_, arc->head_).push_back(arc);
    StateIdContainer const& tails = arc->tails_;
    if (this->properties_ & kStoreFirstTailOutArcs) {
      this->properties_ &= ~kSortedOutArcs;
      if (!tails.empty()) Util::atExpand(outArcsPerState_, tails.front()).push_back(arc);
    } else if (this->properties_ & kStoreOutArcs) {
      this->properties_ &= ~kSortedOutArcs;
      for (StateId t : tails) {
        Util::atExpand(outArcsPerState_, t).push_back(arc);
      }
    }
  }

  /// addArc after calling addStateId on head, tails.
  virtual void addArcCreatingStates(Arc* arc) override {
    StateId maxState = arc->head_;
    for (StateId t : arc->tails_) Util::maxEq(maxState, t);
    Util::maxEq(numStates_, maxState + 1);
    addArc(arc);
  }

  void forceFirstTailOutArcs() override {
    if (!(this->properties_ & kStoreFirstTailOutArcs)) {
      this->properties_ |= kStoreFirstTailOutArcs;
      bool hadout = this->properties_ & kStoreOutArcs;
      if (!hadout)
        buildOut<AddFirstTailOut>();
      else {
        this->properties_ &= ~kStoreOutArcs;
        StateId s = 0;
        for (ArcsContainer& arcs : outArcsPerState_) {
          ArcsContainer::iterator in = arcs.begin(), out = in, end = arcs.end();
          for (; in != end; ++in) {
            Arc* a = (Arc*)*in;
            assert(!a->tails_.empty());
            if (a->tails_[0] == s) {
              /// s is the state for [out]arcs so this means first tail arc.
              *out = a;
              ++out;  // keeping a.
            }
          }
          arcs.erase(out, end);
          ++s;
        }
      }
    }
  }

  void forceOutArcs() override {
    if (!(this->properties_ & kStoreOutArcs)) buildOut<AddOut>();
  }

  void forceInArcs() override {
    assert(this->storesArcs());
    if (!(this->properties_ & kStoreInArcs)) {
      StateId ns = (StateId)outArcsPerState_.size();
      Util::reinit(inArcsPerState_, ns);
      // avoids pitfall of first-tail being a repeat
      this->forArcs(AddIn(*this));
      this->properties_ |= kStoreInArcs;
    }
    assert(this->storesArcs());
  }

  void clearOutArcs(StateId state) override {
    if (state < outArcsPerState_.size()) outArcsPerState_[state].clear();
  }

  bool forceCanonicalLex() override {
    this->properties_ |= kCanonicalLex;
    return forceCanonicalLexImpl();
  }

  void clearCanonicalLex() override {
    this->properties_ &= ~kCanonicalLex;
    lstate.clear();
  }

  void clearCanonicalLexCache() override {
    this->properties_ &= ~kCanonicalLex;
    lstate.clear();
  }

  bool ensureCanonicalLex() override {
    if (this->properties_ & kCanonicalLex) return canonicalLexIsCanonical_;
    this->properties_ |= kCanonicalLex;
    return forceCanonicalLexImpl();
  }

  bool rebuildCanonicalLex() override {
    if (this->properties_ & kCanonicalLex)
      return forceCanonicalLexImpl();
    else {
      lstate.clear();
      return false;
    }
  }

  void setProperties(Properties p) override { this->properties_ = p; }

  Properties properties() const override {
    computeFsmGraphProperty();
    return this->properties_;
  }

  void setPropertyBit(Properties bits, bool on = true) override {
    if (on)
      this->properties_ |= bits;
    else
      this->properties_ &= ~bits;
  }

  ArcsContainer const* maybeOutArcs(StateId state) const override {
    assert(this->emptyArcs_.empty());
    assert(storesOutArcsImpl());
    return state < outArcsPerState_.size() ? &outArcsPerState_[state] : &this->emptyArcs_;
  }

  ArcsContainer* maybeOutArcs(StateId state) override {
    assert(storesOutArcsImpl());
    return state < outArcsPerState_.size() ? &outArcsPerState_[state] : 0;
  }

  void setOutArcs(StateId state, ArcsContainer&& arcs) override {
    if (state >= outArcsPerState_.size()) outArcsPerState_.resize(state + 1);
    outArcsPerState_[state] = arcs;
  }

  ArcsContainer const* maybeInArcs(StateId state) const override {
    assert(this->emptyArcs_.empty());
    assert(storesInArcs());
    return state < inArcsPerState_.size() ? &inArcsPerState_[state] : &this->emptyArcs_;
  }

  ArcsContainer* maybeInArcs(StateId state) override {
    assert(storesInArcs());
    return state < inArcsPerState_.size() ? &inArcsPerState_[state] : 0;
  }

  void setVocabulary(IVocabularyPtr const& pVocab) override { pVocab_ = pVocab; }

  IVocabularyPtr getVocabulary() const override { return pVocab_; }

  IVocabulary* vocab() const override { return pVocab_.get(); }


  // The in and out arcs are not in a separate State class because
  // then every state would have such vectors of in and out
  // arcs. Sometimes we do not store in arcs, for example, at all, and
  // will have just one empty vector now.
  AdjacentArcs inArcsPerState_;
  AdjacentArcs outArcsPerState_;

  void printAdj(std::ostream& out, AdjacentArcs const& adj, char const* adjtype) const {
    for (StateId s = 0, N = adj.size(); s < N; ++s) {
      out << "# " << adjtype << s << ":\n";
      for (ArcBase* arc : adj[s]) Hypergraph::print(out, arc, *this);
    }
  }

  void printUnchecked(std::ostream& out) const override {
    if (this->storesOutArcs()) printAdj(out, outArcsPerState_, "=>");
    if (this->storesInArcs()) printAdj(out, inArcsPerState_, "<=");
  }

  void clearLabelImpl(Properties prop) {
    lstate.clear();
    iLabelForState.clear();
    oLabelForState.clear();
    this->properties_ = prop | kFsmProperties;
  }

  void reserveAdditionalStates(StateId n) override {
    StateId n2 = numStates_ + n;
    Properties p = this->properties_;
    if (p | kStoreInArcs) inArcsPerState_.reserve(n2);
    if (p | kStoreAnyOutArcs) outArcsPerState_.reserve(n2);
  }
};

template <class Arc>
void setEmptyHg(shared_ptr<IHypergraph<Arc>>& hg) {
  hg.reset(new MutableHypergraph<Arc>);
  assert(hg->prunedEmpty());
}


}}

#endif
