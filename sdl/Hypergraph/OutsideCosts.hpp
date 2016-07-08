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
#include <sdl/Util/BitSet.hpp>
#include <sdl/Util/MinMax.hpp>
#include <graehl/shared/priority_queue.hpp>

namespace sdl {
namespace Hypergraph {

struct QueueDistance {};

template <class Arc, class InsideCost>
void outsideCosts(IHypergraph<Arc> const& hg, SdlFloat* outside, InsideCost const& inside,
                  StateId N = kNoState, SdlFloat onlyCostsBelow = HUGE_VAL, bool insideHasAxioms = true) {
  forceInArcs(const_cast<IHypergraph<Arc>&>(hg), "outsideCosts");

  if (N == kNoState) N = hg.sizeForHeads();
  SDL_TRACE(OutsideCosts, "N=" << N << " onlyCostsBelow=" << onlyCostsBelow
                               << " insideHasAxioms=" << insideHasAxioms);
  typedef std::vector<StateId> OutsidePlan;
  OutsidePlan outsidePlan;
  typedef std::pair<SdlFloat, StateId> S;
  typedef std::vector<S> Q;
  graehl::priority_queue<Q> q;
  S s(S(0, hg.final()));
  if (s.second == kNoState) return;
  outside[s.second] = s.first;
  q.push(s);
  Util::BitSet popped(N);
  StateIdContainer nonaxioms;
  for (;;) {
    s = q.top();
    q.pop();
    StateId head = s.second;
    if (Util::latch(popped, head)) {
      // prevent infinite loop if cycles (compromise accuracy on
      // effective-negative-cost outside edges)
      SDL_TRACE(OutsideCosts, "reaching from head " << head << " cost=" << s.first);
      for (ArcId ai = 0, ae = hg.numInArcs(head); ai < ae; ++ai) {
        Arc* a = hg.inArc(head, ai);
        StateIdContainer const& tails = a->tails_;
        StateIdContainer::const_iterator b = tails.begin(), e = tails.end(), i;
        SdlFloat c = s.first + a->weight_.value_;
        if (!insideHasAxioms) nonaxioms.clear();
        for (i = b; i != e; ++i) {
          StateId t = *i;
          if (t < N) {
            if (insideHasAxioms || !hg.isAxiom(t)) {
              SdlFloat it = inside[t];
              if (is_null(it)) goto nexta;
              c += it;
              if (!insideHasAxioms) nonaxioms.push_back(t);
            }
          }
        }
        if (c < onlyCostsBelow) {
          if (insideHasAxioms)
            i = b;
          else {
            i = nonaxioms.begin();
            e = nonaxioms.end();
          }
          for (; i != e; ++i) {
            StateId t = *i;
            SdlFloat ot2 = c;
            if (t < N) {
              SdlFloat& ot = outside[t];
              ot2 -= inside[t];
              if (ot2 < ot) {
                q.push(S(ot2, t));
                SDL_TRACE(OutsideCosts, "reached top-down " << t << " outside=" << ot2);
                ot = ot2;
              } else
                Util::minEq(ot, c);
            }
          }
        } else {
          SDL_TRACE(OutsideCosts, "pruned " << c << " >= " << onlyCostsBelow);
        }
      }
    nexta:;
    }
    if (q.empty()) break;
  }
  SDL_TRACE(OutsideCosts, "outside: " << Util::arrayPrintable(outside, N, true));
}


}}

#endif
