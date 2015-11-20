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

    transparently index in arcs for an IHypergraph, unless they already are so
    indexed, in which case we use that.

    provides higher-performance (if mutable hg) visiting of adjacent arcs than
    through the usual ArcId based IHypergraph interface

    see <sdl/Util/Generator.hpp> for 'Generator' stuff
*/

#ifndef HYP__INARCS_JG_2013_06_10_HPP
#define HYP__INARCS_JG_2013_06_10_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Util/SmallVector.hpp>
#include <sdl/Array.hpp>
#include <sdl/Util/DfsColor.hpp>

namespace sdl {
namespace Hypergraph {


/**
   will call visit.acceptIn(arc, head) for all arcs with head = head.
*/
template <class Visitor>
void visitInArcs(Visitor& visit, StateId head, HypergraphBase const& hg, bool isMutable) {
  if (isMutable) {
    ArcsContainer const* arcs = hg.maybeInArcs(head);
    if (arcs)
      for (ArcsContainer::const_iterator a = arcs->begin(), ae = arcs->end(); a != ae; ++a)
        visit.acceptIn(*a, head);
  } else {
    for (ArcId i = 0, N = hg.numInArcs(head); i < N; ++i) visit.acceptIn(hg.inArc(head, i), head);
  }
}

/**
   will call visit.acceptOut(arc, tail) for all arcs with (first, depending on indexed type) tail = tail.
*/
template <class Visitor>
void visitOutArcs(Visitor& visit, StateId tail, HypergraphBase const& hg, bool isMutable) {
  if (isMutable) {
    ArcsContainer const* arcs = hg.maybeOutArcs(tail);
    if (arcs)
      for (ArcsContainer::const_iterator a = arcs->begin(), ae = arcs->end(); a != ae; ++a)
        visit.acceptOut(*a, tail);
  } else {
    for (ArcId i = 0, N = hg.numOutArcs(tail); i < N; ++i) visit.acceptOut(hg.outArc(tail, i), tail);
  }
}

struct ToTail0 {
  StateId from(ArcBase const* arc) const { return arc->head_; }
  StateId to(ArcBase const* arc) const {
    assert(!arc->tails_.empty());
    return arc->tails_.front();
  }
};

struct ToHead {
  StateId from(ArcBase const* arc) const {
    assert(!arc->tails_.empty());
    return arc->tails_.front();
  }
  StateId to(ArcBase const* arc) const { return arc->head_; }
};

template <class Arc>
struct PeekableArcGeneratorBase {
  typedef void Peekable;
  typedef ArcHandle value_type;
  typedef ArcHandle reference;
  typedef ArcHandle result_type;
};

template <class Arc>
struct InArcsGenerator : PeekableArcGeneratorBase<Arc> {
  InArcsGenerator(HypergraphBase const& hg, StateId s) : i_(), end_(hg.numInArcs(s)), s_(s), hg_(&hg) {
    SDL_TRACE(Hypergraph.InArcs, "InArcsGenerator(" << s << ")");
  }

 protected:
  ArcId i_, end_;
  StateId s_;
  HypergraphBase const* hg_;

 public:
  // StateId state() const { return s_; }

  Arc* peek() const { return (Arc*)hg_->inArc(s_, i_); }
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
  OutArcsGenerator(HypergraphBase const& hg, StateId s) : i_(), end_(hg.numOutArcs(s)), s_(s), hg_(&hg) {
    SDL_TRACE(Hypergraph.InArcs, "OutArcsGenerator(" << s << ")");
  }

 protected:
  ArcId i_, end_;
  StateId s_;
  HypergraphBase const* hg_;

 public:
  // StateId state() const { return s_; }

