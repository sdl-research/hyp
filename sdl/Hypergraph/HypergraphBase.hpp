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
#include <sdl/Hypergraph/Label.hpp>
#include <sdl/Hypergraph/ArcBase.hpp>
#include <sdl/Hypergraph/Properties.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Sym.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Util/Map.hpp>
#include <sdl/Util/PointerSet.hpp>
#include <sdl/Util/Once.hpp>
#include <graehl/shared/noreturn.hpp>
#include <sdl/Util/MaybeRef.hpp>

namespace sdl {
namespace Hypergraph {

using Vocabulary::WhichFstComposeSpecials;

/**
   Parts of IHypergraph that relate to states and don't care about arc type.
*/
struct HypergraphBase : Resource {

  static char const* staticType() { return "HypergraphBase"; }
  char const* dynamicType() const { return typename_.c_str(); }
  std::string typename_;

 protected:
  // for uncomputedProperties() - properties() forces update of
  // fsm/graph/etc. mutable because properties() is const.
  mutable Properties properties_;
  void initProcessPhase(InitProcessPhase phase) override {}

  StateId maxNotTerminalStateImpl() const {
    StateId const numStates = size();
    if (numStates == 0) return kNoState;
    for (StateId s = numStates;;) {
      if (!--s) return kNoState;
      if (!hasTerminalLabel(s)) return s;
    }
  }
  StateId start_;
  StateId final_;

 public:
  void setStart(StateId s) {
    assert(isMutable());
    start_ = s;
  }

  void setFinal(StateId s) {
    assert(isMutable());
    final_ = s;
  }

  std::string const& typeStr() const { return typename_; }

  HypergraphBase(std::string const& typeString) : typename_(typeString) { setThreadSpecific(); }

  virtual ~HypergraphBase() {}

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
  virtual Properties uncomputedProperties() const { return properties_; }


  /** fast estimate for reserving storage for # of edges (getNumEdges may be linear time in #states) */
  virtual std::size_t estimatedNumEdges() const { return size(); }

  bool tryForceFirstTailOutArcs() const {
    if (storesFirstTailOutArcs()) return true;
    if (isMutable()) {
      const_cast<HypergraphBase*>(this)->forceFirstTailOutArcs();
      return true;
    } else
      return false;
  }

  /// will throw ConfigException if hg is not mutable and outarcs (or
  /// first-tail) aren't already stored. else you'll have at least first-tail
  /// out-arcs
  virtual void forceFirstTailOutArcs() {
    if (!(properties() & kStoreFirstTailOutArcs))
      SDL_THROW_LOG(Hypergraph.forceFirstTailOutArcs, ConfigException,
                    "hypergraph doesn't have at least first-tail-out-arcs");
  }

  /// so you can use GraphInlineInputLabels
  virtual void forceNotAllOutArcs() {
    if (storesAllOutArcs())
      SDL_THROW_LOG(Hypergraph.forceFirstTailOutArcs, ConfigException,
                    "hypergraph should not store *all* out-arcs");
  }

  void forceFirstTailOutArcsOnly() {
    this->forceFirstTailOutArcs();
    this->removeInArcs();
  }

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
     \return hg is an Fsm-Graph with each arc having exactly 2 tails

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
    for (StateIdInterval states(possiblyInputTerminalLabeledStates()); states.first < states.second; ++states.first)
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
  virtual StateId sizeForInputLabels() const { return sizeForLabels(); }

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
     by Tokens.hpp-so at most one lexical tail per arc can
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

  typedef HypergraphBase Self;

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

  std::string name() const override { return typename_; }

