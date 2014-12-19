














#include <vector>
#include <cassert>










namespace Hypergraph {

/**














































*/
template<class Arc>
struct ComputeOutsideScoreStatesVisitor : public IStatesVisitor {

  typedef typename Arc::Weight Weight;

  ComputeOutsideScoreStatesVisitor(IHypergraph<Arc> const& hg,









  /**

  */






  }






};

/**






*/
template<class Arc>
void outsideAlgorithm(IHypergraph<Arc> const& hg,



  // Traverse states in reverse topsorted order (i.e., starting from
  // FINAL root), and compute outsideScore for each state:
  ComputeOutsideScoreStatesVisitor<Arc> outsideScoreComputer(
      hg, insideScores, outsideScores);

}




#endif
