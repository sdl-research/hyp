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

    modify arc properties w/ lexical scoping (i.e. put them back when a local
    object is destroyed)
*/

#ifndef RESTOREARCPROPERTIES_JG_2013_12_15_HPP
#define RESTOREARCPROPERTIES_JG_2013_12_15_HPP
#pragma once

#include <sdl/Hypergraph/MutableHypergraph.hpp>

namespace sdl {
namespace Hypergraph {

struct RestoreArcPropertiesOptions {
  RestoreArcPropertiesOptions() {}
  RestoreArcPropertiesOptions(bool restoreArcProperties) : restoreArcProperties(restoreArcProperties) {}
  bool restoreArcProperties = false;  // TODO: was true (needs test)
  template <class Config>
  void configure(Config& config) {
    config("restore-arc-properties", &restoreArcProperties)
        .defaulted()("restore exactly the same arc-state adjacency indices (e.g. heads or tails) on exit");
  }
};

RestoreArcPropertiesOptions const kNoRestoreArcProperties((false));
RestoreArcPropertiesOptions const kRestoreArcProperties((true));

/**
   temporarily make sure arcs are stored how you want, then put them back after
   if restore (but does nothing if bool restoreOldArcProperties is false)
*/
template <class Arc>
struct RestoreArcProperties {
  RestoreArcProperties(IMutableHypergraph<Arc>& hg, RestoreArcPropertiesOptions options = kRestoreArcProperties)
      : hg(hg)
      , restoreArcProperties(options.restoreArcProperties)
      , prop(hg.properties())
      , graph(prop & kGraph)
      , hadInArcs(prop & kStoreInArcs)
      , hadOutArcs(prop & kStoreOutArcs)
      , hadFirstTailOutArcs(prop & kStoreFirstTailOutArcs) {}

  ~RestoreArcProperties() {
    if (restoreArcProperties) {
      Properties const propNow = hg.properties();
      if (hadInArcs && !(propNow & kStoreInArcs)) hg.forceInArcs();
      if (hadOutArcs && !(propNow & kStoreOutArcs))
        hg.forceOutArcs();
      else if (hadFirstTailOutArcs && !(propNow & kStoreFirstTailOutArcs))
        hg.forceFirstTailOutArcs();
    }
  }

  bool hadAnyOut() const { return hadFirstTailOutArcs || hadOutArcs; }

  IMutableHypergraph<Arc>& hg;
  bool const restoreArcProperties;
  Properties const prop;
  bool const graph;
  bool const hadInArcs;
  bool const hadOutArcs;
  bool const hadFirstTailOutArcs;

  void forceInArcsOnly() {
    hg.forceInArcs();
    if (hadOutArcs || hadFirstTailOutArcs) hg.removeOutArcs();
  }

  void forceFirstTailOutArcsOnly() {
    hg.forceFirstTailOutArcs();
    if (hadInArcs) hg.removeInArcs();
  }

  /**
     make at least one of first tail out, or in, arcs remain - for binarization,
     we'll modify an existing arc such that some of its non-first tails are no
     longer connected to it.
  */
  void removeOutArcsExceptFirstTail() {
    if (hadOutArcs) hg.removeOutArcsMaybeAddingFirstTail();
  }

  void modifyingLexicalTails() {
    if (graph && hadFirstTailOutArcs) {
      assert(!hadOutArcs);
    } else if (hadOutArcs || hadFirstTailOutArcs) {
      if (!hadInArcs) hg.forceInArcs();
      hg.removeOutArcs();
    }
  }
};

template <class Arc>
struct LocalArcPropertiesForModifyingLexicalTails : RestoreArcProperties<Arc> {
  LocalArcPropertiesForModifyingLexicalTails(IMutableHypergraph<Arc>& hg,
                                             RestoreArcPropertiesOptions options = kRestoreArcProperties)
      : RestoreArcProperties<Arc>(hg, options) {
    this->modifyingLexicalTails();
  }
};


}}

#endif