  /**
     helper for prettier iteration syntax: forall (ArcId arci :
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

  /// lexical or special terminal (but not jump-wall or block start/end or epsilon) symbol
  virtual Sym firstRuleSrcInput(ArcBase const* a) const { return firstRuleSrcInputTpl(a); }
  /// \return NoSymbol if none
  virtual Sym firstLexicalInput(ArcBase const* a) const { return firstLexicalInputTpl(a); }

  /// \return NoSymbol if none
  virtual Sym firstLexicalOutput(ArcBase const* a) const { return firstLexicalOutputTpl(a); }

  /// \return kNullLabelPair if none
  virtual LabelPair firstLexicalLabelPair(ArcBase const* a) const { return firstLexicalLabelPairTpl(a); }

  virtual LabelPair firstLexicalLabelPairOrEps(ArcBase const* a) const {
    LabelPair r = firstLexicalLabelPairTpl(a);
    return r.first == NoSymbol ? getEpsilonLabelPair() : r;
  }

  /// \return whether empty string is accepted (other paths that aren't empty string may also be present)
  virtual bool containsEmptyString() const;

  /// \return kNullLabelPair if none
  virtual LabelPair firstLexicalInputLabelPair(ArcBase const* a) const {
    return firstLexicalInputLabelPairTpl(a);
  }

  /// \return kNullLabelPair if none
  virtual LabelPair firstLexicalOutputLabelPair(ArcBase const* a) const {
    return firstLexicalOutputLabelPairTpl(a);
  }

  typedef StateId const* StateIdPtr;

  /// \return -1 if none
  virtual TailId firstLexicalTail(StateIdPtr tails, TailId n) const {
    for (TailId i = 0; i < n; ++i)
      if (hasLexicalLabel(tails[i])) return i;
    return (TailId)-1;
  }

  TailId firstLexicalTail(StateIdContainer const& tails) const {
    return firstLexicalTail(&tails[0], tails.size());
  }

  TailId firstLexicalTail(ArcBase const* a) const { return firstLexicalTail(a->tails_); }

  /// annotations are special terminals other than eps, rho, sigma, phi (the
  /// compose-specials) and if they appear on graphs must occupy tails 1...k consecutive. (only before
  /// non-annotations). \return k + 1 (i.e. 1 if there are none).
  virtual TailId endAnnotations(StateIdPtr tails, TailId n) const {
    for (TailId i = 1; i < n; ++i)
      if (!Vocabulary::isAnnotation(inputLabel(tails[i]))) return i;
    return n;
  }

  TailId endAnnotations(StateIdContainer const& tails) const {
    return endAnnotations(&tails[0], tails.size());
  }

  TailId endAnnotations(ArcBase const* arc) const { return endAnnotations(arc->tails_); }

  /// same as LabelPair(inputLabel(s), outputLabel(s)), but may be faster if overriden
  LabelPair fsmLabelPair(ArcBase const& arc) const { return labelPair(arc.fsmSymbolState()); }
  virtual Distance heuristic(StateId st) const { return 0; }
  virtual void computeHeuristic() const {}
  // does s have an outgoing transition matching every possible input symbol
  virtual bool hasAllOut(StateId s) const = 0;

  /**
     an fsm arc has two tails: second has the terminal (special or
     lexical)label, first does not.
  */
  virtual bool isFsmArc(ArcBase const& a) const {
    StateIdContainer const& tails = a.tails_;
    return tails.size() == 2 && !hasTerminalLabel(tails[0]) && hasTerminalLabel(tails[1]);
  }


  /**
     notes on kFsm and kGraph properties: when you query these, a linear time
     check is performed; thereafter (as long as no new arcs are added to an
     IMutableHypergraph) you get a constant time answer. here are the definitions:

     definition of axiom: start state or terminal-labeled state. (was
     formerly excluding special terminals e.g. epsilon, which is bad for obvious
     reasons)

     kGraph property: arcs have >= 1 tails, terminal label iff tail is not first

     (this means that if you don't have start and final state for a graph, its nbest
     is empty)

     kFsm property: arcs have 2 tails, terminal label iff tail
     is not first. kFsm implies kGraph.

     (a graph behaves just like an fsm but with any number of label states, not exactly 1)
  */

  /**
     a graph arc has N tails: all but the first have an input terminal label (special
     or lexical).

     note: an arc that has in the first tail a lexical (leaf) state is not a
     graph. graphs must have a defined start and final state or else they're
     considered empty.

     fsm is set to false if the arc isn't an fsm arc.

     also checks whether at most one tail label may be lexical (this is required for
     storing word->word alignments in arc feature ids)

     a lexical head label is never allowed (so both graph, fsm, and oneLexical would be false)

     fsm and oneLexical were initialized by caller as true and are only set
     false by us (we never set them true). further, it's ok for us to set them
     false as soon as any arc isn't a graph arc. oneLexical might get set to
     false as soon as we detect non-graph (even though technically a non-graph
     CFG could have at most one lex label per arc)
  */
  virtual bool isGraphArc(ArcBase const& a, bool& fsm, bool& oneLexical) const;

