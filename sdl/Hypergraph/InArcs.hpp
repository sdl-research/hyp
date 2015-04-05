// Copyright 2014-2015 SDL plc
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

    transparently index in arcs for an IHypergraph, unless they already are so
    indexed, in which case we use that.

    provides higher-performance (if mutable hg) visiting of adjacent arcs than
    through the usual ArcId based IHypergraph interface

    see <sdl/Util/Generator.hpp> for 'Generator' stuff
*/

#ifndef HYP__INARCS_JG_2013_06_10_HPP
#define HYP__INARCS_JG_2013_06_10_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Util/SmallVector.hpp>
#include <sdl/Array.hpp>


namespace sdl {
namespace Hypergraph {


/**
   will call visit.acceptIn(arc, head) for all arcs with head = head.
*/
template <class Visitor, class Arc>
void visitInArcs(Visitor& visit, StateId head, IHypergraph<Arc> const& hg,
                 IMutableHypergraph<Arc> const* hgAsMutable = 0) {
  if (hgAsMutable) {
    typedef typename IMutableHypergraph<Arc>::ArcsContainer ArcsContainer;
    ArcsContainer const* arcs = hgAsMutable->maybeInArcs(head);
    if (arcs)
      for (typename ArcsContainer::const_iterator a = arcs->begin(), ae = arcs->end(); a != ae; ++a)
        visit.acceptIn(*a, head);
  } else {
    for (ArcId i = 0, N = hg.numInArcs(head); i < N; ++i) visit.acceptIn(hg.inArc(head, i), head);
  }
}

/**
   will call visit.acceptOut(arc, tail) for all arcs with (first, depending on indexed type) tail = tail.
*/
template <class Visitor, class Arc>
void visitOutArcs(Visitor& visit, StateId tail, IHypergraph<Arc> const& hg,
                  IMutableHypergraph<Arc> const* hgAsMutable = 0) {
  if (hgAsMutable) {
    typedef typename IMutableHypergraph<Arc>::ArcsContainer ArcsContainer;
    ArcsContainer const* arcs = hgAsMutable->maybeOutArcs(tail);
    if (arcs)
      for (typename ArcsContainer::const_iterator a = arcs->begin(), ae = arcs->end(); a != ae; ++a)
        visit.acceptOut(*a, tail);
  } else {
    for (ArcId i = 0, N = hg.numOutArcs(tail); i < N; ++i) visit.acceptOut(hg.outArc(tail, i), tail);
  }
}

template <class Arc>
struct PeekableArcGeneratorBase {
  typedef void Peekable;
  typedef Arc* value_type;
  typedef Arc* reference;
  typedef Arc* result_type;
};

template <class Arc>
struct InArcsGenerator : PeekableArcGeneratorBase<Arc> {
  InArcsGenerator(IHypergraph<Arc> const& hg, StateId s) : i_(), end_(hg.numInArcs(s)), s_(s), hg_(&hg) {}

 protected:
  ArcId i_, end_;
  StateId s_;
  IHypergraph<Arc> const* hg_;

 public:
  StateId state() const { return s_; }

  Arc* peek() const { return hg_->inArc(s_, i_); }
  operator bool() const { return i_ != end_; }
  void pop() { ++i_; }
  Arc* get() const { return peek(); }
  void got() { pop(); }
  Arc* operator()() {
    Arc* r = get();
    got();
    return r;
  }
};

template <class Arc>
struct OutArcsGenerator : PeekableArcGeneratorBase<Arc> {
  OutArcsGenerator(IHypergraph<Arc> const& hg, StateId s) : i_(), end_(hg.numOutArcs(s)), s_(s), hg_(&hg) {}

 protected:
  ArcId i_, end_;
  StateId s_;
  IHypergraph<Arc> const* hg_;

 public:
  StateId state() const { return s_; }

