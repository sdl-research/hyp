/**
 * @file Hypergraph demo C++ code
 */

#include <iostream>
#include <stdexcept>













template<class Arc>
IMutableHypergraph<Arc>* createForest() {

  typedef typename Arc::Weight Weight;








  // Construct hypergraph that stores incoming arcs per state, and
  // uses canonical state IDs for terminal symbols:
  IMutableHypergraph<Arc>* hyp = new MutableHypergraph<Arc>(kStoreInArcs | kCanonicalLex);
  hyp->setVocabulary(voc);

  StateId s0 = hyp->addState(vp);
  StateId s1 = hyp->addState();

  hyp->addArc(new Arc(Head(s0),
                      Tails(hyp->addState(likes), hyp->addState(mary)),
                      Weight(2.0f)));
  hyp->addArc(new Arc(Head(s0),
                      Tails(hyp->addState(loves), hyp->addState(mary)),
                      Weight(4.0f)));
  hyp->addArc(new Arc(Head(s1),
                      Tails(hyp->addState(john), s0),
                      Weight(8.0f)));

  hyp->setFinal(s1);
  return hyp;
}

template<class Arc>
IMutableHypergraph<Arc>* createLattice() {

  typedef typename Arc::Weight Weight;







  IMutableHypergraph<Arc>* hyp = new MutableHypergraph<Arc>();
  hyp->setVocabulary(voc);

  StateId s0 = hyp->addState();
  StateId s1 = hyp->addState();
  StateId s2 = hyp->addState();
  StateId s3 = hyp->addState();

  hyp->addArc(new Arc(Head(s1),
                      Tails(s0, hyp->addState(john)),
                      Weight(2.0f)));
  hyp->addArc(new Arc(Head(s2),
                      Tails(s1, hyp->addState(loves)),
                      Weight(4.0f)));
  hyp->addArc(new Arc(Head(s2),
                      Tails(s1, hyp->addState(likes)),
                      Weight(8.0f)));
  hyp->addArc(new Arc(Head(s3),
                      Tails(s2, hyp->addState(mary)),
                      Weight(16.0f)));

  hyp->setStart(s0);
  hyp->setFinal(s3);
  return hyp;
}

/// Computes and prints inside costs
template<class Arc>
void printInsideCosts(IHypergraph<Arc> const& hyp) {
  typedef typename Arc::Weight Weight;
  boost::ptr_vector<Weight> costs;
  insideAlgorithm(hyp, &costs);

  for (unsigned i = 0; i < costs.size(); ++i) {

  }
}

/// Prints lattice and forest and prints their inside costs (given
/// templated weight)
template<class Weight>
void demo() {

  typedef ArcTpl<Weight> Arc;


  IHypergraph<Arc>* lattice = createLattice<Arc>();

  printInsideCosts(*lattice);
  delete lattice;


  IHypergraph<Arc>* forest = createForest<Arc>();

  printInsideCosts(*forest);
  delete forest;


}

/// Runs hypergraph demo
int main(int argc, char** argv) {
  try{
    demo<ViterbiWeight>();
    demo<LogWeight>();
  }
  catch(std::exception& e){

    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
