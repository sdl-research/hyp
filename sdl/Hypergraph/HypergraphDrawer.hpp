





#ifndef HYPERGRAPH_HYPERGRAPHDRAWER_HPP
#define HYPERGRAPH_HYPERGRAPHDRAWER_HPP






namespace Hypergraph {

/**

*/
struct DrawArcFct {

  DrawArcFct(std::ostream& out_, std::size_t firstNodeId_)
      : out(out_), nodeId(firstNodeId_) {}

  template<class Arc>

    typedef typename Arc::Weight Weight;
    if (arc->getNumTails() == 1) { // just one tail: no aux node needed
      out << arc->getTail(0) << " -> " << arc->head() << " [label=\"";
      if (arc->weight() != Weight::one())
        out << arc->weight();
      out << "\" fontsize = 12]\n";
      return;
    }

    // Draw auxiliary node for the hyperarc
    out << nodeId << " [label = \"\", shape = circle, style=solid, width = 0]"


    // Draw tails into aux arc
    std::size_t cnt = 1;

      out << sid << " -> " << nodeId << " [label = \""<< cnt
          <<"\", arrowhead = none, fontcolor = gray, fontsize = 9]"

      ++cnt;
    }

    // Draw aux arc to head
    out << nodeId << " -> " << arc->head() << " [label=\"";
    if (arc->weight() != Weight::one())
      out << arc->weight();
    out << "\" fontsize=12]\n";

    ++nodeId;
  }

  std::ostream& out;

};

bool writeStateLabel(std::ostream& out,
                     Hypergraph::StateId stateId,


  if (!pVoc || symId == NoSymbol) {
    out << stateId;
  }
  else {

    if (symId.isTerminal()) {
      out << "\\\"" << sym << "\\\"";
    }
    else {
      out << sym;
    }
  }
  return true;
}

template<class A>
std::ostream& drawHypergraph(std::ostream& out,


  // Begin





      << "  ranksep = 0.25;\n"
      << "  nodesep = 0.1;\n"
      << "  bgcolor = \"transparent\";\n"
      << "  node [margin=0.01];\n";

  // States

  std::size_t maxSid = 0;



    out << sid << " [";
    if (inId.isTerminal()) {
      out << "fontcolor=blue,";
    }
    out << "label = \"";
    writeStateLabel(out, sid, inId, pVoc);
    if (inId != outId) {
      out << " ";
      writeStateLabel(out, sid, outId, pVoc);
    }
    out << "\", shape = "

        << ", width = .5, fontsize = 12";
    if (hg.numInArcs(sid) == 0)       // no in arcs = no need to derive = 'observed'
      out << ", style=filled, fillcolor=\"#E6E6E6\""; // => filled grey like in graphical models
    else
      out << ", style=solid";
    if (sid == hg.start())
      out << ", penwidth=2";
    out << "]\n";
    maxSid = sid;
  }

  // Arcs


  // End


  return out;
}




#endif
