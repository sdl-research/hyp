





#ifndef HYPERGRAPH_SORTARCS_HPP
#define HYPERGRAPH_SORTARCS_HPP





#include <algorithm>





namespace Hypergraph {
















struct CmpFsmArcInput {
  const IHypergraph<Arc>& fst;
  CmpFsmArcInput(const IHypergraph<Arc>& fst)
      : fst(fst) {}








  }




























template<class Arc>











  }
}

















#endif