  Arc* peek() const { return hg_->outArc(s_, i_); }
  operator bool() const { return i_ != end_; }
  void pop() { ++i_; }
  Arc* get() const { return peek(); }
  void got() { pop(); }
  Arc* operator()() {
    Arc* r = get();
    got();
    return r;
  }
};

/**
   this may be more efficient even for non-mutable hg (but then you have to keep
   a copy of the arcs for the duration)

*/
template <class Arc>
struct ContainerArcsGenerator : PeekableArcGeneratorBase<Arc> {
  template <class ArcsContainer>
  void init(ArcsContainer const* arcs) {
    if (arcs && !arcs->empty()) {
      i = &*arcs->begin();
#if SDL_WIN32_SECURE_SCL_WORKAROUND
      end = i + arcs->size();
#else
      end = &*arcs->end();
#endif
    } else
      i = end = 0;
  }
  ContainerArcsGenerator() {}
  template <class ArcsContainer>
  ContainerArcsGenerator(ArcsContainer const* arcs) {
    init(arcs);
  }
  ContainerArcsGenerator(IHypergraph<Arc> const& hg, StateId s, OutArcsT) {
    assert(hg.isMutable());
    init(static_cast<IMutableHypergraph<Arc> const&>(hg).maybeOutArcs(s));
  }
  ContainerArcsGenerator(IHypergraph<Arc> const& hg, StateId s, InArcsT) {
    assert(hg.isMutable());
    init(static_cast<IMutableHypergraph<Arc> const&>(hg).maybeInArcs(s));
  }
  typedef Arc* ArcP;
  typedef ArcP const* ArcPI;
  ArcPI i, end;
  Arc* peek() const { return *i; }
  Arc* get() const { return *i; }
  operator bool() const { return i != end; }
  void pop() { ++i; }
  void got() { ++i; }
  Arc* operator()() {
    Arc* r = get();
    got();
    return r;
  }
};

/**
   base class for InArcs, FirstTailOutArcs. template constant argument allows for no-overhead use of hg known
   to possess the adjacency already.
*/
template <class Arc, bool mustStoreNatively = false>
struct AdjacentArcsBase {
  IHypergraph<Arc> const& hg;
  typedef typename IMutableHypergraph<Arc>::ArcsContainer ArcsContainer;

 protected:
  ArcsContainer tmpInArcs, emptyInArcs;
  bool hgHasNative;
  IMutableHypergraph<Arc> const* hgAsMutable;
  typedef std::vector<ArcsContainer> AdjacentArcs;

 public:
  AdjacentArcsBase(IHypergraph<Arc> const& hg, bool hgHasNative) : hg(hg), hgHasNative(hgHasNative) {
    hgAsMutable = hg.isMutable() ? static_cast<IMutableHypergraph<Arc> const*>(&hg) : 0;
    if (hasNative())
      if (!hgHasNative)
        SDL_THROW_LOG(Hypergraph.InArcs, ConfigException,
                      "hypergraph must store in-arcs (head indexed, top down) natively");
  }
  bool hasNative() const { return mustStoreNatively || hgHasNative; }
};


/**
   transparently index in arcs for an IHypergraph, unless they already are so indexed, in which case we use
   that.
*/
template <class Arc, bool mustStoreNatively = false>
struct InArcs : AdjacentArcsBase<Arc, mustStoreNatively> {
 private:
  typedef AdjacentArcsBase<Arc, mustStoreNatively> Base;
  mutable typename Base::AdjacentArcs adj;
  using Base::hg;
  using Base::hgAsMutable;
  using Base::tmpInArcs;
  using Base::emptyInArcs;

 public:
  typedef typename Base::ArcsContainer ArcsContainer;
  using Base::hasNative;

  StateId size() const { return hg.sizeForHeads(); }

  InArcs(IHypergraph<Arc> const& hg) : Base(hg, hg.storesInArcs()), adj(hasNative() ? 0 : hg.sizeForHeads()) {
    if (!adj.empty()) hg.forArcs(*this);
  }

  void operator()(Arc* a) const {
    assert(a->head() < adj.size());
    adj[a->head()].push_back(a);
    // could grow as needed so that in case hg has nonlex states first, we don't
    // waste memory. could preallocate to maxNotTerminalState + 1, cutting
    // down on swaps as we grow. instead, i preallocate numstates, which may
    // waste if hg is sorted
  }

  template <class Visitor>
  void visitInArcs(Visitor& visit, StateId head) {
    if (hasNative())
      Hypergraph::visitInArcs(visit, head, hg, hgAsMutable);
    else if (head < adj.size())
      for (typename ArcsContainer::const_iterator a = adj[head].begin(), ae = adj[head].end(); a != ae; ++a)
        visit.accept(*a, head);
  }

  TailId numInArcs(StateId st) const {
    if (hasNative()) return hg.numInArcs(st);
    return st < adj.size() ? adj[st].size() : 0;
  }

  Arc* inArc(StateId st, TailId tail) const {
    if (hasNative()) return hg.inArc(st, tail);
    return adj[st][tail];
  }

  /**
     concurrent use of more than one return value not allowed for non-mutable
     non-inarc IHypergraph, because of single tmpInArcs
  */
  ArcsContainer const* inArcs(StateId st) {
    if (hasNative()) {
      if (hgAsMutable) return hgAsMutable->maybeInArcs(st);
      hg.inArcs(st, tmpInArcs);
      return &tmpInArcs;
    } else if (st < adj.size())
      return &adj[st];
    else
      return &emptyInArcs;
  }