  bool isGraphArc(ArcBase const& arc) const {
    bool ignore;
    return isGraphArc(arc, ignore, ignore);
  }

  /**
     \return (start is none only if final is also none) and forall arcs,
     isGraphArc(arc). sets isFsm to the result of isFsmCheck()

     isFsm/Graph does not imply storing any out arcs - it's a
     structural check
  */
  virtual bool isGraphCheck(bool& isFsm, bool& oneLexical) const = 0;
  /**
     \return (start is none only if final is also none) and forall arcs,
     isFsmArc(arc).

     isFsm/Graph does not imply storing any out arcs - it's a
     structural check
  */

  virtual bool isFsmCheck() const = 0;

  /// for detail::isGraphArcImpl-compare to more efficient MutableHypergraphLabels impl
  Sym inputLabelImpl(StateId s) const { return inputLabel(s); }

  /// \return assuming state s has an explicit input label of some sort
  /// e.g. epsilon, is the labelpair considered 'lexical' by virtue of a lexical
  /// output symbol - intended use: inputLabelImpl(s) || elseHasLexicalOutputLabelImpl(s)
  bool elseHasLexicalOutputLabelImpl(StateId s) const { return outputLabel(s).isLexical(); }

  Sym getFsmLabel(ArcBase const& a, LabelType labelType) const {
    assert(isFsm());
    StateId labelState = a.fsmSymbolState();
    return labelType == kOutput ? outputLabel(labelState) : inputLabel(labelState);
  }
  Sym getFsmInput(ArcBase const& a) const {
    assert(isFsm());
    StateId l = a.fsmSymbolState();
    return inputLabel(l);
  }
  Sym getFsmOutput(ArcBase const& a) const {
    assert(isFsm());
    StateId l = a.fsmSymbolState();
    return outputLabel(l);
  }

  LabelPair getFsmLabelPair(ArcBase const& a) const {
    assert(isFsm());
    StateId l = a.fsmSymbolState();
    return LabelPair(inputLabel(l), outputLabel(l));
  }

  typedef std::vector<ArcsContainer> Adjs;

  typedef Util::MaybeRef<Adjs const> AdjsPtr;

  /**
     may be invalidated by IMutableHypergraph addArc operations. may have fewer than #states entries
  */
  virtual AdjsPtr inArcs() const;

  /**
     may be invalidated by IMutableHypergraph addArc operations. may have fewer than #states entries
  */
  virtual AdjsPtr getFirstTailOutArcs(bool allowNonFirstTail = true) const;

  virtual void getFirstTailOutArcs(Adjs& adjs, bool allowNonFirstTail = true) const;

  virtual void getFirstTailOutArcs(StateId st, ArcsContainer& arcs, bool allowNonFirstTail = true) const;

  virtual void outArcs(StateId st, ArcsContainer& arcs) const;

  virtual void inArcs(Adjs& adjs) const;

  /**
     (if overriden) may actually be faster than making several virtual calls.
  */
  virtual void inArcs(StateId st, ArcsContainer& arcs) const;

  /**
     these may return NULL instead of empty. (the const versions will return a
     pointer to empty instead)

     you should assume that these pointers are invalidated whenever calling
     addArcs with bigger stateids than before, in the relevant position (head
     for in, first tail for first-tail-out-arcs, all tails for out-arcs))

     if you can bound how many additional arcs you'll add, try reserveAdditionalStates(n)
  */
  virtual ArcsContainer* maybeInArcs(StateId state) {
    unimplementedMutableOnly("maybeInArcs");
    return 0;
  }

  virtual ArcsContainer const* maybeInArcs(StateId state) const {
    unimplementedMutableOnly("maybeInArcs");
    return 0;
  }

