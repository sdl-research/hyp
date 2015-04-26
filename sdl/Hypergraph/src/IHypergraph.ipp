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
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/Visit.hpp>

// templated IHypergraph functions

namespace sdl {
namespace Hypergraph {

template <class A>
typename IHypergraph<A>::Weight IHypergraph<A>::final(StateId s) const {
  // typedef typename A::Weight Weight;
  StateId f = this->final();
  if (s == f) return Weight::one();
  if (isFsm()) {
    assert(storesOutArcs());
    for (ArcId a = 0, f = numOutArcs(s); a < f; ++a) {
      Arc const& arc = *outArc(s, a);
      // TODO: binary search if sorted - if EPSILON::ID is minimum, reduces to checking first arc.
      if (arc.head() == f && inputLabel(arc.fsmSymbolState()) == EPSILON::ID) {
        return arc.weight();
      }
    }
  }
  return Weight::zero();
}

template <class A>
std::size_t IHypergraph<A>::getNumEdges() const {
  return countArcs().n;
}

template <class A>
bool IHypergraph<A>::hasAllOut(StateId s) const {
  assert(isFsm());
  assert(storesOutArcs());
  for (ArcId a = 0, f = numOutArcs(s); a < f; ++a) {
    Arc const& arc = *outArc(s, a);
    Sym s = inputLabel(arc.fsmSymbolState());
    if (s == RHO::ID || s == SIGMA::ID || s == PHI::ID || (s == EPSILON::ID && arc.head() != f)) return true;
  }
  return false;
}

template <class A>
bool IHypergraph<A>::checkValid() const {
  StateId N = size();
  StateId maxhead = maxHead(*this);
  assert(maxhead == kNoState || maxhead < N);
  StateId maxtail = maxTail(*this);
  assert(maxtail == kNoState || maxtail < N);
  StateId s = start(), f = final();
  assert(s == kNoState || s < N);
  assert(f == kNoState || f < N);
  return true;
}

template <class A>
LabelPair IHypergraph<A>::fsmLabelPair(Arc const& a) const {
  return labelPair(a.fsmSymbolState());
}

struct CheckFsm {
  template <class Arc>
  bool operator()(IHypergraph<Arc> const& hg, Arc* arc) const {
    return hg.isFsmArc(*arc);
  }
};

// note: an fsm missing either start or final state has no derivations (but is still an fsm)
template <class A>
bool IHypergraph<A>::isFsmCheck() const {
  return visitArcsAtLeastOnce(*this, CheckFsm());
}

struct CheckGraph {
  CheckGraph() : fsm(true), oneLexical(true) {}
  mutable bool fsm;
  mutable bool oneLexical;
  template <class Arc>
  bool operator()(IHypergraph<Arc> const& hg, Arc* arc) const {
    return hg.isGraphArc(*arc, fsm, oneLexical);
  }
};

// note: a graph missing either start or final state has no derivations (but is still a graph)
template <class A>
bool IHypergraph<A>::isGraphCheck(bool& fsm, bool& oneLexical) const {
  bool const mustBeCfg = start() == kNoState && final() != kNoState;
  // above check: because we require that kStart == kNoState iff the cfg is
  // empty, whenever kFsm or kGraph are set. but some CFG may accidentally be
  // left-recursive, so we don't wish to prune them if they have a final state.

  CheckGraph check;
  bool r = visitArcsAtLeastOnce(*this, check);
  oneLexical = check.oneLexical;
  if (mustBeCfg) return (fsm = false);
  fsm = check.fsm;
  return r;
}


}}