  Arc* peek() const { return (Arc*)hg_->outArc(s_, i_); }
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
  // ContainerArcsGenerator() {}
  // ContainerArcsGenerator(ArcsContainer const* arcs) { init(arcs); }
  ContainerArcsGenerator(HypergraphBase const& hg, StateId s, OutArcsT) {
    assert(hg.isMutable());
    init(hg.maybeOutArcs(s));
    SDL_TRACE(Hypergraph.InArcs,
              "ContainerArcsGenerator("
                  << s << ", OutArcsT()): " << Util::printablePointerRange(i, end, Util::commas()));
  }
  ContainerArcsGenerator(HypergraphBase const& hg, StateId s, InArcsT) {
    assert(hg.isMutable());
    init(hg.maybeInArcs(s));
    SDL_TRACE(Hypergraph.InArcs,
              "ContainerArcsGenerator("
                  << s << ", InArcsT()): " << Util::printablePointerRange(i, end, Util::commas()));
  }
  typedef ArcHandle const* ArcHandleI;
  ArcHandleI i, end;
  ArcHandle peek() const { return (ArcHandle)*i; }
  ArcHandle get() const { return (ArcHandle)*i; }
  operator bool() const { return i != end; }
  void pop() { ++i; }
  void got() { ++i; }
  ArcHandle operator()() {
    ArcHandle r = get();
    got();
    return r;
  }
};

/**
   base class for InArcs, FirstTailOutArcs. template constant argument allows for no-overhead use of hg known
   to possess the adjacency already.
*/
template <bool kMustStoreNatively = false>
struct AdjacentArcsBase {
  HypergraphBase const& hg;

 protected:
  ArcsContainer tmpInArcs, emptyInArcs;
  bool hgHasNative;
  bool isMutable;
  typedef std::vector<ArcsContainer> AdjacentArcs;

 public:
  AdjacentArcsBase(HypergraphBase const& hg, bool hgHasNative) : hg(hg), hgHasNative(hgHasNative) {
    SDL_TRACE(Hypergraph.InArcs, "AdjacentArcsBase(" << hgHasNative << ")");
    isMutable = hg.isMutable();
    if (hasNative())
      if (!hgHasNative)
        SDL_THROW_LOG(Hypergraph.InArcs, ConfigException,
                      "hypergraph must store in-arcs (head indexed, top down) natively");
  }
  bool hasNative() const { return kMustStoreNatively || hgHasNative; }
};

typedef std::vector<StateId> StateOrder;

enum { kSelfLoopIsBackEdge = 0 };

template <class StateOrder>
struct ComputeStateOrder {
  HypergraphBase const& hg;
  StateOrder& order;
  Util::DfsColorArray color;
  StateId nBackEdges, maxBackEdges, nStates;
  ComputeStateOrder(StateOrder& order, HypergraphBase const& hg, StateId nStates, StateId maxBackEdges = kNoState)
      : hg(hg), order(order), color(nStates), maxBackEdges(maxBackEdges), nBackEdges(), nStates(nStates) {
    assert(hg.isGraph());
  }
  template <class Context>
  void start(StateId from) {
    assert(from < nStates);
    color.set(from, Util::kOpened);
    open<Context>(from);
  }
  // TODO (maybe) explicit DFS stack instead of using real stack locals
  template <class Context>
  void open(StateId from) {
    assert(from < nStates);
    assert(color[from] == Util::kOpened);
    Context context(hg, from);
    while (context) {  // next (new) child
      ArcBase const* arc = context.get();
      assert(context.from(arc) == from);
      context.got();
      StateId const to = context.to(arc);
      Util::DfsColor const prevColor = color.test_set_if_false(to, Util::kOpened);
      assert((prevColor == Util::kFresh) == !prevColor);
      // SDL_TRACE(Hypergraph.InArcs, "dfs color for " << to << " was " << prevColor << " now is " <<
      // color[to]);
      if (!prevColor) {
        assert(color[to] == Util::kOpened);
        SDL_TRACE(Hypergraph.InArcs, "visiting fresh edge " << from << " => " << to << " via " << *arc);
        open<Context>(to);
      } else if (prevColor == Util::kOpened) {  // back edge
        if (kSelfLoopIsBackEdge || to != from) {
          SDL_DEBUG(Hypergraph.InArcs, "back edge => " << to << " (already dfs-opened) from " << from
                                                       << " via " << *arc);
          SDL_DEBUG(Hypergraph.InArcs, "hg with back edge:\n" << hg);
          if (++nBackEdges > maxBackEdges) {
            SDL_INFO(Hypergraph.BestPath, "aborting acyclic best after "
                                              << nBackEdges << " back edges; this most recent one is " << *arc);
            throw CycleException();
          }
        }
      } else {  // else already queued or closed
        assert(color[to] == Util::kQueuedOrClosed);
        SDL_TRACE(Hypergraph.InArcs, "already queued " << to);
      }
    }
    order.push_back(from);
    SDL_TRACE(Hypergraph.InArcs, "dfs finished " << from);
    color.set(from, Util::kQueuedOrClosed);
    assert(color[from] == Util::kQueuedOrClosed);
  }
};


/**
   transparently index in arcs for an IHypergraph, unless they already are so indexed, in which case we use
   that.
*/
template <class Arc, bool kMustStoreNatively = false>
struct InArcs : AdjacentArcsBase<kMustStoreNatively> {
 private:
  typedef AdjacentArcsBase<kMustStoreNatively> Base;
  mutable typename Base::AdjacentArcs adj;
  using Base::hg;
  using Base::isMutable;
  using Base::tmpInArcs;
  using Base::emptyInArcs;

