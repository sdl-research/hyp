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

    topological ordering of states connected to final only (doesn't visit other
    roots in a forest-hypergraph)
*/

#ifndef HYP__HYPERGRAPH_STATESTRAVERSAL_HPP
#define HYP__HYPERGRAPH_STATESTRAVERSAL_HPP
#pragma once


#include <set>
#include <queue>
#include <stdexcept>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/Types.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/ShrinkVector.hpp>
#include <sdl/Hypergraph/InArcs.hpp>

namespace sdl {
namespace Hypergraph {

struct IStatesVisitor {
  virtual void visit(StateId s) = 0;
  virtual ~IStatesVisitor() {}
};

template <class Arc>
void visitStates(IHypergraph<Arc> const& hg, IStatesVisitor *visitor) {
  for (StateId s = 0, N = hg.size(); s<N; ++s)
    visitor->visit(s);
}


struct PrintStatesVisitor : public IStatesVisitor {
  PrintStatesVisitor(std::ostream& out = std::cout,
                     std::string const& separator = "\n")
      : out_(out), separator_(separator) {}

  void visit(StateId s) {
    out_ << s << separator_;
  }

  std::ostream& out_;
  std::string separator_;
};


/**
   Traverses all states (that are reachable from the root) in
   topological order, calling the visitor on every state.

   TODO: Ignores cycles. TODO: Throw error on cycle.
*/
template<class Arc>
class TopsortStatesTraversal {
  //GOTCHA: //TODO to the extent that you use this for inside computation on hgs, it's
  //wrong. but this is useful for graphs (not hypergraphs)

  //TODO@MD from JG: this in-arcs based head->tails dfs order isn't useful for
  //much except for (after reverse, what you need for outside) and to give you
  //an out-arcs based topo order of a graph without first needing to reverse the
  //result of a DFS. it's no good for acyclic HG - for that, see the approach i
  //use in Hypergraph/Empty and Hypergraph/Level.
 public:

  TopsortStatesTraversal(IHypergraph<Arc> const& hg, IStatesVisitor *visitor, StateId maxState = (StateId)-1)
      : hg_(hg)
      , nStates_(maxState == (StateId)-1 ? hg.size() : maxState + 1)
      , visited_(nStates_)
  {
    if (!hg_.storesInArcs()) {
      // MD: TODO: If incoming arcs are not stored we can still do it
      // efficiently for an FSA. But it's not as easy for a general hg (see
      // Level and BestPath - you need to count # of tails reached for each
      // arc).

      // (JG notes: actually, an FSA specific outarcs recursion gives you a way
      // to get the reverse without having to explicitly store+reverse, since
      // reverse(topo-order(graph)) is a topo-order(reverse(graph)).
      SDL_THROW_LOG(Hypergraph.StatesTraversal, ConfigException,
                    "TopsortStatesTraversal needs incoming arcs");
    }
    accept(visitor);
  }

 private:
  void accept(IStatesVisitor* visitor) {
    visitor_ = visitor;
    StateId final = hg_.final();
    if (final == Hypergraph::kNoState)
      return;
    assert(final < nStates_);
    traverse(final);
  }

  //TODO@MD from JG: this uses O(path-length) stack. might be bad for many threads and (pathological) large input
  void traverse(StateId head) {
    if (head < nStates_ && Util::latch(visited_, head)) {
      for (ArcId a = 0, N = hg_.numInArcs(head); a != N; ++a) {
        StateIdContainer const& tails = hg_.inArc(head, a)->tails();
        for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i!=e; ++i)
          traverse(*i);
      }
      visitor_->visit(head);
    }
  }
  IHypergraph<Arc> const& hg_;
  StateId nStates_;
  StateSet visited_;
  IStatesVisitor* visitor_;

};

/**
   Traverses all states (that are reachable from the root) in topological
   order on reversed head->tail graph, calling the visitor on every state.

   TODO: Ignores cycles. TODO: Throw error on cycle.
*/
template<class Arc>
class OutsideTopsortStatesTraversal {

