













#include <map>











namespace Hypergraph {

namespace UnionHelper {
/**


*/
template <class Arc>
struct ArcCopyFct {







    Arc* newArc = new Arc();



    }


  }






  }











};


/**

*/
template <class Arc>


  ASSERT_VALID_HG(sourceFst);
  if (empty(sourceFst)) {  // FIXME: this is not a cheap operation. prunedEmpty is, though.

    return;
  }




  // Write sourceFst into pTargetFst
  UnionHelper::ArcCopyFct<Arc> fct(sourceFst, pTargetFst);


  StateId epsLabelState = pTargetFst->addState(EPSILON::ID);

  // Add common new start state
  StateId unionStartSid = pTargetFst->addState();




  // Add common new final state
  StateId unionFinalSid = pTargetFst->addState();



  ASSERT_VALID_HG(*pTargetFst);
}

template <class Arc>


    UnionHelper::ArcCopyFct<Arc> copier(sourceHg, pTargetHg);




    // Add common new final state
    StateId superfinal = pTargetHg->addState();



  }
}




#endif
