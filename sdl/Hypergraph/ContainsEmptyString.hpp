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

    Is there a path from start to final with no lexical labels?
*/

#ifndef CONTAINSEMPTYSTRING_JG_2014_09_24_HPP
#define CONTAINSEMPTYSTRING_JG_2014_09_24_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/GraphArcs.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Hypergraph {

inline bool isLexical(HypergraphBase const& hg, ArcBase const& arc) {
  return hg.firstLexicalTail(&arc) != (TailId)-1;
}

template <class Adjacency>
bool containsEmptyString(HypergraphBase const& hg, Adjacency const& adj, StateId from, StateId goal, StateSet &once) {
  if (once.test_set(from)) return false;
  // check once in any order (as opposed to toposort) is ok because we're not
  // trying to get best path, just detect any path to goal (which causes
  // immediate return-true cascade once reached)
  if (from == goal) return true;
  for (ArcId i = 0, n = adj.getNumArcs(hg, from); i < n; ++i) {
    ArcBase* a = adj.getArc(hg, from, i);
    if (!isLexical(hg, *a) && containsEmptyString(hg, adj, Adjacency::to(a), goal, once)) return true;
  }
  return false;
}

struct GraphInArcs;

inline bool containsEmptyString(HypergraphBase const& hg) {
  if (!hg.isGraph())
    SDL_THROW_LOG(Hypergraph.containsEmptyString, UnimplementedException,
                  "not implemented (yet) - containsEmptyString for CFG"
                  " (easy using bottom-up Empty.hpp but only using not-lexical arcs");
  StateId final = hg.final();
  if (final == kNoState) return false;
  StateId start = hg.start();
  if (start == kNoState) return false;
  StateSet once(hg.sizeForHeads());
  if (hg.storesInArcs())
    return containsEmptyString(hg, GraphInArcs(), final, start, once);
  else
    return containsEmptyString(hg, GraphFirstTailOutArcsCached(hg), start, final, once);
}


}}

#endif
