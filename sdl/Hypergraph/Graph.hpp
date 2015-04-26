// Copyright 2014 SDL plc
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

    build subgraph of Fsm with just epsilon arcs (or phi arcs). no weights yet. for determinize.
*/

#ifndef HYP__GRAPH_HPP
#define HYP__GRAPH_HPP
#pragma once


#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <algorithm>

namespace sdl {
namespace Hypergraph {

template <class SpecialSymbol>
struct isSpecial {
  bool operator()(Sym s) const {
    return s==SpecialSymbol::ID;
  }
};

struct isEps {
  bool operator()(Sym s) const {
    return s==EPSILON::ID;
  }
};

struct isRho {
  bool operator()(Sym s) const {
    return s==RHO::ID;
  }
};

struct isPhi {
  bool operator()(Sym s) const {
    return s==PHI::ID;
  }
};

struct isSigma {
  bool operator()(Sym s) const {
    return s==SIGMA::ID;
  }
};

struct Closure {
  bool converged;
  Closure() : converged(false) {  }
  typedef std::vector<StateId> Reach;
  typedef std::vector<Reach> ReachLen;
  ReachLen reachlen; // reachlen[i] = reachable states via paths of len <=i
  Reach const& get_reach(unsigned i) const {
    return i < reachlen.size() ? reachlen[i] : reachlen.back();
  }
};

// alternative: DFS from every t to build reachable set. bit operations should make this competitive
template <class Adjs> // e.g. vector<BitSet>
void warshall_transitive_close(Adjs &adjs) {
  typedef unsigned I;
  I N = (I)adjs.size();
  for (I t = 0; t < N; ++t) {
    typename Adjs::value_type &a = adjs[t];
    for (I h = 0; h<N; ++h)
      if (Util::test(a, h)) { // t->h
        adjs[t] |= adjs[h];
      }

  }
}

/**
   (unweighted) Graph version of a Hypergraph-fsm
*/
struct Graph {
  typedef StateId Edge;
  typedef unordered_set<Edge> Out;
  typedef std::vector<Out> Outs;
  typedef StateSet Adj;
  typedef std::vector<Adj> Adjs;

  Outs outs;
  typedef unsigned I;
  I size() const { // # vecs
    return (I)outs.size();
  }

  void fromAdjs(Adjs const& as) {
    I N = (I)as.size();
    outs.clear();
    outs.resize(N);
    for (I i = 0; i<N; ++i)
      Util::copyBits(as[i], outs[i]);
  }

  void toAdjs(Adjs &as) {
    I N = size();
    as.resize(N);
    for (I i = 0; i<N; ++i) {
      Adj &a = as[i];
      a.resize(N);
      Util::setBits(a, outs[i]);
    }
  }

  void transitiveClose() {
    Adjs as;
    toAdjs(as);
    warshall_transitive_close(as);
    fromAdjs(as);
  }

  template <class A, class LabelP>
  struct adder {
    Outs &outs;
    IHypergraph<A> const& hg;
    LabelP labelPred;
    adder(Outs &outs, IHypergraph<A> const& hg, LabelP const& labelPred) : outs(outs), hg(hg), labelPred(labelPred) {}
    adder(adder const& o) : outs(o.outs), hg(o.hg), labelPred(o.labelPred) {}
    void operator()(A * a) const {
      arc(*a);
    }
    void arc(A const& a) const {
      if (labelPred(hg.getFsmInput(a)))
        Util::add(outs[a.tails()[0]], Edge(a.head()));
    }
  };
  template <class A, class LabelP>
  void addFsmArcs(IHypergraph<A> const& hg, LabelP const& labelPred) {
    outs.resize(hg.size());
    hg.forArcsFsm(adder<A, LabelP>(outs, hg, labelPred));
  }

  template <class Out>
  void print(Out &o) const {
    for (unsigned i = 0; i<outs.size(); ++i) {
      o << i<<" ->";
      forall (StateId d, outs[i])
          o<<' '<<d;
      o << '\n';
    }
  }
  inline friend std::ostream& operator<<(std::ostream &o, Graph const& g) { g.print(o); return o; }
};


}}

#endif