  virtual ArcsContainer* maybeOutArcs(StateId state) {
    unimplementedMutableOnly("maybeOutArcs");
    return 0;
  }

  virtual ArcsContainer const* maybeOutArcs(StateId state) const {
    unimplementedMutableOnly("maybeOutArcs");
    return 0;
  }

  ArcsContainer const* maybeInArcsConst(StateId state) const { return maybeInArcs(state); }

  ArcsContainer const* maybeOutArcsConst(StateId state) const { return maybeOutArcs(state); }

  ArcsContainer const* maybeArcsConst(StateId state, bool inarcs) const {
    return inarcs ? maybeInArcs(state) : maybeOutArcs(state);
  }

  template <class V>
  void visitOutArcs(StateId s, V const& v) const {
    ArcsContainer const* arcs = maybeOutArcs(s);
    if (arcs)
      for (ArcBase* arc : *arcs) v(arc);
  }

  template <class V>
  void visitInArcs(StateId s, V const& v) const {
    ArcsContainer const* arcs = maybeInArcs(s);
    if (arcs)
      for (ArcBase* arc : *arcs) v(arc);
  }

  /// allow adding this many states without invalidating maybe*Arcs
  virtual void reserveAdditionalStates(StateId n) { unimplementedMutableOnly("reserveAdditionalStates"); }

  virtual ArcBase* inArc(StateId state, ArcId aid) const = 0;

  virtual ArcBase* outArc(StateId state, ArcId aid) const = 0;

  virtual HypergraphBase* clone() const = 0;
  typedef ArcBase Arc;
#include <sdl/Hypergraph/src/IHypergraphForArcs.ipp>

  void throwStoresNoArcs() const NORETURN {
    SDL_THROW_LOG(Hypergraph.IHypergraphBase, InvalidInputException,
                  dynamicType() << " has no arcs (use kStoreInArcs and/or kStoreOutArcs)");
  }

  virtual bool isMutable() const;

  typedef std::vector<Sym> Labels;
  typedef std::pair<Labels const*, Labels const*> MaybeLabels;

  /// for MutableHypergraph, this will give direct read-write access to labels,
  /// which may have fewer elements than #states. of course, modifying these
  /// labels (by casting away const) could invalidate label-dependent
  /// (e.g. kOneLexical) properties, after which you should
  /// clearUncomputedProperties(...)
  virtual MaybeLabels maybeLabels() const { return MaybeLabels((Labels const*)0, (Labels const*)0); }

  /// return by value should optimize fine; Labels[s] = inputLabel(s) if
  /// !outputLabels, else outputLabel(s). note: outputLabels missing has special
  /// interpretation (same as input). returned labels size may end early for
  /// no-label, i.e. be less than size()
  Labels copyOfLabels(bool outputLabels = false) {
    MaybeLabels labels = maybeLabels();
    Labels const* from = outputLabels ? labels.second : labels.first;
    if (from)
      return *from;
    else
      return Labels();
  }


  virtual void printArc(std::ostream& out, ArcBase const* a, bool inlineLabel) const;

  void printArc(std::ostream& out, ArcBase const* a) const;

  virtual void print(std::ostream& out) const;

  bool hasGraphInlineLabels() const { return hasProperties(uncomputedProperties(), kGraphInlineInputLabels); }

  /// for debugging
  void printStderr() const;

  // return true if s has no outarcs or s is the left tail of the first of them,
  // i.e. for fsm/graph, the !hasTerminalLabel states
  bool isGraphState(StateId state) const {
    assert(storesOutArcs());
    ArcId n = numOutArcs(state);
    return !n || outArc(state, 0)->isFsmFrom(state);
  }

  ///// below this cut: methods for IMutableHypergraph<Arc> (isMutable())

  void unimplementedMutableOnly(char const* method) const NORETURN {
    SDL_THROW_LOG(Hypergraph.IHypergraphBase, UnimplementedException,
                  "IMutableHypergraph::" << method << " is unimplemented for " << dynamicType());
  }

  /// (it's simpler to use a single virtual base rather than two)

  virtual void removeDeletedArcs(Util::PointerSet const& deletedArcPointers) {
    unimplementedMutableOnly("removeDeletedArcs");
  }

