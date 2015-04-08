// Copyright 2014 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    since most of these are trivial wrappers to existing IHypergraph
    virtual fns, they should be inline and not hidden in .cpp; on the other
    hand, since they're not virtual they could well be optional free fns instead
    of part of the IHypergraph interface.

    //TODO: make these inline
    */

#include <sdl/Hypergraph/IHypergraph.hpp>


namespace sdl {
namespace Hypergraph {

bool IHypergraphStates::hasSortedArcs() const {
  Properties p = this->properties();
  assert(storesOutArcs());
  return p & kSortedOutArcs;
}

bool IHypergraphStates::hasBestFirstArcs() const {
  Properties p = this->properties();
  assert(storesOutArcs());
  return p&kOutArcsSortedBestFirst;
}

bool IHypergraphStates::isMutable() const {
  return false;
}

// may still be empty even if final state exists (can't be reached
// from start/axioms). also, no longer checks #of arcs since that takes time
bool IHypergraphStates::prunedEmpty() const {
  StateId sfinal = final();
  if (sfinal == Hypergraph::kNoState) return true;
  StateId sstart = start();
  if (sfinal == sstart) return false;
  Properties p = properties();
  if (p & kStoreInArcs)
    return numInArcs(sfinal) == 0;
  else if ((p & kStoresAnyArcs) && sstart != Hypergraph::kNoState)
    return numOutArcs(sstart) == 0;
  return false;
}

LabelPair IHypergraphStates::labelPair(StateId s) const { // may be faster if overriden
  return LabelPair(inputLabel(s), outputLabel(s));
}


//TODO: allow setting of per-state axiom weights, or just enforce 0-tails arcs for axioms.

bool IHypergraphStates::isAxiom(StateId s) const {
  return s==start() || hasTerminalLabel(s);
}


bool IHypergraphStates::hasStart() const {
  return start() != Hypergraph::kNoState;
}

bool IHypergraphStates::hasFinal() const {
  return final() != Hypergraph::kNoState;
}

bool IHypergraphStates::storesArcs() const {
  return properties() & kStoresAnyArcs;
}

bool IHypergraphStates::storesInArcs() const {
  return properties() & kStoreInArcs;
}

bool IHypergraphStates::storesOutArcs() const {
  return properties() & kStoresAnyOutArcs;
}

bool IHypergraphStates::storesAllOutArcs() const {
  return properties() & kStoreOutArcs;
}

bool IHypergraphStates::storesFirstTailOutArcs() const {
  return properties() & kStoreFirstTailOutArcs;
}

bool IHypergraphStates::hasLabel(StateId id) const {
  return inputLabel(id)!=NoSymbol;
}

}}
