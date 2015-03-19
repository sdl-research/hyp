/** \file

    options for making sure we have the preferred storage of arcs/labels.
*/

#ifndef FORCEARCS_JG_2014_01_04_HPP
#define FORCEARCS_JG_2014_01_04_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>

namespace sdl { namespace Hypergraph {

struct ForceArcs {
  bool inArcs;
  bool graphOutArcs;
  bool outArcs;
  bool canonicalLex;
  ForceArcs()
      : inArcs()
      , graphOutArcs()
      , outArcs()
      , canonicalLex(false) //TODO: change -> true (but will change many regtest outputs)
  {}

  template <class Config>
  void configure(Config &config) {
    config.is("ForceArcs");
    config("force arc adjacencies");
    config("in-arcs", &inArcs).self_init()
        ("index arcs by head (in-arcs)");
    config("graph-out-arcs", &graphOutArcs).self_init()
        ("index arcs by first tail only - for graph/fsm (first-tail-out-arcs)");
    config("out-arcs", &outArcs).self_init()
        ("index arcs by all tails (out-arcs)");
    config("canonical-lex", &canonicalLex).self_init()
        ("reuse lexical states");
  }

  friend inline void validate(ForceArcs & x) {
    x.validate();
  }

  void validate() {
    if (graphOutArcs && outArcs)
      SDL_WARN(ForceArcs, "graph-out-arcs and out-arcs are redundant - using out-arcs");
  }

  Properties canonicalProperty() const {
    return canonicalLex ? kCanonicalLex : 0;
  }

  Properties inArcsProperty() const {
    return inArcs ? kStoreInArcs : 0;
  }

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
  void forceArcs(IMutableHypergraph<Arc> &hg) const {
    if (inArcs)
      hg.forceInArcs();
    if (outArcs)
      hg.forceOutArcs();
    else if (graphOutArcs)
      hg.forceFirstTailOutArcs();
  }
};


}}

#endif
