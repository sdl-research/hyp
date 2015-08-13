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

    Hypergraph (or Graph, or FSM, as special cases).

    For a hypergraph:

    You have a Vocabulary

    You have input and output labels on states.

    You have arcs with weight (cost and maybe features), head (which can only
    have a nonterminal label), sequence of tails (each of which can have any
    label). We require arcs have at least one tail for now.

    You have a final state (if no final state, then the hypergraph is empty).

    You may optionally have a start state. Start state and all terminal-labeled
    states are *axioms*, meaning that they can appear at the leaves of
    derivation trees (evocative of the near-synonyms: 'forest' for a hypergraph,
    usually acyclic and thus having finitely many trees).

    You have Properties (bits that tell you what structural invariants and
    adjacency queries are available).

    A hypergraph is a graph if the first tail of every arc is a valid head state
    (no terminal label) and every other tail has a terminal label.

    Terminals are words (lexical, e.g. "brown"), or special terminals
    (e.g. <eps>)

    An FSM is a graph with always exactly two tails (so sometimes the axiom tail
    must have an <eps> label).

    Axioms can't be head states, except for the start state in graphs - but
    please forgive bugs if you use start as a head - many algorithms work
    correctly only for acyclic (DAG) graphs.

    In graphs, the legal head states are the same as the legal first-tail
    adjacent states.

    CFGs need not use a start state.

    (see docs/ for more)

    for lazy graphs, see fs/Fst.hpp

    TODO: for lazy hypergraph (top down or bottom up?) we can't allow you to
    predict the total number of states and one of start or final state
    (depending on laziness direction) can't be known in advance
*/

#ifndef HYP__IHYPERGRAPH_HPP
#define HYP__IHYPERGRAPH_HPP
#include <sdl/Hypergraph/fs/Annotations.hpp>
#pragma once

/*
  STYLE WARNINGS:

  visitArcs visitors take Arc * but in many cases you aren't permitted to modify
  such arcs. We'd make a type of Arc const* visitor but that would increase
  lines of code; TODO: decide whether to pay that.

  We could stop exposing Arc* entirely and require access to arc properties to
  be via handle provided to IHypergraph; this would allow more memory-compact
  encoding of FSA vs FST etc ... but the tradeoff would be more complicated
  access and more virtual dispatch.

  Many non-orthogonal helper methods in IHypergraph - some aid efficiency, but
  it's a lot too swallow.

*/

/*

  GOTCHA: Arc * must be given to HG with new Arc(...) or crash on HG dtor (you
  could remove all the arcs violating this property but it's hard to guarantee
  the HG dtor doesn't come first (e.g. exception causes destruction))
*/

/*TODO (algorithms):

  approx minimize weighted NFA

  exact finite CFG -> fsm (PDA stack construction). lazy might be useful

  approx CFG -> NFA

  parse / check membership of string (without doing seq-fsa -> intersect)

  copy within-alpha of weighted inside*outside

  parse = compose with trivial acceptor - but CKY type parse instead of Earley?

  bottom-up rescoring/splitting of CFG
  left-right

*/

/*TODO (structure):

  OpenFst arc mutator/iterator support (many things marked TODO)

  LAZY cfg (???)

  input/output format first-class trees (Regular Tree Grammar) independent of
  state - data structure supports height-1 RTG effectively by state id different
  from label.
*/

#include <stdexcept>
#include <cstddef>
#include <cassert>


#include <sdl/Hypergraph/Label.hpp>
#include <sdl/Hypergraph/Properties.hpp>
#include <sdl/Hypergraph/HypergraphImpl.hpp>
#include <sdl/Hypergraph/Exception.hpp>

#include <sdl/Util/MaybeRef.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/FnReference.hpp>
#include <sdl/Util/Constants.hpp>
#include <sdl/Util/Once.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/AnyGenerator.hpp>
#include <sdl/Util/Forall.hpp>

#include <sdl/SharedPtr.hpp>
#include <boost/range/size.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <sdl/Util/SmallVector.hpp>

#include <sdl/Hypergraph/OperateOn.hpp>

#include <boost/range/algorithm/max_element.hpp>
#include <sdl/Array.hpp>
#include <sdl/Hypergraph/IHypergraphStates.hpp>