  /// remove kCanonicalLex property
  virtual void clearCanonicalLex() { unimplementedMutableOnly("clearCanonicalLex"); }

  /// leave kCanonicalLex alone (may still be set) but clear cache so currently existing states won't be
  /// reused
  virtual void clearCanonicalLexCache() { unimplementedMutableOnly("clearCanonicalLexCache"); }

  /// return true if no duplicate states with same labelpair, setting kCanonicalLex and rebuilding cache
  virtual bool forceCanonicalLex() {
    unimplementedMutableOnly("forceCanonicalLex");
    return 0;
  }

  /// return true if no duplicate states with same labelpair. invalidate existing cache
  virtual bool ensureCanonicalLex() {
    unimplementedMutableOnly("ensureCanonicalLex");
    return 0;
  }

  /**
     \return existing StateId with given label pair, if any. may only call if canonicalLex()
  */
  virtual StateId canonicalExistingStateForLabelPair(LabelPair io) {
    unimplementedMutableOnly("canonicalExistingStateForLabelPair");
    return 0;
  }
  /**
     reserve at least this many states (optional; for efficiency - not required to call with an upper bound,
     or at all).
  */
  virtual void reserve(StateId) {}

  /** an output symbol of NoSymbol means the output changes when you
      setInputLabel later; an explicit same-symbol will fix the output label
      against that eventuality.
  */
  virtual void setLabelPair(StateId state, LabelPair io) { unimplementedMutableOnly("setLabelPair"); }

  void setLabelPair(StateId state, Sym in, Sym out = NoSymbol) { setLabelPair(state, LabelPair(in, out)); }

  virtual StateId addState() { return addStateId(nextStateId()); }

  // optional to call this - might clear out some garbage. won't set
  // kCanonicalLex if it isn't set already (use forceCanonicalLex for
  // that). \return whether labels are in fact canonical
  virtual bool rebuildCanonicalLex() {
    unimplementedMutableOnly("rebuildCanonicalLex");
    return 0;
  }

  /// respects kCanonicalLex property
  virtual StateId addState(Sym inputLabel) {
    unimplementedMutableOnly("addState");
    return 0;
  }

  /**
     addStateId(nextStateId(), ...) is the same as addState(, ...) always.
  */
  virtual StateId nextStateId() const {
    unimplementedMutableOnly("nextStateId");
    return 0;
  }

  /**
     remove last addState provided no arcs using it have been added
  */
  virtual void removeLastState() { unimplementedMutableOnly("removeLastState"); }

  /// note: if state was already added, it keeps existing arcs/labels. returns state always - cannot fail
  virtual StateId addStateId(StateId state) {
    unimplementedMutableOnly("addStateId");
    return 0;
  }

  virtual void addStateId(StateId state, LabelPair l);

  virtual StateId addStateId(StateId stateId, Sym inputLabel, Sym outputLabel) {
    unimplementedMutableOnly("addStateId");
    return 0;
  }

  virtual StateId addState(Sym inputLabel, Sym outputLabel) {
    unimplementedMutableOnly("addState");
    return 0;
  }

  StateId addState(LabelPair io) { return addState(input(io), output(io)); }

  virtual void setInputLabel(StateId state, Sym label) { unimplementedMutableOnly("setInputLabel"); }

  // Remember, setting to NoSymbol (or never setting) means output is
  // same as input.
  virtual void setOutputLabel(StateId state, Sym label) { unimplementedMutableOnly("setOutputLabel"); }

  virtual void setVocabulary(IVocabularyPtr const&) { unimplementedMutableOnly("setVocabulary"); }
  void setVocabulary(IPerThreadVocabulary& perThreadVocab) {
    setVocabulary(perThreadVocab.getPerThreadVocabulary());
  }

  /**
     Add existing labels to new vocab, and setVocabulary(vocab).
  */
  void translateToVocabulary(IVocabularyPtr const& newVocab);

  /// make our hg have the same vocab as another
  template <class A>
  void takeVocabulary(IHypergraph<A> const& h) {
    this->setVocabulary(h.getVocabulary());
  }

