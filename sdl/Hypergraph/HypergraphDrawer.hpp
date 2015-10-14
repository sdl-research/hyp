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

    Graphviz 'dot'-renderable file from a hypergraph
*/


#ifndef HYPERGRAPH_HYPERGRAPHDRAWER_HPP
#define HYPERGRAPH_HYPERGRAPHDRAWER_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>


namespace sdl {
namespace Hypergraph {

/**
   Draws an arc (Graphviz dot format).
*/
template <class Arc>
struct DrawArcFct {

  DrawArcFct(std::ostream& out_, std::size_t firstNodeId_) : out(out_), nodeId(firstNodeId_) {}

  void operator()(Arc* arc) const {
    typedef typename Arc::Weight Weight;
    if (arc->getNumTails() == 1) {  // just one tail: no aux node needed
      out << arc->getTail(0) << " -> " << arc->head() << " [label=\"";
      if (arc->weight() != Weight::one()) out << arc->weight();
      out << "\" fontsize = 12]\n";
      return;
    }

    // Draw auxiliary node for the hyperarc
    out << nodeId << " [label = \"\", shape = circle, style=solid, width = 0]" << '\n';

    // Draw tails into aux arc
    std::size_t cnt = 1;
    for (StateId s : arc->tails()) {
      out << s << " -> " << nodeId << " [label = \"" << cnt
          << "\", arrowhead = none, fontcolor = gray55, fontsize = 10]" << '\n';
      ++cnt;
    }

    // Draw aux arc to head
    out << nodeId << " -> " << arc->head() << " [label=\"";
    if (arc->weight() != Weight::one()) out << arc->weight();
    out << "\" fontsize=12]\n";

    ++nodeId;
  }

  std::ostream& out;
  mutable std::size_t nodeId;
};

/**
   For dot tool, write Greek symbols as HTML entities
 */
std::string dotify(std::string const& sym) {
  if (sym.length() < 3 || !(*sym.begin() == '<' && *sym.rbegin() == '>')) return sym;
  if (sym == "<eps>") return "&epsilon;";
  if (sym == "<phi>") return "&phi;";
  if (sym == "<rho>") return "&rho;";
  if (sym == "<sigma>") return "&sigma;";
  return sym;
}

bool writeStateLabel(std::ostream& out, Hypergraph::StateId stateId, Sym symId, IVocabularyPtr pVoc) {
  if (!pVoc || symId == NoSymbol) {
    out << stateId;
  } else {
    std::string const& sym = pVoc->str(symId);
    if (symId.isTerminal() && !symId.isSpecial()) {
      out << "\\\"" << sym << "\\\"";
    } else {
      out << dotify(sym);
    }
  }
  return true;
}

template <class Arc>
std::ostream& drawHypergraph(std::ostream& out, IHypergraph<Arc> const& hg) {
  // Begin
  out << "digraph HG {\n"
      << "  rankdir = LR;\n"
      << "  label = \"\";\n"
      << "  center = 1;\n"
      << "  orientation = Portrait;\n"
      << "  ranksep = 0.25;\n"
      << "  nodesep = 0.1;\n"
      << "  bgcolor = \"transparent\";\n"
      << "  node [margin=0.01];\n";

  // States
  IVocabularyPtr pVoc = hg.getVocabulary();
  std::size_t maxS = 0;
  for (StateId s : hg.getStateIds()) {
    Sym inId = hg.inputLabel(s);
    Sym outId = hg.outputLabel(s);
    out << s << " [";
    if (inId.isTerminal()) {
      out << "fontcolor=blue,";
    }
    out << "label = \"";
    writeStateLabel(out, s, inId, pVoc);
    if (inId != outId) {
      out << " ";
      writeStateLabel(out, s, outId, pVoc);
    }
    out << "\", shape = " << (hg.final() == s ? "doublecircle" : "circle") << ", width = .5, fontsize = 12";
    if (hg.numInArcs(s) == 0)  // no in arcs = no need to derive = 'observed'
      out << ", style=filled, fillcolor=\"#E6E6E6\"";  // => filled grey like in graphical models
    else
      out << ", style=solid";
    if (s == hg.start()) out << ", penwidth=2";
    out << "]\n";
    maxS = s;
  }

  // Arcs
  hg.forArcs(DrawArcFct<Arc>(out, maxS + 1));

  // End
  out << "}\n";

  return out;
}


}}

#endif