namespace sdl {
namespace Hypergraph {

/// tag class
struct InArcsT {};
/// tag class
struct OutArcsT {};

namespace detail {

/**
   Implementation is templated so MutableHypergraph can bypass all the
   virtual calls to inputLabel.

   sets oneLexical to false (else leaves it alone) if a violation is
   detected. caller is presumed to init oneLexical to true first (likewise fsm)
*/
template <class HG>
inline bool isGraphArcImpl(HG const& hg, typename HG::Arc const& a, bool& fsm, bool& oneLexical) {
  StateIdContainer const& tails = a.tails_;
  TailId N = tails.size();

  if (hg.inputLabelImpl(a.head_).isTerminal()) return (fsm = oneLexical = false);

  if (N == 2) {
    bool const term1 = hg.inputLabelImpl(tails[1]).isTerminal();
    if (hg.inputLabelImpl(tails[0]).isTerminal()) {
      if (term1) oneLexical = false;
      return (fsm = false);
    } else if (!term1) {
      oneLexical = false;  // best assume not since we return early
      return (fsm = false);
    } else
      return true;
  } else {
    fsm = false;
    if (!N) return false;
    bool lex = false;
    bool const firstLabeled = hg.inputLabelImpl(tails[0]).isTerminal();
    TailId i = !firstLabeled;
    for (;; ++i) {
      if (i == N) return true;
      StateId const tail = tails[i];
      Sym const sym = hg.inputLabelImpl(tail);
      if (sym.isLexical())
        break;
      else if (!sym.isSpecialTerminal()) {
        oneLexical = false;  // best assume not since we return early
        return false;
      } else if (hg.elseHasLexicalOutputLabelImpl(tail))
        break;
    }
    for (;;) {
      if (++i == N) return true;
      StateId const tail = tails[i];
      Sym const sym = hg.inputLabelImpl(tail);
      if (sym.isLexical())
        break;
      else if (!sym.isSpecialTerminal()) {
        oneLexical = false;  // best assume not since we return early
        return false;
      } else if (hg.elseHasLexicalOutputLabelImpl(tail))
        break;
    }
    oneLexical = false;
    for (;;) {
      if (++i == N) return true;
      if (!hg.inputLabelImpl(tails[i]).isTerminal()) return false;
    }
  }
  assert(0);  // can't reach
  return true;
}

}  // ns

/// for Hypergraph/fs/* (lazy finite-state-only algorithms)
template <class WeightT, class StateT = StateId>
struct FstArc {
  typedef StateT State;
  typedef StateT InputState;
  typedef WeightT Weight;
  State dst;
  /// unlike hg.labelPair, .second will not be NoSymbol if it's the samea s .first; it will be literally equal
  LabelPair labelPair;
  Weight weight;

#if SDL_HYPERGRAPH_FS_ANNOTATIONS
  /// annotations are special symbols that don't participate in the fst translation but ride with the input hg
  /// e.g. constraint open/close. they're expected before lexical/epsilon arcs and are placed before them when
  /// going back to hg/path in fs/LazyBest or fs/SaveFst
  Syms annotations;
#endif

  void setDst(State const& dst_) { dst = dst_; }
  State const& getDst() const { return dst; }

  typedef typename Weight::FloatT Distance;
  Distance getDistance() const { return weight.getValue(); }

  void printLabel(std::ostream& out) const {
    out << '(' << labelPair.first;
    if (labelPair.first != labelPair.second) out << ' ' << labelPair.second;
    out << ')';
  }
  void printWeight(std::ostream& out) const {
    if (weight != Weight::one()) out << "/" << weight;
  }
  void print(std::ostream& out) const {
    out << dst << "<-";
    printLabel(out);
    printWeight(out);
  }

  friend inline std::ostream& operator<<(std::ostream& out, FstArc const& self) {
    self.print(out);
    return out;
  }

  void printLabel(std::ostream& out, IVocabulary const* vocab) const {
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
    if (!annotations.empty()) out << "annotations:" << Util::print(annotations, vocab);
#endif
    out << '(';
    out << vocab->str(labelPair.first);
    if (labelPair.first != labelPair.second) out << ' ' << vocab->str(labelPair.second);
    out << ')';
  }
  void print(std::ostream& out, IVocabulary const* vocab) const {
    out << dst << "<-";
    printLabel(out, vocab);
    printWeight(out);
  }

  friend inline void print(std::ostream& out, FstArc const& self, IVocabulary const* pvocab) {
    self.print(out, pvocab);
  }
};

/**
   IHypergraph base interface (A is the arc type). (see Arc.hpp)

   Hypergraphs have (input and output) labeled states, Arcs connecting states, a
   final state (if nonempty), a start state (required for nonempty Finite State
   Machine or Graph, optional for Hypergraph as tree-set), and also some
   precomputed properties.  Hypergraphs contain trees (paths, strings).

   TODO: move more members (especially the template-code ones) to free fns; some
   redundant virtual members exist for performance (so subclass impl. can use
   subclass impl. details w/o paying for virtual dispatch)

*/
template <class A>
struct IHypergraph : IHypergraphStates, private boost::noncopyable {
 public:
  typedef IHypergraph<A> Self;
  typedef shared_ptr<Self> Ptr;
  typedef shared_ptr<Self> Ref;
  typedef IHypergraph<A> Immutable;
  typedef shared_ptr<Immutable> ImmutablePtr;
  typedef shared_ptr<Immutable const> ConstImmutablePtr;
  typedef A Arc;
  typedef typename A::ArcVisitor ArcVisitor;  // like a boost::function<void (Arc *)>