  virtual void forceInArcs() {
    if (!storesInArcs()) unimplementedMutableOnly("forceInArcs");
  }

  virtual void forceOutArcs() {
    if (!storesOutArcs()) unimplementedMutableOnly("forceOutArcs");
  }

  virtual void removeInArcs() {
    if (storesInArcs()) unimplementedMutableOnly("removeInArcs");
  }

  virtual void removeOutArcs() {
    if (storesOutArcs()) unimplementedMutableOnly("removeOutArcs");
  }

  /**
     should set kArcsAdded property so we can detect acyclic, fsm, etc.
  */
  virtual void addArc(ArcBase*) { unimplementedMutableOnly("addArc"); }

  virtual bool checkValid() const;

  // if we want we can move even more of IHypergraph / IMutableHypergraph inline
  // helper methods here.

  /// don't modify properties_ unless you're sure it's correct (i.e. for library
  /// writers only). use forceProperties instead to effect a change
  virtual void addUncomputedProperties(Properties p) const {
    assert(isMutable());
    properties_ |= p;
  }

  virtual void clearUncomputedProperties(Properties p) const {
    assert(isMutable());
    properties_ &= ~p;
  }

  /**
     instead of checkGraph, trust the caller.
  */
  virtual void setFsmGraphOneLexical(bool fsm, bool graph, bool one) {
    assert(isMutable());
    setUncomputedPropertiesAt(kGraph, graph);
    setUncomputedPropertiesAt(kFsm, fsm);
    setUncomputedPropertiesAt(kOneLexical, one);
  }

  /**
     if on, set bit, else clear it. does not do anything except change the
     property bit value so should be used by library writers only.
  */
  void setUncomputedPropertiesAt(Properties bits, bool on) {
    assert(isMutable());
    if (on)
      addUncomputedProperties(bits);
    else
      clearUncomputedProperties(bits);
  }

  virtual void setPropertyBit(Properties bits, bool on = true) {
    assert(isMutable());
    setUncomputedPropertiesAt(bits, on);
  }

  /// vouch for no cycles (finite # of trees/paths). may not be important to
  /// notify since for now we try acyclic best paths by default first, falling
  /// back if needed
  void promiseAcyclic() const {
    assert(isMutable());
    addUncomputedProperties(kAcyclic);
  }

  void unknownAcyclic() const { clearUncomputedProperties(kAcyclic | kSortedStates); }

  void unknownSortedStates() const { clearUncomputedProperties(kSortedStates); }

  /// attempt to print adjacencies even if they're wrong (i.e. no sanity checking). for debugging.
  virtual void printUnchecked(std::ostream& out) const;
};

inline std::ostream& operator<<(std::ostream& out, HypergraphBase const& self) {
  self.print(out);
  return out;
}

/// \return max state appearing as tail of an arc
StateId maxTail(HypergraphBase const& h);

/// \return max state appearing as head of an arc
StateId maxHead(HypergraphBase const& h);

/// \return max state appearing as head or tail of some arc
StateId maxArcState(HypergraphBase const& h);

/// \return start, final, maxArcState
StateId maxState(HypergraphBase const& h);

inline PrintProperties printProperties(HypergraphBase const& hg) {
  return PrintProperties(hg.properties());
}

inline Sym getFsmInputLabel(HypergraphBase const& hg, ArcBase const& arc) {
  assert(arc.tails_.size() == 2);
  return hg.inputLabel(arc.tails_[1]);
}

inline void setFsmInputLabel(HypergraphBase* pHg, ArcBase const& arc, Sym symid) {
  assert(arc.tails_.size() == 2);
  pHg->setInputLabel(arc.tails_[1], symid);
}

inline void setFsmOutputLabel(HypergraphBase* pHg, ArcBase const& arc, Sym symid) {
  assert(arc.tails_.size() == 2);
  return pHg->setOutputLabel(arc.tails_[1], symid);
}

inline bool isEpsilonLikeGraphArcAnyWeight(HypergraphBase const& hg, ArcBase const& arc) {
  StateIdContainer const& tails = arc.tails_;
  return tails.size() <= 1 || hg.labelPair(tails[1]) == kEpsilonLabelPair;
}


/**
   \return the single lexical symbol if there is one, else NoSymbol. throw if more than one lexical symbol
*/
inline Sym getSingleLexicalLabel(HypergraphBase const& hg, ArcBase const& arc, LabelType labelType = kInput) {
  StateIdContainer const& tails = arc.tails();
  for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
    Sym const tailSym = hg.label(*i, labelType);
    if (tailSym.isLexical()) return tailSym;
  }
  return NoSymbol;
}

inline TailId nLexicalLabels(HypergraphBase const& hg, ArcBase const& arc, LabelType labelType = kInput) {
  StateIdContainer const& tails = arc.tails_;
  TailId nLexical = 0;
  for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i)
    nLexical += (bool)hg.label(*i, labelType).isLexical();
  return nLexical;
}

