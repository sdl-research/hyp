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

    for PruneToBest, compute outside costs (even if hg has cycles - unlike InsideAlgorithm/OutsideAlgorithm)

    can stop before traversing parts of graph that are going to be pruned (onlyCostsBelow)
*/

#ifndef OUTSIDECOSTS_JG_2015_06_04_HPP
#define OUTSIDECOSTS_JG_2015_06_04_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/graehl/shared/priority_queue.hpp>
#include <sdl/Util/MinMax.hpp>
#include <sdl/Util/BitSet.hpp>

namespace sdl {
namespace Hypergraph {

struct QueueDistance {};

template <class Arc, class InsideCost>
void outsideCosts(IHypergraph<Arc> const& hg, SdlFloat* outside, InsideCost const& inside,
                  StateId N = kNoState, SdlFloat onlyCostsBelow = HUGE_VAL) {
  if (N == kNoState) N = hg.sizeForHeads();
  SDL_TRACE(OutsideCosts, "N="<<N<<" onlyCostsBelow="<<onlyCostsBelow);
  typedef std::vector<StateId> OutsidePlan;
  OutsidePlan outsidePlan;
  typedef std::pair<SdlFloat, StateId> S;
  typedef std::vector<S> Q;
  graehl::priority_queue<Q> q;
  S s(S(0, hg.final()));
  outside[s.second] = s.first;
  q.push(s);
  Util::BitSet popped(N);
  for (;;) {
    s = q.top();
    q.pop();
    if (Util::latch(popped, s.second) || s.first < outside[s.second]) {
      // might loop infinitely if negative cost effective cycle.
      SDL_TRACE(OutsideCosts, "reaching from head "<<s.second<<" cost="<<s.first);
      for (ArcId ai = 0, ae = hg.numInArcs(s.second); ai < ae; ++ai) {
        Arc* a = hg.inArc(s.second, ai);
        StateIdContainer const& tails = a->tails_;
        StateIdContainer::const_iterator b = tails.begin(), e = tails.end(), i;
        SdlFloat c = s.first + a->weight_.value_;
        for (i = b; i != e; ++i) {
          StateId t = *i;
          if (t < N) {
            if (is_null(inside[t]))
              goto nexta;
            c += inside[t];
          }
        }
        if (c < onlyCostsBelow) {
          for (i = b; i != e; ++i) {
            StateId t = *i;
            SdlFloat& ot = outside[t];
            if (t < N) {
              SdlFloat ot2 = c - inside[t];
              if (ot2 < ot) {
                q.push(S(ot2, t));
                ot = ot2;
              }
            } else
              Util::minEq(ot, c);
          }
        }
      }
     nexta: ;
    }
    if (q.empty()) break;
  }
}

template <class A, class OutsideCost, class InsideCost>
void outsideCostsResize(IHypergraph<A> const& hg, OutsideCost& out, InsideCost const& in,
                        StateId N = kNoState, SdlFloat onlyCostsBelow = HUGE_VAL) {
  out.resize(N == kNoState ? hg.sizeForHeads() : N, (SdlFloat)HUGE_VAL);
  outsideCosts(hg, &out[0], in, N, onlyCostsBelow);
}


}}

#endif
