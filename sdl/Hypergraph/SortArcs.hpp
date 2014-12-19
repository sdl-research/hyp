





#ifndef HYPERGRAPH_SORTARCS_HPP
#define HYPERGRAPH_SORTARCS_HPP



#include <boost/function.hpp>

#include <algorithm>





namespace Hypergraph {
















struct CmpFsmArcInput {
  const IHypergraph<Arc>& fst;
  CmpFsmArcInput(const IHypergraph<Arc>& fst)
      : fst(fst) {}








  }






















struct CmpFsmArcHead {
  bool operator()(Arc const* a, Arc const* b) const{

  }
};

template<class Arc>


template<class Arc, class SortPolicy>
void sortArcsImpl(IMutableHypergraph<Arc>* hg, SortPolicy const& cmp){







  }
}

template<class Arc>






}

template<class Arc, class SortPolicy>


}



#endif