  typedef typename A::Weight Weight;
  typedef typename Weight::FloatT Distance;

  typedef FstArc<Weight, StateId> FstArcT;

  /// lexical or special terminal (but not jump-wall or block start/end or epsilon) symbol
  virtual Sym firstRuleSrcInput(Arc const* a) {
    return firstRuleSrcInputTpl(a);
  }
  /// \return NoSymbol if none
  virtual Sym firstLexicalInput(Arc const* a) const { return firstLexicalInputTpl(a); }

  /// \return NoSymbol if none
  virtual Sym firstLexicalOutput(Arc const* a) const { return firstLexicalOutputTpl(a); }

  /// \return kNullLabelPair if none
  virtual LabelPair firstLexicalLabelPair(Arc const* a) const { return firstLexicalLabelPairTpl(a); }

  virtual LabelPair firstLexicalLabelPairOrEps(Arc const* a) const {
    LabelPair r = firstLexicalLabelPairTpl(a);
    return r.first == NoSymbol ? getEpsilonLabelPair() : r;
  }

  /// \return kNullLabelPair if none
  virtual LabelPair firstLexicalInputLabelPair(Arc const* a) const {
    return firstLexicalInputLabelPairTpl(a);
  }

  /// \return kNullLabelPair if none
  virtual LabelPair firstLexicalOutputLabelPair(Arc const* a) const {
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

  TailId firstLexicalTail(Arc const* a) const { return firstLexicalTail(a->tails_); }

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

  TailId endAnnotations(Arc const* a) const { return endAnnotations(a->tails_); }

  struct FstArcFor {
    void init(Self const* hg, bool allowAnnotations = false) {
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
      annotations = allowAnnotations && (true || (hg->properties() & kAnnotations));
// TODO: detect annotations property for compose speedup
#endif
      // TODO: set kAnnotations prop for hgs w/ annotations
      hg_ = hg;
      sizeForLabels_ = hg->sizeForLabels();
      assert(hg_->isGraph());
      assert(hg_->hasAtMostOneLexicalTail());
    }

    FstArcFor() {}
    FstArcFor(Self const* hg_, bool allowAnnotations = false) { init(hg_, allowAnnotations); }
    explicit FstArcFor(Self const& hg, bool allowAnnotations = false) { init(&hg, allowAnnotations); }

    typedef FstArcT result_type;

    result_type operator()(Arc const* arc) const {
      result_type r;
      r.dst = arc->head_;
      r.weight = arc->weight_;
      StateIdContainer const& tails = arc->tails_;

#if SDL_HYPERGRAPH_FS_ANNOTATIONS
      if (annotations) {
        StateIdContainer::const_iterator i = tails.begin(), e = tails.end();
        for (;;) {
          if (++i >= e) {
            r.labelPair.first = r.labelPair.second = EPSILON::ID;
            break;
          }
          StateId const t = *i;
          if (t >= sizeForLabels_) {
            // TODO: test
            r.labelPair.first = r.labelPair.second = EPSILON::ID;
            break;
          }
          r.labelPair = hg_->labelPair(t);
          if (Vocabulary::isAnnotation(r.labelPair.first))
            r.annotations.push_back(r.labelPair.first);
          else
            break;
        }
      } else
// TODO: test
#endif
      {
        if (tails.size() == 2) {
          StateId const t = tails[1];
          if (t < sizeForLabels_) {
            r.labelPair = hg_->labelPair(t);
          } else {
            r.labelPair.first = r.labelPair.second = EPSILON::ID;
          }
        } else {
          r.labelPair = hg_->firstLexicalLabelPairOrEps(arc);
        }
      }
      return r;
    }

   private:
    Self const* hg_;
    StateId sizeForLabels_;
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
    bool annotations;
#endif
  };

  using IHypergraphStates::final;

  // returns one for final(), else zero - in case we want multiple final states later
  virtual Weight final(StateId s) const;

  /**
     don't invalidate the hg you're iterating:

     don't (via IMutableHypergraph) change arc storage options, or otherwise delete states

     however, you may add states (but not arcs) e.g. in changing arc labels
  */
  virtual void visitArcs(ArcVisitor const& v) const { forArcs(v); }

  /**
     this should be an admissible heuristic (outside path to final from st has weight.getValue less than
     this.
     note: in extreme cases (negative costs possible for arcs, as may happen in tuning) this should be less
     than 0
  */
  virtual Distance heuristic(StateId st) const { return 0; }

  /**
     may improve the estimates returned by heuristic. may be as expensive as inside-outside.

     probably will be cheap on repeated calls if hg hasn't changed (an issue for IMutableHypergraph).

     modifying the hg after calling computeHeuristic might not be noticed (and
     thus inadmissible heuristic values might be observed) - for example, if you
     change arc weights behind the scene, we won't notice.

     this is a const method (it should be harmless to call at any time), so any
     state kept by an implementation should probably be marked mutable
  */
  virtual void computeHeuristic() const {}

  /** \return number of edges. may be linear time. */
  std::size_t getNumEdges() const;

  // does s have an outgoing transition for every symbol (aside from
  // epsilon to final, which should be //TODO: multiple final states,
  // so we don't have to distinguish between real and fake epsilons)
  virtual bool hasAllOut(StateId s) const;

  virtual bool checkValid() const;

  /// same as LabelPair(inputLabel(s), outputLabel(s)) may be faster if overriden

  LabelPair fsmLabelPair(Arc const& arc) const;

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
     an fsm arc has two tails: second has the terminal (special or
     lexical)label, first does not.
  */
  virtual bool isFsmArc(Arc const& a) const {
    StateIdContainer const& tails = a.tails_;
    return tails.size() == 2 && !hasTerminalLabel(tails[0]) && hasTerminalLabel(tails[1]);
  }

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
  virtual bool isGraphArc(Arc const& a, bool& fsm, bool& oneLexical) const {
    return detail::isGraphArcImpl(*this, a, fsm, oneLexical);
  }

  bool isGraphArc(Arc const& arc) const {
    bool ignore;
    return isGraphArc(arc, ignore, ignore);
  }

  /// for detail::isGraphArcImpl - compare to more efficient MutableHypergraphLabels impl
  Sym inputLabelImpl(StateId s) const { return inputLabel(s); }

  /// \return assuming state s has an explicit input label of some sort
  /// e.g. epsilon, is the labelpair considered 'lexical' by virtue of a lexical
  /// output symbol - intended use: inputLabelImpl(s) || elseHasLexicalOutputLabelImpl(s)
  bool elseHasLexicalOutputLabelImpl(StateId s) const { return outputLabel(s).isLexical(); }

  /**
     \return (start is none only if final is also none) and forall arcs,
     isGraphArc(arc). sets isFsm to the result of isFsmCheck()

     isFsm/Graph does not imply storing any out arcs - it's a
     structural check
  */
  virtual bool isGraphCheck(bool& isFsm, bool& oneLexical) const;
  /**
     \return (start is none only if final is also none) and forall arcs,
     isFsmArc(arc).

     isFsm/Graph does not imply storing any out arcs - it's a
     structural check
  */
  virtual bool isFsmCheck() const;

  /* call v(a) for all Arc *a - possibly more than once if you have nodes
     appearing more than once as a tail. */

  template <class V>
  void forArcsOut(V const& v, bool keepRepeats = false) const {
    if (storesFirstTailOutArcs() || keepRepeats)
      forArcsOutEveryTail(v);
    else
      forArcsOutOnce(v);
  }

  // f(Arc *): Val ; note f by ref
  // note: f(Arc *) because updateBy uses map type
  template <class Val, class F>
  void arcsOnce(F& f, unordered_map<Arc*, Val>& map) const {
    if (storesInArcs())
      arcsInOnce(f, map);
    else
      arcsOutOnce(f, map);
  }

  // safe even if you update arcs as you go e.g. deleting them
  template <class V>
  void forArcsOutOnceSafe(V const& v) const {
    Util::Once once;
    forArcsOutEveryTail(Util::makeVisitOnceRef(v, &once));
    // TODO: JG: more efficient: small open hash inside per-state loop (only iterating over tails),
    // or just quadratic scan (#tails is always small)
  }

  template <class V>
  void forArcsFsm(V const& v) const {
    if (!getVocabulary()) {
      forArcsOutFirstTail(v);
      return;
    }
#define FORSTATE(state)                                \
  do {                                                 \
    if (!hasLexicalLabel(state)) forArcsOut(state, v); \
  } while (0)
    StateId st = start();
    StateId state = 0, e = size();
    if (st == Hypergraph::kNoState)
      for (; state < e; ++state) FORSTATE(state);
    else {
      assert(!hasLexicalLabel(st));
      FORSTATE(st);
      for (; state < st; ++state) FORSTATE(state);
      for (++state; state < e; ++state) FORSTATE(state);
#undef FORSTATE
    }
  }

  template <class V>
  void forArcsOutOnce(V const& v) const {
    if (storesFirstTailOutArcs())
      forArcsOutEveryTail(v);
    else if (isFsm())
      forArcsFsm(v);
    else
      forArcsOutOnceSafe(v);
  }

  template <class V>
  void forArcsOut(StateId state, V const& v, bool firstTailOnly) const {
    if (firstTailOnly)
      forArcsOutFirstTail(state, v);
    else
      forArcsOut(state, v);
  }

  // return true if s has no outarcs or s is the left tail of the first of them, i.e. for Fsm, the
  // nonlexical
  // states
  bool isFsmState(StateId state) const {
    assert(storesOutArcs());
    ArcId n = numOutArcs(state);
    return n == 0 || outArc(state, 0)->fsmSrc() == state;
  }

  bool isNonsinkFsmState(StateId state) const {
    assert(storesOutArcs());
    ArcId n = numOutArcs(state);
    return n > 0 && outArc(state, 0)->fsmSrc() == state;
  }

  // v(inputLabel(a->tails_[1]), a) - input label only (fsa)
  template <class V>
  void forArcsFsa(StateId state, V const& v) const {
    for (ArcId a = 0, f = numOutArcs(state); a < f; ++a) {
      Arc* pa = outArc(state, a);
      v(inputLabel(pa->fsmSymbolState()), *pa);
    }
  }

  /**
     v(tail, &a) -> bool. for arcs a with tail 'tail'.

     \return true iff any v(tail, &a) returned true
  */

  template <class V>
  bool forArcsOutSearch(StateId tail, V const& v) const {
    if (tail == kNoState) return false;
    if (properties() & kStoreOutArcs)
      return forArcsOutAny(tail, v);
    else
      // TODO: test
      for (StateId state = 0, e = size(); state < e; ++state)
        for (ArcId a = 0, f = numInArcs(state); a != f; ++a) {
          Arc* pa = inArc(state, a);
          if (Util::contains(pa->tails_, tail))
            if (v(tail, pa)) return true;
        }
    return false;
  }

  // v(a) -> bool. continue search only if false
  template <class V>
  bool forArcsInSearch(StateId head, V const& v, bool firstTailOnly = true) const {
    if (head == kNoState) return false;
    Properties p = properties();
    if (p & kStoreInArcs)
      // TODO: test
      return forArcsInAny(head, v);
    else {
      bool storefirst = p & kStoreFirstTailOutArcs;
      for (StateId state = 0, e = size(); state < e; ++state)
        for (ArcId a = 0, f = numOutArcs(state); a != f; ++a) {
          Arc* pa = outArc(state, a);
          if (pa->head_ == head && (storefirst || !firstTailOnly || pa->tails_[0] == state))
            // TODO: test
            if (v(pa)) return true;
        }
    }
    return false;
  }

  // prefer in if have, else out
  template <class V>
  void forArcs(StateId state, V const& v) const {
    if (storesInArcs())
      forArcsIn(state, v);
    else
      forArcsOut(state, v);
  }

  /**
     v(state, &a) -> bool. for arcs a with tail 'state'.

     \return true iff any v(tail, &a) returned true
  */
  template <class V>
  bool forArcsOutAny(StateId state, V const& v) const {
    for (ArcId a = 0, f = numOutArcs(state); a < f; ++a)
      if (v(state, outArc(state, a))) return true;
    return false;
  }

  /**
     v(&a) -> bool. for arcs a with head 'state'.

     \return true iff any v(&a) returned true
  */
  template <class V>
  bool forArcsInAny(StateId state, V const& v) const {
    for (ArcId a = 0, f = numInArcs(state); a < f; ++a)
      if (v(inArc(state, a))) return true;
    return false;
  }

  template <class V>
  void forArcsOut(StateId state, V const& v) const {
    for (ArcId a = 0, f = numOutArcs(state); a < f; ++a) v(outArc(state, a));
  }

  template <class V>
  void forArcsOutEveryTail(V const& v) const {
    for (StateId state = 0, e = size(); state < e; ++state) forArcsOut(state, v);
  }

  template <class V>
  void forArcsIn(StateId state, V const& v) const {
    for (ArcId a = 0, f = numInArcs(state); a != f; ++a) v(inArc(state, a));
  }
  template <class V>
  void forArcsIn(V const& v) const {
    assert(storesInArcs());
    for (StateId state = 0, e = size(); state != e; ++state) forArcsIn(state, v);
  }

  // forArcs visits each Arc * once. for sure. also doesn't mind if there are no
  // arc indices (because this is used by destructor and we may wish to allow
  // arcless "hypergraphs" w/ states+labels only?

  void throwStoresNoArcs() const {
    SDL_THROW_LOG(Hypergraph.IHypergraph, InvalidInputException,
                  "Hypergraph has no arcs (use kStoreInArcs and/or kStoreOutArcs)");
  }

  // ok to modify arcs without affecting visit-once property
  template <class V>
  void forArcsSafe(V const& v) const {
    // no repeats
    if (storesInArcs())
      forArcsIn(v);
    else if (storesOutArcs())
      forArcsOutOnceSafe(v);
  }

  template <class V>
  void forArcs(V const& v) const {
    // no repeats
    if (storesInArcs())
      forArcsIn(v);
    else if (storesOutArcs())
      forArcsOutOnce(v);
    else
      throwStoresNoArcs();
  }

  template <class V>
  void forArcsPreferRepeats(V const& v) const {
    if (storesOutArcs())
      forArcsOutEveryTail(v);
    else if (storesInArcs())
      forArcsIn(v);
    else
      throwStoresNoArcs();
  }

  template <class V>
  void forArcsAllowRepeats(V const& v) const {
    if (storesInArcs())
      forArcsIn(v);
    else if (storesOutArcs())
      forArcsOutEveryTail(v);
    else
      throwStoresNoArcs();
  }

  virtual void printArcsCout() const {  // virtual preventing inlining for debugger
    printArcsOut(std::cout);
  }

  void printArcsOut(std::ostream& o, bool keepRepeats = false) const {
    forArcsOut(ArcPrinter(o), keepRepeats);
  }
  void printArcsIn(std::ostream& o = std::cout) const { forArcsIn(ArcPrinter(o)); }
  virtual void printArcs(std::ostream& o) const { forArcsOut(ArcPrinter(o)); }

  WeightCount countArcs() const {
    WeightCount r;
    forArcs(Util::visitorReference(r));
    return r;
  }
  void setWeight1() const { forArcsAllowRepeats(impl::set_weight1()); }
  // TODO: cache in props?
  bool unweighted() const { return countArcs().unweighted(); }

  IHypergraph(std::string const& typeStr = "sdl::IHypergraph") : IHypergraphStates(typeStr) {}

  virtual IHypergraph<A>* clone() const = 0;

  virtual ~IHypergraph() {}

  /**
     If your hypergraph is really an FSA you will naturally
     have one designated start state; if it's an actual hypergraph you
     can have an artificial start state point to all the axioms.
  */

  /**
     Not directly returning the Arc pointers here because that would
     expose the internal container.
  */
  virtual Arc* inArc(StateId state, ArcId aid) const = 0;

  virtual Arc* outArc(StateId state, ArcId aid) const = 0;

  typedef Util::small_vector<A*, 1, ArcId> ArcsContainer;

  typedef std::vector<ArcsContainer> Adjs;

  virtual void getFirstTailOutArcs(Adjs& adjs, bool allowNonFirstTail = true) const {
    StateId N = size();
    adjs.resize(N);
    Properties prop = properties();
    if (prop & (kStoreFirstTailOutArcs | kStoreOutArcs)) {
      if (prop & kStoreFirstTailOutArcs) allowNonFirstTail = true;
      for (StateId i = 0; i < N; ++i) getFirstTailOutArcs(i, adjs[i], allowNonFirstTail);
    } else {
      for (StateId i = 0; i < N; ++i)
        for (ArcId a = 0, f = numInArcs(i); a < f; ++a) {
          Arc* pa = inArc(i, a);
          adjs[pa->graphSrc()].push_back(pa);
        }
    }
  }

  virtual void inArcs(Adjs& adjs) const {
    StateId N = size();
    adjs.resize(N);
    if (storesInArcs())
      for (StateId i = 0; i < N; ++i) inArcs(i, adjs[i]);
    else {
      for (StateId i = 0; i < N; ++i)
        for (ArcId a = 0, f = numOutArcs(i); a < f; ++a) {
          Arc* pa = outArc(i, a);
          adjs[pa->head_].push_back(pa);
        }
    }
  }

  typedef Util::MaybeRef<Adjs const> AdjsPtr;

  /**
     may be invalidated by IMutableHypergraph addArc operations. may have fewer than #states entries
  */
  virtual AdjsPtr inArcs() const {
    Adjs* p = new Adjs;
    AdjsPtr r(p);
    inArcs(*p);
    return r;
  }

  /**
     may be invalidated by IMutableHypergraph addArc operations. may have fewer than #states entries
  */
  virtual AdjsPtr getFirstTailOutArcs(bool allowNonFirstTail = true) const {
    Adjs* p = new Adjs;
    AdjsPtr r(p);
    getFirstTailOutArcs(*p, allowNonFirstTail);
    return r;
  }

  /**
     this may actually be faster than making several virtual calls.
  */
  virtual void inArcs(StateId st, ArcsContainer& arcs) const {
    ArcId i = 0, N = this->numInArcs(st);
    arcs.resize(N);
    for (; i < N; ++i) arcs[i] = this->inArc(st, i);
  }

  virtual void outArcs(StateId st, ArcsContainer& arcs) const {
    ArcId i = 0, N = this->numOutArcs(st);
    arcs.resize(N);
    for (; i < N; ++i) arcs[i] = this->outArc(st, i);
  }

  virtual void outAdjStates(StateId st, StateIdContainer& adjStates) const {
    ArcId i = 0, N = this->numOutArcs(st);
    adjStates.resize(N);
    for (; i < N; ++i) adjStates[i] = this->outArc(st, i)->head_;
  }

  virtual void getFirstTailOutArcs(StateId st, ArcsContainer& arcs, bool allowNonFirstTail = true) const {
    if (allowNonFirstTail)
      return outArcs(st, arcs);
    else {
      ArcId i = 0, N = this->numOutArcs(st);
      arcs.resize(N);
      typename ArcsContainer::iterator o0 = arcs.begin(), o = o0;
      for (; i < N; ++i) {
        Arc* a = this->outArc(st, i);
        if (a->graphSrc() == st) *o++ = a;
      }
      arcs.resize((ArcId)(o - o0));
    }
  }

  ArcsContainer inArcsCopy(StateId st) const {
    ArcsContainer r;
    inArcs(st, r);
    return r;
  }

  ArcsContainer outArcsCopy(StateId st) const {
    ArcsContainer r;
    outArcs(st, r);
    return r;
  }

  LabelPair getFsmLabelPair(Arc const& a) const {
    assert(isFsm());
    StateId l = a.fsmSymbolState();
    return LabelPair(inputLabel(l), outputLabel(l));
  }
  Sym getFsmLabel(Arc const& a, LabelType labelType) const {
    assert(isFsm());
    StateId labelState = a.fsmSymbolState();
    return labelType == kOutput ? outputLabel(labelState) : inputLabel(labelState);
  }
  Sym getFsmInput(Arc const& a) const {
    assert(isFsm());
    StateId l = a.fsmSymbolState();
    return inputLabel(l);
  }
  Sym getFsmOutput(Arc const& a) const {
    assert(isFsm());
    StateId l = a.fsmSymbolState();
    return outputLabel(l);
  }

  // NO LONGER start state first, if there is one
  // - only WriteOpenFstFormat wanted that, and we replaced it w/ explicit per-state calls.
  template <class V>
  void forArcsOutFirstTail(V const& v) const {
    assert(storesOutArcs());
    StateId state = 0, e = size();
    for (; state < e; ++state) forArcsOutFirstTail(state, v);
  }

  // visits out arc only when tail node is same as first tail (can still lead to repeats, though not in Fsm)
  template <class V>
  void forArcsOutFirstTail(StateId state, V const& v) const {
    for (ArcId a = 0, f = numOutArcs(state); a < f; ++a) {
      Arc* pa = outArc(state, a);
      typename Arc::StateIdContainer const& tails = pa->tails_;
      assert(!tails.empty());
      StateId t0 = *tails.begin();
      if (t0 == state) v(pa);
    }
  }

  /// see Util/Generator.hpp
  typedef Util::AnyGenerator<Arc*, Util::NonPeekableT> ConstOutArcsGenerator;

  virtual ConstOutArcsGenerator outArcsConst(StateId state) const {
    assert(storesOutArcs());
    SDL_THROW_LOG(Hypergraph, UnimplementedException,
                  "outArcsConst");  // TODO - generator iterating over arc ids
  }

 protected:
  /* appropriate only if the added arcs were made with new Arc(...) and
     would otherwise all leak. also: doesn't clear lists of arcs! call just
     before destroying a hypergraph, in other words */
  virtual void deleteArcs() { forArcsSafe(impl::deleter()); }

  template <class Val, class F>
  void arcsInOnce(F& f, unordered_map<Arc*, Val>& map) const {
    assert(storesInArcs());
    for (StateId state = 0, e = size(); state != e; ++state)
      for (ArcId a = 0, n = numInArcs(state); a != n; ++a) Util::getLazy(f, map, inArc(state, a));
  }

  template <class Val, class F>
  void arcsOutOnce(F& f, unordered_map<Arc*, Val>& map) const {
    assert(storesOutArcs());
    for (StateId state = 0, e = size(); state != e; ++state)
      for (ArcId a = 0, n = numOutArcs(state); a != n; ++a) Util::getLazy(f, map, outArc(state, a));
  }

};  // end class IHypergraph

// iterate over pointers to arcs
template <class A>
struct ArcPointers : public std::vector<A*> {
  typedef std::vector<A*> V;
  typedef IHypergraph<A> H;
  void operator()(A* a) { this->push_back(a); }
  ArcPointers(H const& h) {
    this->reserve(h.estimatedNumEdges());
    h.forArcs(Util::visitorReference(*this));
  }
  template <class V>
  void visit(V const& v) const {
    forall (A* arc, *this)
      v(arc);
  }
  template <class V>
  void visit(V& v) const {
    forall (A* arc, *this)
      v(arc);
  }
};

template <class A>
StateId maxTail(IHypergraph<A> const& h) {
  StateId N = h.size();
  if (N == 0) return kNoState;
  if (h.storesOutArcs()) {
    for (StateId state = N; state;) {
      --state;
      if (h.numOutArcs(state)) return state;
    }
    return kNoState;
  }
  StateId r = kNoState;
  if (h.storesInArcs()) {
    for (StateId state = 0; state < N; ++state) {
      for (ArcId t = 0, e = h.numInArcs(state); t != e; ++t) {
        StateIdContainer const& tails = h.inArc(state, t)->tails_;
        if (tails.empty()) continue;
        StateId mt = *boost::max_element(tails);
        if (r == kNoState || mt > r) r = mt;
      }
    }
  }
  return r;
}

template <class A>
StateId maxHead(IHypergraph<A> const& h) {
  StateId N = h.size();
  if (N == 0) return kNoState;
  if (h.storesInArcs()) {
    for (StateId state = N; state;) {
      --state;
      if (h.numInArcs(state)) return state;
    }
    return kNoState;
  }
  StateId r = kNoState;
  if (h.storesOutArcs()) {
    for (StateId state = 0; state < N; ++state) {
      for (ArcId t = 0, e = h.numOutArcs(state); t != e; ++t) {
        StateId hd = h.outArc(state, t)->head_;
        if (r == kNoState || hd > r) r = hd;
      }
    }
  }
  return r;
}

template <class A>
StateId maxArcState(IHypergraph<A> const& h) {
  return maxState(maxHead(h), maxTail(h));
}

// I don't bother to take a lazy max (stop early if search for one component falls < previously found max)
template <class A>
StateId maxState(IHypergraph<A> const& h) {
  StateId r = maxArcState(h);
  StateId state = h.start(), f = h.final();
  r = maxState(r, state);
  r = maxState(r, f);
  r = maxState(r, h.maxLabeledState());
  return r;
}


template <class Arc>
bool isEpsilonLikeGraphArcAnyWeight(IHypergraph<Arc> const& hg, Arc const& arc) {
  StateIdContainer const& tails = arc.tails_;
  return tails.size() <= 1 || hg.labelPair(tails[1]) == kEpsilonLabelPair;
}

template <class Arc>
bool isEpsilonLikeGraphArc(IHypergraph<Arc> const& hg, Arc const& arc) {
  return isOne(arc.weight_) && isEpsilonLikeGraphArcAnyWeight(hg, arc);
}

template <class Arc>
struct PrintInArcs {
  IHypergraph<Arc> const& hg;
  StateId s;
  PrintInArcs(IHypergraph<Arc> const& hg, StateId s) : hg(hg), s(s) { assert(hg.storesInArcs()); }
  friend inline std::ostream& operator<<(std::ostream& out, PrintInArcs const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out, bool pre = true) const {
    if (pre) out << " in[" << s << "]: ";
    hg.forArcsIn(s, ArcPrinter(out));
  }
};

template <class Arc>
PrintInArcs<Arc> printInArcs(IHypergraph<Arc> const& hg, StateId s) {
  return PrintInArcs<Arc>(hg, s);
}

template <class Arc>
struct PrintOutArcs {
  IHypergraph<Arc> const& hg;
  StateId s;
  PrintOutArcs(IHypergraph<Arc> const& hg, StateId s) : hg(hg), s(s) { assert(hg.storesOutArcs()); }
  friend inline std::ostream& operator<<(std::ostream& out, PrintOutArcs const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out, bool pre = true) const {
    if (pre) out << " out[" << s << "]: ";
    hg.forArcsOut(s, ArcPrinter(out));
  }
};

template <class Arc>
PrintOutArcs<Arc> printOutArcs(IHypergraph<Arc> const& hg, StateId s) {
  return PrintOutArcs<Arc>(hg, s);
}

inline PrintProperties printProperties(IHypergraphStates const& hg) {
  return PrintProperties(hg.properties());
}

void printState(std::ostream& out, StateId sid, IHypergraphStates const& hg, bool inlineLabel = false);
void printArcTails(std::ostream& out, StateIdContainer const& tails, IHypergraphStates const* hg,
                   bool inlineGraphLabels = false);

template <class Arc>
void printArc(std::ostream& out, Arc const* arc, IHypergraphStates const* hg) {
  bool inlineGraphLabels = hasProperties(hg->properties(), kGraphInlineInputLabels);
  printState(out, arc->head_, hg);
  out << " <- ";
  printArcTails(out, arc->tails_, hg, inlineGraphLabels);
  out << " / " << arc->weight_;
}

template <class Arc>
void printArc(std::ostream& out, Arc const* arc, IHypergraphStates const& hg) {
  printArc(out, arc, &hg);
}

template <class Arc>
struct PrintArc {
  Arc* arc;
  IHypergraphStates const* hg;
  PrintArc(Arc* arc, IHypergraphStates const* hg) : arc(arc), hg(hg) {}
  PrintArc(Arc* arc, IHypergraphStates const& hg) : arc(arc), hg(&hg) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintArc const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { printArc(out, arc, hg); }
};

template <class Arc, class Hg>
PrintArc<Arc> printArc(Arc* arc, Hg const& hg) {
  return PrintArc<Arc>(arc, hg);
}


}}

#include <sdl/Hypergraph/src/IHypergraph.ipp>

#endif
