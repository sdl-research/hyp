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

 tree: inarcs = 1 per state starting from final, until leaves

 *simple* tree: tree where nonlexical nonleaf states come first, leaves are all lexical
*/

#include <sdl/Hypergraph/SimpleTree.hpp>
#include <sdl/Util/MinMax.hpp>

namespace sdl {
namespace Hypergraph {

bool isTree(HypergraphBase const& hg, StateId root) {
  assert(hg.storesInArcs());
  StateId n = hg.numInArcs(root);
  if (!n) return true;
  if (n > 1) return false;
  for (StateId c : hg.inArc(root, 0)->tails_)
    if (!isTree(hg, c)) return false;
  return true;
}

bool isTreeImpl(HypergraphBase const& hg) {
  StateId f = hg.final();
  return f != kNoState && isTree(hg, f);
}


bool isTree(HypergraphBase const& hg) {
  return hg.storesInArcs() && isTreeImpl(hg);
}

/// may force inarcs. post: \return isTree(hg)
bool forceTree(HypergraphBase& hg) {
  hg.forceInArcs();
  return isTreeImpl(hg);
}

bool isSimpleTreeRec(HypergraphBase const& hg, StateId root, StateId& maxInterior, StateId& numInterior) {
  assert(hg.storesInArcs());
  StateId n = hg.numInArcs(root);
  Sym label = hg.inputLabel(root);
  if (!n) return label.isLexical();
  if (label && !label.isNonterminal()) return false;
  ++numInterior;
  Util::maxEq(maxInterior, root);
  if (n > 1) return false;
  for (StateId c : hg.inArc(root, 0)->tails_)
    if (!isSimpleTreeRec(hg, c, maxInterior, numInterior)) return false;
  return true;
}

bool isSimpleTree(bool& interiorStatesFirst, HypergraphBase const& hg, StateId& numInterior) {
  if (!hg.storesInArcs()) return false;
  numInterior = 0;
  StateId f = hg.final();
  if (f == kNoState) return false;
  StateId maxInterior = 0;
  if (!isSimpleTreeRec(hg, f, maxInterior, numInterior)) return false;
  interiorStatesFirst = maxInterior == numInterior-1;
  return true;
}


}}
