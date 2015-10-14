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

 tree: nonempty, inarcs = 1 per state starting from final, until leaves

 *simple* tree: leaves are all lexical (no epsilon or other special terminals),
 *interior nodes are all nonterminal (must be labeled even if just by X)

 could also return #nonterminals, depth, #nodes at same time as checking tree-ness
*/

#ifndef SIMPLETREE_GRAEHL_2015_10_13_HPP
#define SIMPLETREE_GRAEHL_2015_10_13_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/SortStates.hpp> //TODO: in-place HypergraphBase-only SortStates.

namespace sdl { namespace Hypergraph {

bool isTree(HypergraphBase const& hg, StateId root);

bool isTree(HypergraphBase const& hg);

/// may force inarcs. post: \return isTree(hg)
bool forceTree(HypergraphBase & hg);


/// post: numInterior = # of interior tree nodes if \return true, and
/// interiorStatesFirst iff the interior states are number 0...numInterior-1
bool isSimpleTree(bool &interiorStatesFirst, HypergraphBase const& hg, StateId &numInterior);

/// may force inarcs and reorder states. post: \return isSimpleTree(hg),
/// numInterior, forceInterorStatesFirst and forceInterorStatesFirst is forced
/// to what you specified
template <class A>
bool forceSimpleTree(bool forceInterorStatesFirst, IHypergraph<A> & hg, StateId &numInterior) {
  bool interiorStatesFirst;
  if (!isSimpleTree(interiorStatesFirst, hg, numInterior)) return false;
  if (!forceInterorStatesFirst) return true;
  if (!hg.isMutable()) return false;
  sortStates(static_cast<IMutableHypergraph<A> &>(hg), SortStatesOptions(kTerminalLast));
  return true;
}


}}

#endif
