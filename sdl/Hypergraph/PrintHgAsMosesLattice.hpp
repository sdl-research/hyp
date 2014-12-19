






#include <deque>
#include <boost/unordered_map.hpp>






// output the lattice in plf format with arc weight


namespace Hypergraph {

template<typename Arc> inline
void printHgAsMosesLattice(IHypergraph<Arc> const& hg){
  if(!hg.isFsm()){

  }
  SortStatesOptions opt(SortStatesOptions::kTopSort);
  MutableHypergraph<Arc>* hg_sort = dynamic_cast<MutableHypergraph<Arc>* >(hg.clone());
  if(hg_sort == NULL){

  }




  std::cout << '(';

    std::cout << '(' ;



      int step = next_sid - sid;
      if(step < 0){

      }





    } // foreach
    std::cout << "), " ;
  } // end while

}




#endif
