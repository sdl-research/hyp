// Copyright 2014 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#define SDL_TRANSFORM_MAIN_LOG_WEIGHT 1
#define SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT 1

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>

#include <sdl/Hypergraph/InsideAlgorithm.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>

#ifndef NDEBUG
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#endif

#include <sdl/IVocabulary-fwd.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>

#include <sdl/Util/Forall.hpp>
#include <sdl/Util/ProgramOptions.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <sdl/SharedPtr.hpp>

#include <sdl/Hypergraph/NgramWeightMapper.hpp>
#include <sdl/Hypergraph/MapHypergraph.hpp>
#include <sdl/Util/Input.hpp>
#include <sdl/Util/Matrix.hpp>
#include <sdl/Hypergraph/SortStates.hpp>
#include <sdl/Hypergraph/AllPairsShortestDistance.hpp>

#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/ExpectationWeight.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Util/Locale.hpp>

namespace po = boost::program_options;

namespace sdl {
namespace Hypergraph {

template <class Arc>
void printDistances(IHypergraph<Arc> const& hg, bool allPairs, bool dag, StateIdTranslation& stateRemap,
                    StateId partBoundary) {
  typedef typename Arc::Weight Weight;
  if (allPairs) {
    assert(partBoundary != kNoState);
    StateId n = hg.size();
    assert(partBoundary <= n);
    Util::Matrix<Weight> D(partBoundary, partBoundary, Weight::zero());
    if (dag) {
      AllPairsSortedDag<IHypergraph<Arc> > compute(hg, D);
      for (StateId i = 0; i < n; ++i) {  // state in input
        StateId topi = stateRemap.existingState(i);  // actual state in hg, modified by topo sort
        if (topi == kNoState || topi >= partBoundary) continue;
        for (StateId j = 0; j < n; ++j) {
          if (i == j) continue;
          StateId topj = stateRemap.existingState(j);
          if (topj == kNoState || topj >= partBoundary) continue;
          Weight const& wij = D(topi, topj);
          if (!isZero(wij)) {
            std::cout << i << " -> " << j << " = " << wij << "\n";
          }
        }
      }
    } else {
      floydWarshall(hg, &D);
      for (StateId i = 0; i < n; ++i)
        for (StateId j = 0; j < n; ++j) {
          if (i == j) continue;
          Weight const& wij = D(i, j);
          if (!isZero(wij)) std::cout << i << " -> " << j << " = " << wij << "\n";
        }
    }
  } else {
    boost::ptr_vector<Weight> weights;
    insideAlgorithm(hg, &weights);
    std::size_t i = 0;
    forall (Weight w, weights) { std::cout << i++ << '\t' << w << '\n'; }
  }
}

template <class Arc>
void process(const std::string& file, unsigned ngramMax = 0, bool allPairs = false, bool dag = false) {
  std::cerr << "file=" << file << " ngramMax=" << ngramMax << "\n";
  Util::Input in(file);
  MutableHypergraph<Arc> hg(kDefaultProperties
                            | (allPairs ? (kStoreFirstTailOutArcs | (dag ? kStoreInArcs : 0)) : kStoreInArcs));

  IVocabularyPtr pVoc = Vocabulary::createDefaultVocab();
  hg.setVocabulary(pVoc);

  parseText(*in, file, &hg);

  if (allPairs)
    if (!hg.isFsm()) throw std::runtime_error("hg is not an fsm - can't show all-pairs");

  SortStates<Arc> sort;
  sort.partBoundary = hg.size();
  if (allPairs && dag) {
    sort.clearRemap = false;
    inplace(hg, sort);
  }

  typedef typename Arc::Weight Weight;
  if (ngramMax) {
    typedef NgramWeightMapper<Arc> Mapper;
    typedef typename Mapper::TargetArc TargetArc;
    ;
    MapHypergraph<Arc, TargetArc, Mapper> ngramhg(hg, Mapper(hg, ngramMax));
    printDistances(ngramhg, allPairs, dag, sort.stateRemap, sort.partBoundary);
  } else
    printDistances(hg, allPairs, dag, sort.stateRemap, sort.partBoundary);
}
}
}


int main(int argc, char** argv) {
  using namespace std;
  try {
    sdl::Util::DefaultLocaleFastCout initCout;
    bool help = false;
    unsigned ngramMax = 0;
    bool allPairs = false, dag = false;
    string file = "-", arcType = "log";
    po::options_description generic("Allowed options");
    sdl::AddOption opt(generic);
    opt("arc-type,a", po::value(&arcType)->default_value("log"),
        "arc type (e.g., log, viterbi, expectation, feature)");
    opt("config-file,c", po::value<string>(), "config file name");
    opt("ngram-max,n", po::value(&ngramMax)->default_value(0),
        "if >0, show NgramWeight<arc-type> up to this #of words, starting from arc symbol unigrams");
    opt("help,h", po::bool_switch(&help), "produce help message");
    opt("input-file,i", po::value(&file)->default_value("-"), "input file; - is stdin");
    opt("all-pairs,p", po::bool_switch(&allPairs), "output all-pairs shortest paths (s -> d = w) - fsm only");
    opt("dag,d", po::bool_switch(&dag), "for all-pairs, assume input is a DAG (no cycles)");

    po::options_description cmdline_options;
    cmdline_options.add(generic);

    po::options_description config_file_options;
    config_file_options.add(generic);

    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);

    using namespace sdl;

    if (vm.count("config-file")) {
      Util::Input ifs(vm["config-file"].as<std::string>());
      store(parse_config_file(*ifs, config_file_options), vm);
    }
    notify(vm);

    if (help) {
      std::cout << "Run inside algorithm on hypergraph and output inside weight for each "
                << "state reachable from the FINAL state. if all-pairs, output source -> destination = weight"
                << '\n';
      std::cout << generic << "\n";
      return EXIT_FAILURE;
    }

    using namespace sdl::Hypergraph;

    typedef ViterbiWeightTpl<float> Viterbi;
    if (arcType == "log") {
      typedef ArcTpl<LogWeightTpl<float> > Arc;
      process<Arc>(file, ngramMax, allPairs, dag);
    } else if (arcType == "viterbi") {
      typedef ArcTpl<Viterbi> Arc;
      process<Arc>(file, ngramMax, allPairs, dag);
    } else if (arcType == "expectation") {
      typedef ArcTpl<ExpectationWeight> Arc;
      process<Arc>(file, ngramMax, allPairs, dag);
    } else if (arcType == "feature") {
      typedef ArcTpl<FeatureWeight> Arc;
      process<Arc>(file, ngramMax, allPairs, dag);
    } else {
      std::cerr << "unknown arc-type " << arcType << "\n";
      return EXIT_FAILURE;
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
