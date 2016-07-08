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

    Count adjacent arcs even if a hg doesn't store that type of adjacency.
*/

#ifndef ADJACENCY_LW20111227_HPP
#define ADJACENCY_LW20111227_HPP
#pragma once

#include <sdl/Hypergraph/HypergraphBase.hpp>

namespace sdl {
namespace Hypergraph {

inline ArcId countInArcs(HypergraphBase const& h, StateId s) {
  if (h.storesInArcs()) return h.numInArcs(s);
  ArcId n = 0;
  // TODO: test
  h.forArcs([&n, s](ArcBase const* a) {
    if (a->head_ == s) ++n;
  });
  return n;
}

/// return # of first-tail outarcs or outarcs (not saying which). i.e. call graph states
inline ArcId countOutArcs(HypergraphBase const& h, StateId s) {
  if (h.storesOutArcs()) return h.numOutArcs(s);
  // TODO: test
  ArcId n = 0;
  h.forArcs([&n, s](ArcBase const* a) {
    if (!a->tails_.empty() && a->tails_[0] == s) ++n;
  });
  return n;
}


}}

#endif
