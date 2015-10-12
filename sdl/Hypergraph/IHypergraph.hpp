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

#include <sdl/Hypergraph/Properties.hpp>
#include <sdl/Hypergraph/HypergraphImpl.hpp>
#include <sdl/Hypergraph/Exception.hpp>

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
#include <sdl/Hypergraph/HypergraphBase.hpp>
#include <sdl/Hypergraph/fs/Annotations.hpp>

namespace sdl {
namespace Hypergraph {

/// tag class
struct InArcsT {};
/// tag class
struct OutArcsT {};


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
struct IHypergraph : HypergraphBase, private boost::noncopyable {
 public:
  static char const* staticType() { return "IHypergraph"; }

  typedef IHypergraph<A> Self;
  typedef shared_ptr<Self> Ptr;
  typedef shared_ptr<Self> Ref;
  typedef IHypergraph<A> Immutable;
  typedef shared_ptr<Immutable> ImmutablePtr;
  typedef shared_ptr<Immutable const> ConstImmutablePtr;
  typedef A Arc;
  typedef typename A::ArcVisitor ArcVisitor;  // like a boost::function<void (Arc *)>

  typedef typename A::Weight Weight;

  typedef FstArc<Weight, StateId> FstArcT;


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

    result_type operator()(ArcBase const* arc) const {
      result_type r;
      r.dst = arc->head_;
      r.weight = static_cast<Arc const*>(arc)->weight_;
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

  virtual Weight final(StateId s) const;

  using HypergraphBase::final;

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
  Distance heuristic(StateId st) const override { return 0; }

  /**
     may improve the estimates returned by heuristic. may be as expensive as inside-outside.

     probably will be cheap on repeated calls if hg hasn't changed (an issue for IMutableHypergraph).

     modifying the hg after calling computeHeuristic might not be noticed (and
     thus inadmissible heuristic values might be observed) - for example, if you
     change arc weights behind the scene, we won't notice.

     this is a const method (it should be harmless to call at any time), so any
     state kept by an implementation should probably be marked mutable
  */
  void computeHeuristic() const override {}

  /** \return number of edges. may be linear time. */
  std::size_t getNumEdges() const;

  // does s have an outgoing transition for every symbol (aside from
  // epsilon to final, which should be //TODO: multiple final states,
  // so we don't have to distinguish between real and fake epsilons)
  bool hasAllOut(StateId s) const override;

  /// forArcs(v) -  call v(a) for all Arc *a, exactly once for each a

  /// forArcsAllowRepeats - v(a) 1 or more times per Arc *a
#include <sdl/Hypergraph/src/IHypergraphForArcs.ipp>

  WeightCount countArcs() const {
    WeightCount r;
    forArcs(Util::visitorReference(r));
    return r;
  }

  virtual void printArcsCout() const {  // virtual preventing inlining for debugger
    printArcsOut(std::cout);
  }

  void printArcsOut(std::ostream& o, bool keepRepeats = false) const {
    forArcsOut(ArcPrinter<Arc>(o), keepRepeats);
  }

  void printArcsIn(std::ostream& o = std::cout) const { forArcsIn(ArcPrinter<Arc>(o)); }
  virtual void printArcs(std::ostream& o) const { forArcsOut(ArcPrinter<Arc>(o)); }

  void setWeight1() const { forArcsAllowRepeats(impl::ArcWeightSetOne<Arc>()); }

  // TODO: cache in props?
  bool unweighted() const { return countArcs().unweighted(); }

  IHypergraph(std::string const& typeStr = "sdl::IHypergraph") : HypergraphBase(typeStr) {}

  virtual IHypergraph<A>* clone() const override = 0;

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
  virtual Arc* inArc(StateId state, ArcId aid) const override = 0;

  virtual Arc* outArc(StateId state, ArcId aid) const override = 0;

  virtual void outAdjStates(StateId st, StateIdContainer& adjStates) const {
    ArcId i = 0, N = this->numOutArcs(st);
    adjStates.resize(N);
    for (; i < N; ++i) adjStates[i] = this->outArc(st, i)->head_;
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

  /// see Util/Generator.hpp
  typedef Util::AnyGenerator<Arc*, Util::NonPeekableT> ConstOutArcsGenerator;

  virtual ConstOutArcsGenerator outArcsConst(StateId state) const {
    assert(storesOutArcs());
    SDL_THROW_LOG(Hypergraph, UnimplementedException,
                  "outArcsConst");  // TODO - generator iterating over arc ids
  }

  /* appropriate only if the added arcs were made with new Arc(...) and
     would otherwise all leak. also: doesn't clear lists of arcs! call just
     before destroying a hypergraph, in other words */
  virtual void deleteArcs() { forArcsSafe(impl::ArcDeleter<Arc>()); }

  bool isFsmCheck() const override;
  bool isGraphCheck(bool& isFsm, bool& oneLexical) const override;

  void printArc(std::ostream& out, ArcBase const* arc, bool inlineLabel) const override {
    Hypergraph::printArc<Arc>(out, (Arc*)arc, (HypergraphBase const*)this, inlineLabel);
  }

};  // end class IHypergraph

// iterate over pointers to arcs
template <class Arc>
struct ArcPointers : public std::vector<Arc*> {
  void operator()(ArcBase* a) { this->push_back((Arc*)a); }
  ArcPointers(HypergraphBase const& h) {
    this->reserve(h.estimatedNumEdges());
    h.forArcs(Util::visitorReference(*this));
  }
  template <class V>
  void visit(V const& v) const {
    forall (Arc* arc, *this)
      v(arc);
  }
  template <class V>
  void visit(V& v) const {
    forall (Arc* arc, *this)
      v(arc);
  }
};

template <class Arc>
bool isEpsilonLikeGraphArc(IHypergraph<Arc> const& hg, Arc const& arc) {
  return isOne(arc.weight_) && isEpsilonLikeGraphArcAnyWeight(hg, arc);
}


template <class Arc>
struct PrintInArcs {
  HypergraphBase const& hg;
  StateId s;
  PrintInArcs(IHypergraph<Arc> const& hg, StateId s) : hg(hg), s(s) { assert(hg.storesInArcs()); }
  friend inline std::ostream& operator<<(std::ostream& out, PrintInArcs const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out, bool pre = true) const {
    if (pre) out << " in[" << s << "]: ";
    hg.forArcsIn(s, ArcPrinter<Arc>(out));
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
    hg.forArcsOut(s, ArcPrinter<Arc>(out));
  }
};

template <class Arc>
PrintOutArcs<Arc> printOutArcs(IHypergraph<Arc> const& hg, StateId s) {
  return PrintOutArcs<Arc>(hg, s);
}

template <class Arc>
struct ArcPrint {
  Arc const* arc;
  HypergraphBase const* hg;
  ArcPrint(Arc const* arc, HypergraphBase const* hg) : arc(arc), hg(hg) {}
  ArcPrint(Arc const* arc, HypergraphBase const& hg) : arc(arc), hg(&hg) {}
  friend inline std::ostream& operator<<(std::ostream& out, ArcPrint const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { printArc(out, arc, hg); }
};

template <class Arc>
ArcPrint<Arc> arcPrint(Arc const* arc, HypergraphBase const& hg) {
  return ArcPrint<Arc>(arc, hg);
}

template <class Arc>
ArcPrint<Arc> arcPrint(Arc const* arc, HypergraphBase const* hg) {
  return ArcPrint<Arc>(arc, hg);
}

template <class Arc>
void print(std::ostream &out, Arc const& arc, IHypergraph<Arc> const* hg) {
  printArc(out, &arc, hg);
}

template <class Arc>
void print(std::ostream &out, Arc const* arc, IHypergraph<Arc> const* hg) {
  printArc(out, arc, hg);
}

template <class Arc>
void print(std::ostream &out, Arc const& arc, IHypergraph<Arc> const& hg) {
  printArc(out, &arc, hg);
}

template <class Arc>
void print(std::ostream &out, Arc const* arc, IHypergraph<Arc> const& hg) {
  printArc(out, arc, hg);
}

}}

#include <sdl/Hypergraph/src/IHypergraph.ipp>

#endif