 public:
  using Base::hasNative;

  StateId size() const { return hg.sizeForHeads(); }

  InArcs(HypergraphBase const& hg) : Base(hg, hg.storesInArcs()), adj(hasNative() ? 0 : hg.sizeForHeads()) {
    if (!adj.empty()) hg.forArcs(*this);
  }

  void operator()(ArcBase* a) const {
    assert(a->head() < adj.size());
    adj[a->head()].push_back((ArcHandle)a);
    // could grow as needed so that in case hg has nonlex states first, we don't
    // waste memory. could preallocate to maxNotTerminalState + 1, cutting
    // down on swaps as we grow. instead, i preallocate numstates, which may
    // waste if hg is sorted
  }

  template <class Visitor>
  void visitInArcs(Visitor& visit, StateId head) {
    if (hasNative())
      Hypergraph::visitInArcs(visit, head, hg, isMutable);
    else if (head < adj.size())
      for (ArcsContainer::const_iterator a = adj[head].begin(), ae = adj[head].end(); a != ae; ++a)
        visit.accept(*a, head);
  }

  TailId numInArcs(StateId st) const {
    if (hasNative()) return hg.numInArcs(st);
    return st < adj.size() ? adj[st].size() : 0;
  }

  ArcHandle inArc(StateId st, TailId tail) const {
    if (hasNative()) return hg.inArc(st, tail);
    return (ArcHandle)adj[st][tail];
  }

