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

    cached bits indicating structural or data-structure-availability properties of a hg.
*/

#ifndef HYP__HYPERGRAPH_PROPERTIES_HPP
#define HYP__HYPERGRAPH_PROPERTIES_HPP
#pragma once

#include <sdl/IntTypes.hpp>
#include <boost/cstdint.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/graehl/shared/configure_named_bits.hpp>

namespace sdl {
namespace Hypergraph {

typedef uint64 PropertiesInt;

// TODO: make Properties configurable (implies a strong typedef)

/**
   a Graph in xmt is a left-recursive CFG. the set of all kGraph hypergraphs is
   a superset of the strictly binary kFsm. a hypergraph has property kGraph iff
   all its arcs are graph arcs. (kFsm implies kGraph)

   a graph arc has N tails: all but the first have a terminal label
   (special or lexical).

   note: an arc that has in the first tail a lexical (leaf) state is not a
   graph. graphs must have a defined start and final state or else they're
   considered empty.
   .
*/
const PropertiesInt kGraph = 0x001ULL;

// An FSM is a special case of a CFG, i.e., it is a left-recursive,
// binary CFG, where the 2nd tail of each arc is lexical.
const PropertiesInt kFsm = 0x002ULL;

// Does hypergraph store incoming arcs per state?
const PropertiesInt kStoreInArcs = 0x004ULL;

// Does hypergraph store outgoing arcs per state?
// (Only one of kStoreFirstTailOutArcs or kStoreOutArcs
// can be effective for now. May want to make them
// independent/orthogonal.)
/*
  discussion: someone could want:

  1. outarcs from first tail only
  2. outarcs from first tail and optionally other tails (don't care). this is fine for fsm since we'll never
  query outarcs of lexical
  3. outarcs from all tails

  there should be a way to specify what you want in a call to mutablehg::forceProperties or for optionally
  copying in Transform

  right now you can only do 1 or 3. but testing 1||3 is easy: hg.storesOutArcs()

  would have a new kStoreJustFirstTailOutArcs for 3 and kStoreFirstTailOutArcs would be for 2.
*/
const PropertiesInt kStoreOutArcs = 0x008ULL;
// only one of these two will be set
const PropertiesInt kStoreFirstTailOutArcs = 0x10ULL;

// Does Hypergraph have one or more states that have an output label
// (i.e., it's a transducer)?
const PropertiesInt kHasOutputLabels = 0x020ULL;

// TODO: rename kSortedOutArcs to kOutArcsSortedByInputLabel, and add
// kOutArcsSortedByOutputLabel (somtimes you want to sort by output label)
// TODO: This property is currently not automatically updated as arcs
// are added.
const PropertiesInt kSortedOutArcs = 0x040ULL;  // for compose.

/// triggers hg.addState(...) to use a single state per lexical in/out
/// label leaf. property may be set even though there are some
/// redundant same-LabelPair leaf states.
const PropertiesInt kCanonicalLex = 0x080ULL;
// TODO: make kCanonicalLex indicate policy, but have a hidden bool track
// invalidation automatically; right now we're removing this property whenever
// the cache is possibly invalidated.

/**
   mutually exclusive w/ kSortedOutArcs. lowest cost comes first
*/
const PropertiesInt kOutArcsSortedBestFirst = 0x100ULL;  // for compose.

const PropertiesInt kSortedStates = 0x200ULL;  // terminal-labeled states come last. further, if acyclic,
// non-terminal states are in topo order (all tails before any
// head they're connected to) (call sortStates() to activate
// this property)

const PropertiesInt kAcyclic
    = 0x400ULL;  // if set, it's definitely acyclic. otherwise, it still might be acyclic - call
// isAcyclic(hg)t to check (TODO: could track a separate bit for acyclic known - but usually
// we want to *require* acyclic). we could do something like checking fsm but that has
// dependency drawbacks (people who don't care about the implementations that check acyclic
// shouldn't have to compile). we could use a serial number w.r.t. modification of arcs (we
// have to trust people to not modify the heads/tails through Arc *, though they're welcome
// to modify the weight)

/// at most one lexical tail per state, with no terminal heads
const PropertiesInt kOneLexical = 0x800ULL;

/// have <xmt-blockN> symbols that will need to be preserved across transformations for later use
const PropertiesInt kAnnotations = 0x1000ULL;

/// have <xmt-blockN> symbols and each constraint id is on an arc <=> a fixed
/// state startBlock(id) is the graph source. implies kAnnotations
const PropertiesInt kConstraintStarts = 0x2000ULL;

/// have <xmt-blockN> symbols and each </xmt-block> for an id is on an arc <=> a
/// fixed state endBlock(id) is the graph source
const PropertiesInt kConstraintEnds = 0x4000ULL;

const PropertiesInt kPropertyEnd = kConstraintEnds << 1;
const PropertiesInt kAllProperties = kPropertyEnd - 1;

const PropertiesInt kAnyConstraints = kConstraintEnds | kConstraintStarts;

const PropertiesInt kFsmProperties = kFsm | kGraph | kOneLexical;

const PropertiesInt kDefaultProperties = kFsmProperties;

const PropertiesInt kFsmOutProperties = kStoreFirstTailOutArcs | kFsmProperties | kCanonicalLex;

const PropertiesInt kFullInOutArcs = kStoreInArcs | kStoreOutArcs;
const PropertiesInt kDefaultStoreArcsPerState = kFullInOutArcs;
// TODO: change all algorithms to be ok with kStoreFirstTailOutArcs

const PropertiesInt kStoresAnyOutArcs = kStoreOutArcs | kStoreFirstTailOutArcs;
const PropertiesInt kStoresAnyArcs = kStoreInArcs | kStoresAnyOutArcs;

inline PropertiesInt unknownPropertyBits(PropertiesInt p) {
  return p & ~kAllProperties;
}

struct PropertyNames {
  template <class Bits>
  static void bits(Bits& b) {
    b("graph", kGraph);
    b("fsm", kFsm);
    b("monolexical-arcs", kOneLexical);
    b("in-arcs", kStoreInArcs);
    b("out-arcs", kStoreOutArcs);
    b("first-tail-out-arcs", kStoreFirstTailOutArcs);
    b("output-labels", kHasOutputLabels);
    b("input-label-sorted-arcs", kSortedOutArcs);
    b("best-first-arcs", kOutArcsSortedBestFirst);
    b("label-states-last", kSortedStates);
    b("acyclic", kAcyclic);
    b("shared-label-states", kCanonicalLex);
    b("constraint-starts", kConstraintStarts);
    b("constraint-ends", kConstraintEnds);
    // TODO: these names are better; update the kSortedStates etc. var names
  }
};

typedef graehl::named_bits<PropertyNames, PropertiesInt> PrintProperties;

typedef PropertiesInt Properties;

inline PrintProperties printProperties(PropertiesInt p) {
  return PrintProperties(p);
}

inline bool hasProperties(Properties p, Properties required) {
  return (p & required) == required;
}

inline bool hasPropertiesOff(Properties p, Properties avoid) {
  return (p & avoid) == 0;
}

inline bool hasProperties(Properties p, Properties requiredOnOrOff, bool on) {
  return on ? hasProperties(p, requiredOnOrOff) : hasPropertiesOff(p, requiredOnOrOff);
}

inline Properties withArcs(Properties p, Properties arcTypeIfNoArcs = kStoreInArcs) {
  return (p & kStoresAnyArcs) ? p : (p | arcTypeIfNoArcs);
}


}}

#endif
