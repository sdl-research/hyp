// Copyright 2014 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**
 * @file Hypergraph demo C++ code
 */

#include <iostream>
#include <stdexcept>

#include <sdl/Vocabulary/HelperFunctions.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/InsideAlgorithm.hpp>
#include <sdl/Hypergraph/HelperFunctions.hpp>

using namespace sdl;
using namespace sdl::Hypergraph;
using namespace sdl::Vocabulary;

template<class Arc>
IMutableHypergraph<Arc>* createForest() {

  typedef typename Arc::Weight Weight;

  IVocabularyPtr voc(createDefaultVocab());
  Sym john = voc->add("John", kTerminal);
  Sym loves = voc->add("loves", kTerminal);
  Sym likes = voc->add("likes", kTerminal);
  Sym mary = voc->add("Mary", kTerminal);
  Sym vp = voc->add("VP", kNonterminal);

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

  IVocabularyPtr voc(createDefaultVocab());
  Sym john = voc->add("John", kTerminal);
  Sym loves = voc->add("loves", kTerminal);
  Sym likes = voc->add("likes", kTerminal);
  Sym mary = voc->add("Mary", kTerminal);

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
  std::cout << "Inside costs:" << '\n';
  for (unsigned i = 0; i < costs.size(); ++i) {
    std::cout << i << " " << costs[i] << '\n';
  }
}

/// Prints lattice and forest and prints their inside costs (given
/// templated weight)
template<class Weight>
void demo() {
  std::cout << "Using " << weightName<Weight>() << " weight" << '\n';
  typedef ArcTpl<Weight> Arc;

  std::cout << "=> Lattice" << '\n';
  IHypergraph<Arc>* lattice = createLattice<Arc>();
  std::cout << *lattice << '\n';
  printInsideCosts(*lattice);
  delete lattice;

  std::cout << '\n' << "=> Forest" << '\n';
  IHypergraph<Arc>* forest = createForest<Arc>();
  std::cout << *forest << '\n';
  printInsideCosts(*forest);
  delete forest;

  std::cout << '\n';
}

/// Runs hypergraph demo
int main(int argc, char** argv) {
  try{
    demo<ViterbiWeight>();
    demo<LogWeight>();
  }
  catch(std::exception& e){
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
