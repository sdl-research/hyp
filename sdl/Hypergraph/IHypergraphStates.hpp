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

    Hypergraph states+labels (not arcs)
*/

#ifndef IHYPERGRAPHSTATES_GRAEHL_2015_08_09_HPP
#define IHYPERGRAPHSTATES_GRAEHL_2015_08_09_HPP
#pragma once

#include <sdl/Resource.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Sym.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>

namespace sdl { namespace Hypergraph {

using Vocabulary::WhichFstComposeSpecials;


/**
   Parts of IHypergraph that relate to states and don't care about arc type.
*/
struct IHypergraphStates : Resource {

  static char const* staticType() { return "IHypergraph"; }
  char const* dynamicType() const {
    return typename_.c_str();
  }
  std::string typename_;

  std::string const& typeStr() const { return typename_; }

  IHypergraphStates(std::string const& typeString) : typename_(typeString) { setThreadSpecific(); }

  virtual ~IHypergraphStates() {}

  virtual IVocabularyPtr getVocabulary() const = 0;

  virtual IVocabulary* vocab() const { return getVocabulary().get(); }

  /**
     properties() & kOutArcsSortedBestFirst: whether (approximately) best-first
     (lowest weight) outarcs. unlike hasSortedArcs, this is not invalidated by
     adding new arcs
  */

  /**
     \return (properties() & kSortedOutArcs): whether sorted by input label
     (best-first for same input label) outarcs.
  */
  bool hasSortedArcs() const;

  /**
     \return structural and arc-adjacency property bitmask (see Properties.hpp).

     The first call might take O(n) time, but subsequent calls without modifying the hg should be O(1)
  */
  virtual Properties properties() const = 0;

  /// may exclude computed properties (like kFsmProperties)
  virtual Properties hgUncomputedProperties() const { return properties(); }

  /** fast estimate for reserving storage for # of edges (getNumEdges may be linear time in #states) */
  virtual std::size_t estimatedNumEdges() const { return size(); }

  virtual bool isMutable() const;

  /// will throw ConfigException if hg is not mutable and outarcs (or
  /// first-tail) aren't already stored. else you'll have at least first-tail
  /// out-arcs
  virtual void forceFirstTailOutArcs() {
    if (!storesOutArcs())
      SDL_THROW_LOG(Hypergraph.forceFirstTailOutArcs, ConfigException,
                    "hypergraph doesn't have at least first-tail-out-arcs");
  }

  /// so you can use GraphInlineInputLabels
  virtual void forceNotAllOutArcs() {
    if (storesAllOutArcs())
      SDL_THROW_LOG(Hypergraph.forceFirstTailOutArcs, ConfigException,
                    "hypergraph should not store *all* out-arcs");
  }

  // TODO: one or both of start/final state should just be direct data members (for
  // efficiency). use case for virtual isn't on the horizon

  /**
     fsm and graph require start state; for cfg it's optional. terminal labeled
     and start states are all reachable bottom-up in a hg.
  */
  StateId start() const { return start_; }

  /**
     If your hypergraph naturally has more than one final
     state, add an artificial final ("super-final") state that they
     all point to, using epsilon arcs.
  */
  StateId final() const { return final_; }

  /**
     \return N such that you're allowed to use StateId 0 <= s < N
  */
  virtual StateId size() const = 0;

  /**
     return the exact number of nonlexical states. may be O(n). see SortStates
  */
  virtual StateId numNotTerminalStates() const { return countNumNotTerminalStates(); }

  /**
     A Graph in xmt is a left-recursive CFG. the set of all kGraph
     hypergraphs is a superset of the strictly binary kFsm. a
     hypergraph has property kGraph iff all its arcs are graph
     arcs. (kFsm implies kGraph)

     A graph arc has N tails: all but the first have a terminal label
     (special or lexical).

     Note: an arc that has in the first tail a lexical (leaf) state is
     not a graph. graphs must have a defined start and final state or
     else they're considered empty.

     This may run an expensive check O(n), but the next calls w/o
     changing an IMutableHypergraph will be O(1).

     Accepting-empty-sequence (start=final) and empty (final is kNoState or no
     path from start->final) are still graphs.
  */
  bool isGraph() const { return properties() & kGraph; }