inline Sym getFsmOutputLabel(HypergraphBase const& hg, ArcBase const& arc) {
  assert(arc.tails_.size() == 2);
  return hg.outputLabel(arc.tails_[1]);
}

void printState(std::ostream& out, StateId s, HypergraphBase const* hg, bool inlineLabel = false);
void printState(std::ostream& out, StateId s, HypergraphBase const& hg, bool inlineLabel = false);

void printArcTails(std::ostream& out, StateIdContainer const& tails, HypergraphBase const* hg,
                   bool inlineGraphLabels = false);

inline void print(std::ostream& out, StateIdContainer const& tails, HypergraphBase const* hg) {
  printArcTails(out, tails, hg);
}

inline void print(std::ostream& out, Syms const& syms, HypergraphBase const* hg) {
  print(out, syms, hg ? hg->vocab() : 0);
}

void printArc(std::ostream& out, ArcBase const* arc, HypergraphBase const* hg, bool inlineGraphLabels);

void printArc(std::ostream& out, ArcBase const* arc, HypergraphBase const* hg);

template <class Arc>
void printArc(std::ostream& out, Arc const* arc, HypergraphBase const* hg, bool inlineLabel) {
  printArc(out, (ArcBase const*)arc, hg, inlineLabel);  // from HypergraphBase.hpp
  out << " / " << arc->weight_;
}

template <class Arc>
void printArc(std::ostream& out, Arc const* arc, HypergraphBase const* hg) {
  printArc(out, arc, hg, hg ? hg->hasGraphInlineLabels() : false);
}

template <class Arc>
void printArc(std::ostream& out, Arc const* arc, HypergraphBase const& hg) {
  printArc(out, arc, &hg, hg.hasGraphInlineLabels());
}

template <class Arc>
void print(std::ostream& out, Arc const* arc, HypergraphBase const* hg) {
  printArc(out, arc, hg);
}


inline void print(std::ostream& out, ArcBase const* arc, HypergraphBase const* hg) {
  if (hg)
    hg->printArc(out, arc);
  else
    out << *arc;
}

inline void print(std::ostream& out, ArcBase const& arc, HypergraphBase const* hg) {
  if (hg)
    hg->printArc(out, &arc);
  else
    out << arc;
}

inline void print(std::ostream& out, ArcBase const* arc, HypergraphBase const& hg) {
  hg.printArc(out, arc);
}

inline void print(std::ostream& out, ArcBase const& arc, HypergraphBase const& hg) {
  hg.printArc(out, &arc);
}

inline void print(std::ostream& o, StateId s, HypergraphBase const& hg) {
  printState(o, s, &hg);
}

inline void print(std::ostream& o, StateId s, HypergraphBase const* hg) {
  printState(o, s, hg);
}

#ifndef _MSC_VER
#if !SDL_64BIT_STATE_ID
inline void print(std::ostream& o, std::size_t s, HypergraphBase const& hg) {
  printState(o, (StateId)s, &hg);
}
#endif
#endif

void printStartAndFinal(std::ostream& out, HypergraphBase const& hg);

struct PrintUnchecked {
  HypergraphBase const& hg;
  PrintUnchecked(HypergraphBase const& hg) : hg(hg) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintUnchecked const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { hg.printUnchecked(out); }
};


}}

#endif
