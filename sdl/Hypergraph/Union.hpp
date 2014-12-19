













#include <map>











namespace Hypergraph {

namespace UnionHelper {
/**


*/
template <class Arc>
struct ArcCopyFct {






  void operator()(Arc* pArc) const {
    Arc* newArc = new Arc();

    for (TailId i = 0, e = (TailId)pArc->getNumTails(); i < e; ++i) {
      newArc->addTail(getNewStateId(pArc->getTail(i)));
    }


  }

  StateId getNewStateId(StateId oldSid) const {




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
  sourceFst.forArcs(fct);  // must store arcs

  StateId epsLabelState = pTargetFst->addState(EPSILON::ID);

  // Add common new start state
  StateId unionStartSid = pTargetFst->addState();




  // Add common new final state
  StateId unionFinalSid = pTargetFst->addState();



  ASSERT_VALID_HG(*pTargetFst);
}

template <class Arc>


    UnionHelper::ArcCopyFct<Arc> copier(sourceHg, pTargetHg);
    sourceHg.forArcs(copier);

    StateId epsLabelStateId = pTargetHg->addState(EPSILON::ID);

    // Add common new final state
    StateId superfinal = pTargetHg->addState();



  }
}




#endif
