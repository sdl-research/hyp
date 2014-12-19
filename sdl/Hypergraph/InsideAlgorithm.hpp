/**





*/





#include <vector>











#include <boost/ptr_container/ptr_vector.hpp>


namespace Hypergraph {

/**












struct ComputeDistanceStatesVisitor
    : IStatesVisitor, private ArcWtFn, private StateWtFn
{




  /**


     *







  */













  /**

  */
  void visit(StateId sid) {




























        }


      }
    }
  }

 private:




};

/**


   // state weights are only used for states w/ no inarcs - not
   // exactly the same as what AxiomWeightHypergraph does

   \param maxNonLexical if set is the greatest non-leaf state id
   (i.e. state weights will be used for all states > than this). side
   effect: pDistances doesn't grow larger than maxNonLexical+1
*/

void insideAlgorithm(






  shared_ptr<IHypergraph<Arc> const> phg = ensureProperties(hg, kStoreInArcs);






  ComputeDistanceStatesVisitor<Arc, StateWtFn, ArcWtFn, Distances> distanceComputer(
      *phg, *pDistances, stateWtFn, arcWtFn, maxNotTerminal == (StateId)-1);













template<class Arc>
void insideAlgorithm(
    IHypergraph<Arc> const& hg,



}




#endif