  /**
     for mutable hg only.
  */
  struct Context : ContainerArcsGenerator<Arc> {
    StateId state_;
    Context(IMutableHypergraph<Arc> const* hg, StateId s)
        : ContainerArcsGenerator<Arc>(*hg, s, InArcsT()), state_(s) {}
    StateId state() const { return state_; }
  };
  typedef std::vector<Context> Stack;
  void stackPush(Stack& stack, StateId s) const { stack.push_back(Context(hgAsMutable, s)); }

  /**
     for any hg.
  */
  struct SlowContext : InArcsGenerator<Arc> {
    SlowContext(IHypergraph<Arc> const& hg, StateId s) : InArcsGenerator<Arc>(hg, s) {}
  };
  typedef std::vector<SlowContext> SlowStack;
  void stackPush(SlowStack& stack, StateId s) const { stack.push_back(SlowContext(hg, s)); }

  enum { selfLoopIsBackEdge = 0 };

  template <class Ctx, class StateOrder>
  std::size_t orderHeadsLast(StateOrder& order, StateId final, StateId nStates, std::size_t maxBackEdges) const {
    order.reserve(nStates);
    StateSet opened(nStates);
    StateSet closed(nStates);

    // a 2-bit bundle bit-vector test and set (or a single bitvector of size
    // 2*nStates would have better cache locality and reduced bit
    // arithmetic. going to a char array would take 4x the space and waste
    // cache. to be even more cute, could do base-3 since the possible states are
    // 0, open, closed (but slower math - not worth it). but we often modify just
    // one part - it's only when we find something already open that we then
    // check closed for back-edge-ness

    std::vector<Ctx> stack;
    stack.reserve(100 + nStates / 2);
    stackPush(stack, final);
    opened.set(final);

    // could use fixed sized stack of this size as well. instead we'll use vector
    Ctx* context;
    StateId pushState, openState;
    std::size_t nBack = 0;
    while (!stack.empty()) {
    nonEmptyStack:
      context = &stack.back();
      openState = context->state();
      opened.set(openState);
      while (*context) {  // next (new) child
        Arc* arc = context->get();
        context->got();
        StateIdContainer const& tails = arc->tails();
        assert(!tails.empty());
        pushState = tails[0];
        if (!opened.test_set(pushState)) {
          stackPush(stack, pushState);
          goto nonEmptyStack;
        } else if (!closed.test(pushState) && (selfLoopIsBackEdge || pushState != openState)
                   && ++nBack > maxBackEdges) {
          SDL_WARN(Hypergraph.BestPath, "aborting acyclic best after over "
                                        << maxBackEdges << " back edges; this most recent one is " << *arc);
          return nBack;
        }
      }
      order.push_back(openState);
      closed.set(openState);
      stack.pop_back();
    }
    return nBack;
  }

  template <class StateOrder>
  std::size_t orderHeadsLast(StateOrder& order, std::size_t maxBackEdges = 0, bool isGraph = true) const {
    assert(isGraph);  // TODO: we can extend this to non-graph but not trivially - would have to use similar
                      // approach as Hypergraph/Level
    StateId final = hg.final();
    SDL_DEBUG(Hypergraph.InArcs, "single axiom for graph w/ inarcs: " << final);
    if (final == kNoState) return 0;
    return hgAsMutable ? orderHeadsLast<Context>(order, final, size(), maxBackEdges)
                       : orderHeadsLast<SlowContext>(order, final, size(), maxBackEdges);
  }
};

/**
   transparently index in arcs for an IHypergraph, unless they already are so indexed, in which case we use
   that.
*/
template <class Arc, bool mustStoreNatively = false>
struct FirstTailOutArcs : AdjacentArcsBase<Arc, mustStoreNatively> {
 private:
  typedef AdjacentArcsBase<Arc, mustStoreNatively> Base;
  mutable typename Base::AdjacentArcs adj;
  using Base::hg;
  using Base::hgAsMutable;
  using Base::tmpInArcs;
  using Base::emptyInArcs;

 public:
  using Base::hasNative;
  typedef typename Base::ArcsContainer ArcsContainer;

  FirstTailOutArcs(IHypergraph<Arc> const& hg)
      : Base(hg, hg.storesOutArcs()), adj(hasNative() ? 0 : hg.size()) {
    if (!adj.empty()) hg.forArcs(*this);
  }

  StateId size() const {
    return hg.size();  // we can't bound tighter than this because we have to visit axioms only once in
                       // AppendAxioms
  }

  TailId numOutArcs(StateId st) const {
    if (hasNative()) return hg.numOutArcs(st);
    return st < adj.size() ? adj[st].size() : 0;
  }
  Arc* outArc(StateId st, TailId tail) const {
    if (hasNative()) return hg.outArc(st, tail);
    return adj[st][tail];
  }