  /**
     \return bound M such that only states s < M can be a head or start state (for graphs, this means all
     graph source/target states are <M)

     may return larger than exactSizeForHeads for efficiency.
  */
  virtual StateId sizeForHeads() const { return exactSizeForHeads(); }

  /**
     \return tight bound M such that only states s < M can be a head or start state
  */
  virtual StateId exactSizeForHeads() const { return maxNotTerminalState() + 1; }

  /**
     \return hg is an Fsm - Graph with each arc having exactly 2 tails

     this may run an expensive check O(n), but the next calls w/o changing an IMutableHypergraph will be O(1).
  */
  bool isFsm() const { return properties() & kFsm; }

  /**
     \return at most one lexical tail per state, with no terminal heads

     this may run an expensive check O(n), but the next calls w/o changing an IMutableHypergraph will be O(1).
  */
  bool hasAtMostOneLexicalTail() const { return properties() & kOneLexical; }

  /**
     \return isFsm() || isGraph() && hasAtMostOneLexicalTail() but faster.
  */
  bool isFsmLike() const {
    Properties const p = properties();
    return (p & kFsm) || ((p & kGraph) && (p & kOneLexical));
  }

  /// returns an exact count of not-temrinal-labeled states
  StateId countNumNotTerminalStates() const {
    StateId numNotTerminal = 0;
    for (StateId stateId = 0, N = size(); stateId < N; ++stateId)
      if (!hasTerminalLabel(stateId)) ++numNotTerminal;
    return numNotTerminal;
  }

  virtual StateId numNotTerminalStatesSorted() const {
    SDL_THROW_LOG(Hypergraph, ProgrammerMistakeException,
                  "numNotTerminalStatesSorted is only implemented for MutableHypergraph at present, since "
                  "you can only sort a MutableHypergraph.");
  }


  StateIdRange getStateIds() const { return StateIdRange(StateIdIterator(0), StateIdIterator(size())); }

  virtual StateIdInterval possiblyInputTerminalLabeledStates() const {
    return StateIdInterval(0, this->size());
  }

  virtual WhichFstComposeSpecials whichInputFstComposeSpecials() const {
    WhichFstComposeSpecials r;
    for (StateIdInterval states(possiblyInputTerminalLabeledStates()); states.first < states.second;
         ++states.first)
      r.check(inputLabel(states.first));
    return r;
  }

  // TODO: allow setting of per-state axiom weights, or just enforce 0-tails arcs for axioms.
  /**
     \return whether state is start or has a lexical label.
  */
  virtual bool isAxiom(StateId state) const;

  bool isHeadState(StateId state) const { return !isAxiom(state); }

  /// constant time - because final state is set to kNoState when we detect that there are no
  // derivations. may return false even if there are no derivations.
  bool prunedEmpty() const;

  virtual StateId maxLabeledState(LabelType labelType) const {
    if (labelType == kInput_Output_Label) return maxLabeledState();
    StateId numStates = size();
    if (numStates == 0) return kNoState;
    for (StateId state = numStates; state;) {
      --state;
      if (label(state, labelType) != NoSymbol) return state;
    }
    return kNoState;
  }

  /**
     Returns the highest ID of the labeled states (may be
     lexical or other labels)
  */
  StateId maxLabeledState() const {
    return hasOutputLabels() ? maxState(maxLabeledState(kInput), maxLabeledState(kOutput))
                             : maxLabeledState(kInput);
  }

  virtual StateId sizeForLabels() const { return maxLabeledState() + 1; }

  bool haveState(StateId s) const { return s < size(); }

  /**
     Returns the highest ID of the non-lexical states. This is
     often needed in finite-state machines, to construct state
     transition matrices (which should consist of only the non-lexical
     states).
     *

     \see SortStates
     *
     O(1) for sorted MutableHypergraph
  */
  virtual StateId maxNotTerminalState() const { return maxNotTerminalStateImpl(); }

  // may return NoSymbol
  virtual Sym inputLabel(StateId state) const = 0;

  /* returns output label if hasOutputLabels() && output label
     wasn't set to NoSymbol, else returns input label
  */
  virtual Sym outputLabel(StateId state) const = 0;

  /** \return true iff outputLabel is NoSymbol, and so tracks the input
      label. This can save redundantly transforming the output
      label. (outputLabel returns inputLabel in that case). */
  virtual bool outputLabelFollowsInput(StateId state) const = 0;

