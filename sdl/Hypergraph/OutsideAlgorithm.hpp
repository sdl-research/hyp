














#include <vector>
#include <cassert>







#include <boost/ptr_container/ptr_vector.hpp>


namespace Hypergraph {

/**





void outsideFromInside(StateId stateId







            "Computing outside score for " << stateId << ", sum=" << sum);

  if (stateId == final)


    forall (ArcId aid, hg.outArcIds(stateId)) {
      Arc const& arc = *hg.outArc(stateId, aid);


      // Product order doesn't matter, since outside is not well
      // defined for non-commutative semirings (would have to store a
      // pair of left-outside, right-outside).

        //TODO: Doesn't allow for multiple occurences of same state in
        // tails (seen w/ e.g. copying tree transducers or
        // determinize-minimize of some unweighted tree forest, but
        // otherwise unlikely). to be clear: we should only skip one
        // occurence of the stateId in tails, not all of them.
        if (tail != stateId) {














*/
template<class Arc>
struct ComputeOutsideScoreStatesVisitor : public IStatesVisitor {

  typedef typename Arc::Weight Weight;

  ComputeOutsideScoreStatesVisitor(IHypergraph<Arc> const& hg,
                                   boost::ptr_vector<Weight> const& insideScores,
                                   boost::ptr_vector<Weight>* outsideScores)







  /**

  */
  void visit(StateId stateId) {

    Weight &outside = Util::atExpandPtr(*outsideScores_, stateId, kZero);
    outsideFromInside(stateId, hg_, insideScores_, *outsideScores_, outside, final);

              "Stored outside distance: " << (*outsideScores_)[stateId] << " to state " << stateId);
  }






};

/**






*/
template<class Arc>
void outsideAlgorithm(IHypergraph<Arc> const& hg,
                      boost::ptr_vector<typename Arc::Weight> const& insideScores,
                      boost::ptr_vector<typename Arc::Weight>* outsideScores) {

  // Traverse states in reverse topsorted order (i.e., starting from
  // FINAL root), and compute outsideScore for each state:
  ComputeOutsideScoreStatesVisitor<Arc> outsideScoreComputer(
      hg, insideScores, outsideScores);

}




#endif
