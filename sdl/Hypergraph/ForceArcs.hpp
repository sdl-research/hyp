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

    options for making sure we have the preferred storage of arcs/labels.
*/

#ifndef FORCEARCS_JG_2014_01_04_HPP
#define FORCEARCS_JG_2014_01_04_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>

namespace sdl {
namespace Hypergraph {

struct ForceArcs {
  bool inArcs;
  bool graphOutArcs;
  bool outArcs;
  bool canonicalLex;
  ForceArcs()
      : inArcs()
      , graphOutArcs()
      , outArcs()
      , canonicalLex(false)  // TODO: change -> true (but will change many regtest outputs)
  {}

  template <class Config>
  void configure(Config& config) {
    config.is("ForceArcs");
    config("force arc adjacencies");
    config("in-arcs", &inArcs).defaulted()("index arcs by head (in-arcs)");
    config("graph-out-arcs", &graphOutArcs)
        .defaulted()("index arcs by first tail only - for graph/fsm (first-tail-out-arcs)");
    config("out-arcs", &outArcs).defaulted()("index arcs by all tails (out-arcs)");
    config("canonical-lex", &canonicalLex).defaulted()("reuse lexical states");
  }

  friend inline void validate(ForceArcs& x) { x.validate(); }

  void validate() {
    if (graphOutArcs && outArcs)
      SDL_WARN(ForceArcs, "graph-out-arcs and out-arcs are redundant - using out-arcs");
  }

  Properties canonicalProperty() const { return canonicalLex ? kCanonicalLex : 0; }

  Properties inArcsProperty() const { return inArcs ? kStoreInArcs : 0; }

  Properties outArcsProperty(Properties elseProperty = 0) const {
    return (outArcs ? kStoreOutArcs : graphOutArcs ? kStoreFirstTailOutArcs : elseProperty);
  }

  Properties arcPropertiesForced() const {
    return canonicalProperty() | inArcsProperty() | outArcsProperty(0);
  }

  Properties arcProperties() const {
    return canonicalProperty() | inArcsProperty() | outArcsProperty(kStoreInArcs);
  }

  template <class Arc>
  void forceArcs(IMutableHypergraph<Arc>& hg) const {
    if (inArcs) hg.forceInArcs();
    if (outArcs)
      hg.forceOutArcs();
    else if (graphOutArcs)
      hg.forceFirstTailOutArcs();
  }
};


}}

#endif