 public:

  OutsideTopsortStatesTraversal(IHypergraph<Arc> const& hg, IStatesVisitor *visitor)
      : hg_(hg)
      , visited_(hg.size())
      , visitor_(visitor)
  {
    if (!hg_.storesOutArcs())
      SDL_THROW_LOG(Hypergraph.StatesTraversal, ConfigException,
                    "TopsortStatesTraversal needs out arcs");
    accept(visitor);
  }

 private:
  void accept(IStatesVisitor* visitor) {
    StateId start = hg_.start();
    if (start == Hypergraph::kNoState)
      return;
    traverse(start);
  }

  void traverse(StateId src) {
    if (Util::latch(visited_, src)) {
      for (ArcId a = 0, N = hg_.numOutArcs(src); a != N; ++a)
        traverse(hg_.outArc(src, a)->head());
      visitor_->visit(src);
    }
  }
  IHypergraph<Arc> const& hg_;
  StateSet visited_;
  IStatesVisitor* visitor_;
};


/**
   Traverses all states (that are reachable from the root) in reverse
   topological order. this used to have a wrong algorithm, but is now correct
   (using TopsortStatesTraversal then reversing the result)

   TODO: Ignores cycles. TODO: Throw error on cycle.

   TODO: makes inside+outside faster by making inside compute reversed order at the same time

   // this is the correct order for outside computation (hg or graph)
   */
template<class Arc>
class ReverseTopsortStatesTraversal : private IStatesVisitor {
  typedef TopsortStatesTraversal<Arc> Topsort;
  typedef std::vector<StateId> States;
  States order_;
 public:
  void visit(StateId s)
  {
    order_.push_back(s);
  }
  ReverseTopsortStatesTraversal(IHypergraph<Arc> const& hg, IStatesVisitor* visitor)
  {
    if (hg.isFsm() && hg.storesOutArcs())
      OutsideTopsortStatesTraversal<Arc>(hg, visitor);
    else {
      order_.reserve(hg.size());
      TopsortStatesTraversal<Arc>(hg, this);
      accept(visitor);
    }
  }
 private:
  void accept(IStatesVisitor* visitor) {
    for (States::const_reverse_iterator i = order_.rbegin(), e = order_.rend(); i!=e; ++i)
      visitor->visit(*i);
    Util::clearVector(order_);
  }
};

// certain to visit start state first if hg is acyclic. all states are visited
// only after those that can reach them are visited first. throw if more than
// maxBackEdges cycle-causing edges
template <class Arc, class Visitor>
void visitGraphOutArcsTopsort(IHypergraph<Arc> const& hg, Visitor &visitor, std::size_t maxBackEdges = 0) {
  std::vector<StateId> orderReverse;
  StateId N = hg.size();
  orderReverse.reserve(N);
  FirstTailOutArcs<Arc> adj(hg);
  std::size_t nBackEdges = adj.orderTailsLast(orderReverse, maxBackEdges, true);
  if (nBackEdges > maxBackEdges)
    SDL_THROW_LOG(StatesTraversal.visitGraphOutArcsTopsort, CycleException,
                  nBackEdges<<" cycle-causing edges; can't topologically sort");
  if (orderReverse.empty()) return;
  typedef StateId const* I;
  for (I i = &orderReverse.back(), last = &orderReverse.front(); ; ) {
    visitor.visit(*i);
    if (i == last) break;
    --i;
  }
}

template <class Arc, class Visitor>
void visitTopsort(IHypergraph<Arc> const& hg, Visitor &visitor) {
  if (hg.storesInArcs())
    TopsortStatesTraversal<Arc>(hg, &visitor);
  else
    visitGraphOutArcsTopsort(hg, visitor);
}

template <class Arc, class Visitor>
void visitTopsort(IHypergraph<Arc> const& hg, Visitor *visitor) {
  return visitTopsort(hg, *visitor);
}


}}

#endif
