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

  viterbi (VHG/FHG-appropriate) inside costs.

  TODO: allow !isGraph and allow cycles (see AcyclicBest, BestPath)
*/

#ifndef INSIDECOSTS_JG_2015_06_14_HPP
#define INSIDECOSTS_JG_2015_06_14_HPP
#pragma once

#include <sdl/Util/Delete.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Util/MinMax.hpp>

namespace sdl {
namespace Hypergraph {

typedef std::vector<StateId> ReverseOrder;

template <class Arc>
void reverseTopologicalOutArcsGraph(IHypergraph<Arc> const& hg, StateId s, StateId*& out, char* color, StateId N) {
  StateIdContainer adj;
  hg.outAdjStates(s, adj);
  for (StateIdContainer::const_iterator i = adj.begin(), e = adj.end(); i != e; ++i) {
    StateId t = *i;
    if (!color[t]) {
      color[t] = 1;
      reverseTopologicalOutArcsGraph(hg, t, out, color, N);
    } else if (color[t] == 1)
      SDL_WARN(InsideCosts, "back edge (cycle) "
                                << t << " => " << s
                                << " - inside cost will be incorrect. TODO: use BestPath in these cases");
  }
  color[s] = 2;
  *out++ = s;
}

template <class Arc>
void insideCosts(IHypergraph<Arc> const& hg, SdlFloat* inside, StateId N) {
  if (!N) return;
  if (!hg.storesOutArcs())
    SDL_THROW_LOG(InsideCosts, ConfigException, "can't get insideCosts without out-arcs");
  if (!hg.isGraph())
    SDL_THROW_LOG(
        InsideCosts, UnimplementedException,
        "TODO: support non-graph for insideCosts (see PruneToBest for approach for with-cycle CFG)");
  Util::AutoDeleteArray<StateId> revorder(N);
  StateId *b = &revorder[0], *e = b;
  char* color = (char*)inside;
  std::memset(color, 0, N);
  reverseTopologicalOutArcsGraph(hg, hg.start(), e, color, N);
  std::fill(inside, inside + N, (float)HUGE_VAL);
  if (hg.isMutable()) {
    IMutableHypergraph<Arc> const& mhg = static_cast<IMutableHypergraph<Arc> const&>(hg);
    for (;;) {
      if (e == b) break;
      StateId s = *--e;
      SdlFloat c = hg.isAxiom(s) ? 0 : inside[s];
      ArcsContainer const* arcs = mhg.maybeOutArcs(s);
      if (!arcs) continue;
      for (ArcsContainer::const_iterator i = arcs->begin(), e = arcs->end(); i != e; ++i) {
        Arc* a = (Arc*)*i;
        assert(a->head_ < N);
        Util::minEq(inside[a->head_], a->weight_.value_ + c);
      }
    }
  } else {
    for (;;) {
      if (--e == b) break;
      StateId s = *e;
      SdlFloat c = hg.isAxiom(s) ? 0 : inside[s];
      ArcsContainer arcs;
      hg.outArcs(s, arcs);
      for (ArcsContainer::const_iterator i = arcs.begin(), e = arcs.end(); i != e; ++i) {
        Arc* a = (Arc*)*i;
        assert(a->head_ < N);
        Util::minEq(inside[a->head_], a->weight_.value_ + c);
      }
    }
  }
  SDL_TRACE(InsideCosts, "inside:  " << Util::arrayPrintable(&inside[0], N, true));
}


}}

#endif
