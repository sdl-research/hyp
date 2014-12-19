








#include <cstddef>






namespace Hypergraph {

/**


*/
template<class SourceArc>
struct NgramWeightMapper {




  NgramWeightMapper(IHypergraph<SourceArc> const& hg,
                    std::size_t maxlen)


  TargetArc* operator()(SourceArc const* sourceArc) const {
    assert(sourceArc->isFsmArc());


    // sets the n-gram (unigram) on the arc. when these arcs are
    // combined in dynamic programming, the unigrams combine into
    // bigrams, trigrams etc.
    StateId labelStateId = sourceArc->getTail(1);

    if (inputLabel == EPSILON::ID) {

    }
    else {
      targetArc->setWeight(

    }

    return targetArc;
  }




};




#endif
