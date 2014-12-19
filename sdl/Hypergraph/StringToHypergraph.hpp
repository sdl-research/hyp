









#include <string>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <boost/noncopyable.hpp>





















namespace Hypergraph {

/**


 */













  StringToHypergraphOptions(IFeaturesPerInputPosition* newInputFeatures)


  explicit StringToHypergraphOptions(Hypergraph::TokensPtr const& tokens, IFeaturesPerInputPosition* feats)


  /*
  void setOneFeaturePerInputPosition() {

  }
  */

  bool doAddUnknownSymbols;



  // Determines what features to put on what input position.

};


















/**



 */







    pHgResult->addState();

  StateId prevSid = 0;

  typedef typename Arc::Weight Weight;







    const StateId nextSid = prevSid + 1;






    }

    pHgResult->addArc(pArc);
    prevSid = nextSid;
  }

}

/**














 */


                     IMutableHypergraph<Arc>* pHgResult,
                     StringToHypergraphOptions const& opts = StringToHypergraphOptions()) {
  if (inputTokens.size() != outputTokens.size()) {

                  "The two strings must have same number of words");
  }

  // 1. Create simple FSA from input tokens:
  stringToHypergraph(inputTokens, pHgResult, opts);

  // 2. Insert output tokens:




  while (stateId != finalId) {


    setFsmOutputLabel(pHgResult, *arc, symId);


  }
}




#endif