  /**
     concurrent use of more than one return value not allowed for non-mutable
     non-inarc IHypergraph, because of single tmpInArcs
  */
  ArcsContainer const* inArcs(StateId st) {
    assert(emptyInArcs.empty());
    if (hasNative()) {
      if (isMutable) return hg.maybeInArcs(st);
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
  struct Context : ToTail0, ContainerArcsGenerator<Arc> {
    // StateId state_;
    Context(HypergraphBase const& hg, StateId s)
        : ContainerArcsGenerator<Arc>(hg, s, InArcsT())
    //    , state_(s)
    {}
    // StateId state() const { return state_; }
  };

  /**
     for any hg.
  */
  struct SlowContext : ToTail0, InArcsGenerator<Arc> {
    SlowContext(HypergraphBase const& hg, StateId s) : InArcsGenerator<Arc>(hg, s) {}
  };


  template <class Ctx, class StateOrder>
  std::size_t orderHeadsLast(StateOrder& order, StateId final, StateId nStates, std::size_t maxBackEdges) const {
    ComputeStateOrder<StateOrder> compute(order, hg, nStates, maxBackEdges);
    try {
      compute.template start<Ctx>(final);
    } catch (CycleException&) {
    }
    return compute.nBackEdges;
  }

  template <class StateOrder>
  std::size_t orderHeadsLast(StateOrder& order, std::size_t maxBackEdges = 0, bool isGraph = true) const {
    assert(isGraph);  // TODO: we can extend this to non-graph but not trivially - would have to use similar
    // approach as Hypergraph/Level
    StateId final = hg.final();
    SDL_DEBUG(Hypergraph.InArcs, "single axiom for graph w/ inarcs: " << final);
    if (final == kNoState) return 0;
    return isMutable ? orderHeadsLast<Context>(order, final, size(), maxBackEdges)
                     : orderHeadsLast<SlowContext>(order, final, size(), maxBackEdges);
  }
};

/**
   transparently index in arcs for an IHypergraph, unless they already are so indexed, in which case we use
   that.
*/
template <class Arc, bool kMustStoreNatively = false>
struct FirstTailOutArcs : AdjacentArcsBase<kMustStoreNatively> {
 private:
  typedef AdjacentArcsBase<kMustStoreNatively> Base;
  mutable typename Base::AdjacentArcs adj;
  using Base::hg;
  using Base::isMutable;
  using Base::tmpInArcs;
  using Base::emptyInArcs;

 public:
  using Base::hasNative;

  FirstTailOutArcs(HypergraphBase const& hg)
      : Base(hg, hg.storesOutArcs()), adj(hasNative() ? 0 : hg.size()) {
    if (!adj.empty()) hg.forArcs(*this);
  }

  StateId size() const {
    return hg.size();  // we can't bound tighter than this because we have to visit axioms only once in
    // AppendAxioms
  }

  TailId numOutArcs(StateId st) const {
    if (hasNative()) return hg.numOutArcs(st);
    return st < adj.size() ? (TailId)adj[st].size() : 0;
  }
  ArcHandle outArc(StateId st, TailId tail) const {
    if (hasNative()) return hg.outArc(st, tail);
    return (ArcHandle)adj[st][tail];
  }

  /**
     will call visit.acceptOut(arc, tail) for all arcs with (first, depending on indexed type) tail = tail.
  */
  template <class Visitor>
  void visitOutArcs(Visitor& visit, StateId st) {
    if (hasNative())
      Hypergraph::visitOutArcs(visit, st, hg, isMutable);
    else if (st < adj.size())
      return &adj[st];
  }

  /**
     concurrent use of more than one return value not allowed for non-mutable
     non-inarc IHypergraph, because of single tmpInArcs
  */
  ArcsContainer const* outArcs(StateId st) {
    assert(emptyInArcs.empty());
    if (hasNative()) {
      if (isMutable) return hg.outArcs(st);
      hg.outArcs(st, tmpOutArcs);
      return &tmpOutArcs;
    } else if (st < adj.size())
      return &adj[st];
    else
      return &emptyOutArcs;
  }

  void operator()(ArcBase* a) const {
    assert(a->head() < adj.size());
    adj[a->head()].push_back((ArcHandle)a);
    // could grow as needed so that in case hg has nonlex states first, we don't
    // waste memory. could preallocate to maxNotTerminalState + 1, cutting
    // down on swaps as we grow. instead, i preallocate numstates, which may
    // waste if hg is sorted
  }


  /**
     for mutable hg only.
  */
  struct Context : ToHead, ContainerArcsGenerator<Arc> {
    // StateId state_;
    Context(HypergraphBase const& hg, StateId s)
        : ContainerArcsGenerator<Arc>(hg, s, OutArcsT())
    //    , state_(s)
    {}
    // StateId state() const { return state_; }
  };

  /**
     for any hg.
  */
  struct SlowContext : ToHead, OutArcsGenerator<Arc> {
    SlowContext(HypergraphBase const& hg, StateId s) : OutArcsGenerator<Arc>(hg, s) {}
  };

  typedef std::vector<SlowContext> SlowStack;
  void stackPush(SlowStack& stack, StateId s) const { stack.push_back(SlowContext(hg, s)); }

  template <class Ctx>
  std::size_t orderTailsLast(StateOrder& order, StateId start, StateId nStates, std::size_t maxBackEdges) const {
    ComputeStateOrder<StateOrder> compute(order, hg, nStates, maxBackEdges);
    try {
      compute.template start<Ctx>(start);
    } catch (CycleException&) {
    }
    return compute.nBackEdges;
  }

  std::size_t orderTailsLast(StateOrder& order, std::size_t maxBackEdges = 0, bool isGraph = true) const {
    assert(isGraph);  // TODO: we can extend this to non-graph but not trivially - would have to use similar
    // approach as Hypergraph/Level
    StateId start = hg.start();
    SDL_DEBUG(Hypergraph.InArcs, "graph w/ out-arcs: start=" << start);
    if (start == kNoState) return 0;
    StateId nStatesForOrder = hg.sizeForHeads();
    return isMutable ? orderTailsLast<Context>(order, start, nStatesForOrder, maxBackEdges)
                     : orderTailsLast<SlowContext>(order, start, nStatesForOrder, maxBackEdges);
  }

 private:
  ArcsContainer tmpOutArcs, emptyOutArcs;
};

/// reverse topological order on tail->head. \return #back edges
template <class Arc>
std::size_t orderTailsLast(IHypergraph<Arc> const& hg, StateOrder& order, std::size_t maxBackEdges = 0,
                           bool isGraph = true) {
  FirstTailOutArcs<Arc> adj(hg);
  return adj.orderTailsLast(order, maxBackEdges, isGraph);
}

template <class Arc, class StateOrder>
std::size_t orderHeadsLast(IHypergraph<Arc> const& hg, StateOrder& order, std::size_t maxBackEdges = 0,
                           bool isGraph = true) {
  InArcs<Arc> adj(hg);
  return adj.orderHeadsLast(order, maxBackEdges, isGraph);
}


}}

#endif
