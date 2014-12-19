



#include <cassert>
#include <list>
#include <utility>
#include <cmath>













namespace Optimization {

/**

 */
template <class ArcT>

 public:
  typedef Optimization::DataObjectiveFunction<typename ArcT::Weight::FloatT> Base;
  typedef ArcT Arc;
  typedef typename Arc::Weight Weight;
  typedef typename Weight::Map Map;
  typedef typename Weight::FloatT FloatT;

  typedef IFeatureHypergraphPairs<Arc> Pairs;
  typedef typename Pairs::IHgPtr IHgPtr;





  /**

   */


  }

  /**


   */



  }




    using namespace Hypergraph;

    // The amount that the specified examples contribute to the
    // function value:
    FloatT fctValDelta = 0.0;

    // Loop over all examples



      // The hypergraph constrained to observed input and output
      IHgPtr pHgConstrained((*pHgTrainingPairs_)[i].first);




      Map featExpectationsConstrained;


      // The "unconstrained" hypergraph, which is *not* constrained to
      // the observed output (i.e., distribution over possible outputs
      // given the input)
      IHgPtr pHgUnconstrained((*pHgTrainingPairs_)[i].second);




      Map featExpectationsUnconstrained;



      // The gradients are the constrained minus the unconstrained
      // feature expectations:
      for (typename Map::const_iterator it = featExpectationsUnconstrained.begin();
           it != featExpectationsUnconstrained.end(); ++it) {
        // OK because the constrained feat expectations are guaranteed
        // to be a subset of the unconstrained feat expectations:
        updates.update(it->first, featExpectationsConstrained[it->first] - it->second);
      }


      fctValDelta += (pathSumConstrained - pathSumUnconstrained);
    }

    return fctValDelta;
  }

 private:

  shared_ptr<Pairs> pHgTrainingPairs_;  /// Training data
};




#endif
