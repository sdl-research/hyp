





#ifndef HYPERGRAPH_SORTARCS_HPP
#define HYPERGRAPH_SORTARCS_HPP



#include <boost/function.hpp>

#include <algorithm>





namespace Hypergraph {

struct CmpByWeight {



  }
};

struct EqualByWeight {
  template <class X>
  bool operator()(X const* a, X const* b) const {




template<class Arc>
struct CmpFsmArcInput {
  const IHypergraph<Arc>& fst;
  CmpFsmArcInput(const IHypergraph<Arc>& fst)
      : fst(fst) {}




  bool operator()(Arc const* a, Arc const* b) const {



  }
};








      : fst(fst) {}




  bool operator()(Arc const* a, Arc const* b) const {






template<class Arc>
struct CmpFsmArcHead {
  bool operator()(Arc const* a, Arc const* b) const{

  }
};

template<class Arc>


template<class Arc, class SortPolicy>
void sortArcsImpl(IMutableHypergraph<Arc>* hg, SortPolicy const& cmp){

  if (hg->hasSortedArcs()) return;

    typedef typename IMutableHypergraph<Arc>::ArcsContainer ArcsContainer;



  }
}

template<class Arc>


    pHg->forceProperties(kStoreFirstTailOutArcs);



}

template<class Arc, class SortPolicy>


}



#endif