  /**
     will call visit.acceptOut(arc, tail) for all arcs with (first, depending on indexed type) tail = tail.
  */
  template <class Visitor>
  void visitOutArcs(Visitor& visit, StateId st) {
    if (hasNative())
      Hypergraph::visitOutArcs(visit, st, hg, hgAsMutable);
    else if (st < adj.size())
      return &adj[st];
  }

  /**
     concurrent use of more than one return value not allowed for non-mutable
     non-inarc IHypergraph, because of single tmpInArcs
  */
  ArcsContainer const* outArcs(StateId st) {
    if (hasNative()) {
      if (hgAsMutable) return hgAsMutable->outArcs(st);
      hg.outArcs(st, tmpOutArcs);
      return &tmpOutArcs;
    } else if (st < adj.size())
      return &adj[st];
    else
      return &emptyOutArcs;
  }

  void operator()(Arc* a) const {
    assert(a->head() < adj.size());
    adj[a->head()].push_back(a);
    // could grow as needed so that in case hg has nonlex states first, we don't
    // waste memory. could preallocate to maxNotTerminalState + 1, cutting
    // down on swaps as we grow. instead, i preallocate numstates, which may
    // waste if hg is sorted
  }


  /**
     for mutable hg only.
  */
  struct Context : ContainerArcsGenerator<Arc> {
    StateId state_;
    Context(IMutableHypergraph<Arc> const* hg, StateId s)
        : ContainerArcsGenerator<Arc>(*hg, s, OutArcsT()), state_(s) {}
    StateId state() const { return state_; }
  };
  typedef std::vector<Context> Stack;
  void stackPush(Stack& stack, StateId s) const { stack.push_back(Context(hgAsMutable, s)); }

  /**
     for any hg.
  */
  struct SlowContext : OutArcsGenerator<Arc> {
    SlowContext(IHypergraph<Arc> const& hg, StateId s) : OutArcsGenerator<Arc>(hg, s) {}
  };
  typedef std::vector<SlowContext> SlowStack;
  void stackPush(SlowStack& stack, StateId s) const { stack.push_back(SlowContext(hg, s)); }

  enum { selfLoopIsBackEdge = 0 };

  /// explicit compact heap stack instead of naive stack recursion (for long derivations)
  template <class Ctx, class StateOrder>
  std::size_t orderTailsLast(StateOrder& order, StateId start, StateId nStates, std::size_t maxBackEdges) const {
    order.reserve(nStates);
    StateSet opened(nStates);
    StateSet closed(nStates);

    // a 2-bit bundle bit-vector test and set (or a single bitvector of size
    // 2*nStates would have better cache locality and reduced bit
    // arithmetic. going to a char array would take 4x the space and waste
    // cache. to be even more cute, could do base-3 since the possible states are
    // 0, open, closed (but slower math - not worth it). but we often modify just
    // one part - it's only when we find something already open that we then
    // check closed for back-edge-ness

    std::vector<Ctx> stack;
    stack.reserve(100 + nStates / 2);
    stackPush(stack, start);
    opened.set(start);

    // could use fixed sized stack of this size as well. instead we'll use vector
    Ctx* context;
    StateId pushState, openState;
    std::size_t nBack = 0;
    while (!stack.empty()) {
    nonEmptyStack:
      context = &stack.back();
      openState = context->state();
      opened.set(openState);
      while (*context) {  // next (new) child
        Arc* arc = context->get();
        context->got();
        pushState = arc->head();
        if (!opened.test_set(pushState)) {
          stackPush(stack, pushState);
          goto nonEmptyStack;
        } else if (!closed.test(pushState) && (selfLoopIsBackEdge || pushState != openState)
                   && ++nBack > maxBackEdges) {
          SDL_WARN(Hypergraph.BestPath, "aborting acyclic best after over "
                                        << maxBackEdges << " back edges; this most recent one is " << *arc);
          return nBack;
        }
      }
      order.push_back(openState);
      closed.set(openState);
      stack.pop_back();
    }
    return nBack;
  }

  template <class StateOrder>
  std::size_t orderTailsLast(StateOrder& order, std::size_t maxBackEdges = 0, bool isGraph = true) const {
    assert(isGraph);  // TODO: we can extend this to non-graph but not trivially - would have to use similar
                      // approach as Hypergraph/Level
    StateId start = hg.start();
    SDL_DEBUG(Hypergraph.InArcs, "single axiom for graph w/ inarcs: " << start);
    if (start == kNoState) return 0;
    return hgAsMutable ? orderTailsLast<Context>(order, start, size(), maxBackEdges)
                       : orderTailsLast<SlowContext>(order, start, size(), maxBackEdges);
  }

 private:
  ArcsContainer tmpOutArcs, emptyOutArcs;
};


}}

#endif