  /** \return true if for all state, outputLabelFollowsInput(state). O(1), may return false if not sure */
  virtual bool outputLabelFollowsInput() const { return !hasOutputLabels(); }

  /**
     \return first tail's lexical input label symbol - NoSymbol if none. - used
     by Tokens.hpp - so at most one lexical tail per arc can
     have (specific) alignment information, because the FeatureWeight is common
     to the arc (we don't have axiom leaf state weights)
  */
  template <class Arc>
  Sym firstLexicalInputTpl(Arc const* a) const {
    StateIdContainer const& tails = a->tails_;
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      Sym const sym = inputLabel(*i);
      if (sym.isLexical()) return sym;
    }
    return NoSymbol;
  }

  template <class Arc>
  Sym firstRuleSrcInputTpl(Arc const* a) const {
    StateIdContainer const& tails = a->tails_;
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      Sym const sym = inputLabel(*i);
      if (Vocabulary::isRuleSrcSymbol(sym)) return sym;
    }
    return NoSymbol;
  }

  template <class Arc>
  Sym firstLexicalOutputTpl(Arc const* a) const {
    StateIdContainer const& tails = a->tails_;
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      Sym const sym = outputLabel(*i);
      if (sym.isLexical()) return sym;
    }
    return NoSymbol;
  }

  template <class Arc>
  LabelPair firstLexicalLabelPairTpl(Arc const* a) const {
    StateIdContainer const& tails = a->tails_;
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      LabelPair const pair = labelPair(*i);
      if (pair.first.isLexical() || pair.second.isLexical()) return pair;
    }
    return kNullLabelPair;
  }

  template <class Arc>
  LabelPair firstLexicalInputLabelPairTpl(Arc const* a) const {
    StateIdContainer const& tails = a->tails_;
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      LabelPair const pair = labelPair(*i);
      if (pair.first.isLexical()) return pair;
    }
    return kNullLabelPair;
  }

  template <class Arc>
  LabelPair firstLexicalOutputLabelPairTpl(Arc const* a) const {
    StateIdContainer const& tails = a->tails_;
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      LabelPair const pair = labelPair(*i);
      if (output(pair).isLexical()) return pair;
    }
    return kNullLabelPair;
  }

  bool hasOutputLabels() const { return properties() & kHasOutputLabels; }

  // first f(ilabel) returing true aborts visiting early
  template <class F>
  bool anyInputLabel(F const& f, bool alsoNolabel = false) const {
    for (StateId state = 0, e = size(); state < e; ++state) {
      Sym i = inputLabel(state);
      if (alsoNolabel || i != NoSymbol)
        if (f(i)) return true;
    }
    return false;
  }

  // first f(stateid, ilabel, olabel) returning true aborts visiting early
  template <class F>
  bool anyLabel(F const& f, bool alsoNolabel = false) const {
    for (StateId state = 0, e = size(); state < e; ++state) {
      Sym i = inputLabel(state);
      Sym o = outputLabel(state);
      if (alsoNolabel || i != NoSymbol)
        if (f(state, i, o)) return true;
    }
    return false;
  }

  Sym label(StateId state, LabelType labelType) const {
    return labelType == kOutput ? outputLabel(state) : inputLabel(state);
  }

  /// Note: you can ask for a state that hasn't been created yet -
  /// you'll get in = out = NoSymbol. alternatively, check
  /// size() first. output is copied from input if needed.
  virtual LabelPair labelPair(StateId state) const;

  /// \return albelPair(state) but output will be NoSymbol if outputLabelFollowsInput(state)
  virtual LabelPair labelPairOptionalOutput(StateId state) const {
    return outputLabelFollowsInput() ? LabelPair(inputLabel(state), NoSymbol) : labelPair(state);
  }

  bool canonicalLex() const { return properties() & kCanonicalLex; }

  /**
     non-special terminal (i.e. lexical) label on input or output
  */
  virtual bool hasLexicalLabel(StateId state) const {
    LabelPair pair(labelPair(state));
    return pair.first.isLexical() || pair.second.isLexical();
  }

  virtual bool hasLexicalInputLabel(StateId state) const { return inputLabel(state).isLexical(); }

  /**
     \return whether state has a special or regular terminal (e.g. lexical or epsilon) input label.
  */
  virtual bool hasTerminalLabel(StateId state) const { return inputLabel(state).isTerminal(); }

  template <class V>  // note - by ref, and pass *this as first arg
  void forAxioms(V& v) const {
    StateId starts = start();
    for (StateId s = 0, e = size(); s < e; ++s)
      if (s == starts || hasTerminalLabel(s)) v(*this, s);
  }

  template <class V>  // note - by ref
  void forAxiomIds(V& v) const {
    for (StateId state = 0, e = size(); state != e; ++state)
      if (isAxiom(state)) v(state);
  }

  template <class V>
  void forAxiomIds(V const& v) const {
    for (StateId state = 0, e = size(); state != e; ++state)
      if (isAxiom(state)) v(state);
  }

  /// meaning there's an input label - it's not allowed to set input label to NoSymbol while having an
  /// outputLabel.
  virtual bool hasLabel(StateId id) const;

  template <class V>
  void forNotTerminalStates(V const& v) const {
    forStates(v, false, true, true);
  }

  template <class V>
  void forLexicalStates(V const& v) const {
    forStates(v, true, false, false);
  }

  template <class V>
  void forLexicalStates(V const& v) {
    forStates(v, true, false, false);
  }

  typedef IHypergraphStates Self;

  /**
     Visits states in the hypergraph. Start state is visited
     first if available.
  */
  template <class V>
  void forStates(V const& v, bool lexical = true, bool unlabeled = true, bool notTerminal = true) const {
    // start first
    Self& hg = const_cast<Self&>(*this);
    for (StateId state = 0, N = size(); state < N; ++state) {
      Sym in = inputLabel(state);
      if (in == NoSymbol) {
        if (unlabeled) v(hg, state);
      } else if (in.isLexical()) {
        if (lexical) v(hg, state);
      } else if (!in.isSpecialTerminal())
        if (notTerminal) v(hg, state);
    }
  }

  /** start() == kNoState means no start state (which is not required for CFG
   * anyway - every lexical leaf is an axiom) */

  /** final() == kNoState means prunedEmpty() - no derivations */

  bool storesArcs() const;

  bool storesInArcs() const;

  /**
     \return iff hg is storing outarcs for at least first tail (canonical case:
     fsm where you don't care about outarcs for lexical states).
  */
  bool storesOutArcs() const;

  /**
     \return iff hg is storing *just* outarcs for first tail (canonical case:
     fsm where you don't care about outarcs for lexical states).
  */
  bool storesFirstTailOutArcs() const;

  virtual bool firstTailOnly() const { return storesFirstTailOutArcs() && !storesInArcs(); }

  /**
     \return storesOutArcs() && !storesFirstTailOutArcs().
  */
  bool storesAllOutArcs() const;

  std::string name() const OVERRIDE { return typename_; }

  /**
     helper for prettier iteration syntax: forall (ArcId arci,
     hg.inArcIds(headState)) { Arc *arc = inArc(headState, arci); }.
  */

  ArcIdRange inArcIds(StateId state) const {
    return ArcIdRange(ArcIdIterator(0), ArcIdIterator(numInArcs(state)));
  }

  ArcIdRange outArcIds(StateId state) const {
    return ArcIdRange(ArcIdIterator(0), ArcIdIterator(numOutArcs(state)));
  }

  /**
     \return # of in arcs for state (or 0 if in arcs aren't stored)
  */
  virtual ArcId numInArcs(StateId state) const = 0;

  /**
     \return # of out arcs for state (or 0 if out arcs aren't stored)
  */
  virtual ArcId numOutArcs(StateId state) const = 0;

  bool getNumArcs(StateId state, bool outArcs) const {
    return outArcs ? numOutArcs(state) : numInArcs(state);
  }

 protected:
  void initProcessPhase(InitProcessPhase phase) OVERRIDE {}

  StateId maxNotTerminalStateImpl() const {
    StateId const numStates = size();
    if (numStates == 0) return kNoState;
    for (StateId s = numStates;;) {
      if (!--s) return kNoState;
      if (!hasTerminalLabel(s)) return s;
    }
  }

  void setStartFinalFrom(IHypergraphStates const& o) {
    start_ = o.start_;
    final_ = o.final_;
  }

  StateId start_;
  StateId final_;
};  // end IHypergraphStates


}}

#endif
