





#ifndef HYPERGRAPH_HYPERGRAPHDRAWER_HPP
#define HYPERGRAPH_HYPERGRAPHDRAWER_HPP






namespace Hypergraph {

/**

*/
struct DrawArcFct {

  DrawArcFct(std::ostream& out_, std::size_t firstNodeId_)
      : out(out_), nodeId(firstNodeId_) {}

  template<class Arc>










    // Draw auxiliary node for the hyperarc



    // Draw tails into aux arc
    std::size_t cnt = 1;

      out << sid << " -> " << nodeId << " [label = \""<< cnt


      ++cnt;
    }

    // Draw aux arc to head





    ++nodeId;
  }

  std::ostream& out;

};

bool writeStateLabel(std::ostream& out,





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










  // States

  std::size_t maxSid = 0;









    if (inId != outId) {
      out << " ";

    }
    out << "\", shape = "









    maxSid = sid;
  }

  // Arcs


  // End


  return out;
}




#endif
