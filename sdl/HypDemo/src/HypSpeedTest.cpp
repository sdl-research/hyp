// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    Hypergraph demo C++ code
*/
char const* usage = "measure performance of compose (excluding file input): HypSpeedTest fst1 fst2 [1|0 (prune-to-nbest)] [N=100 repetitions]";

#include <time.h>
#include <iostream>
#include <stdexcept>

#include <sdl/Vocabulary/HelperFunctions.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/HelperFunctions.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <sdl/Hypergraph/fs/Compose.hpp>
#include <sdl/Hypergraph/SortArcs.hpp>
#include <sdl/Hypergraph/BestPath.hpp>
#include <graehl/shared/monotonic_time.hpp>

using namespace sdl;
using namespace sdl::Hypergraph;
using namespace sdl::Vocabulary;

template <class Arc>
IMutableHypergraph<Arc>* readHyp(std::string const& fname, IVocabularyPtr const& voc) {
  IMutableHypergraph<Arc>* hg = new MutableHypergraph<Arc>(kFsmOutProperties | kCanonicalLex);
  hg->setVocabulary(voc);
  std::ifstream in(fname.c_str());
  parseText(in, fname, hg);
  return hg;
}

inline double seconds(double sec, double nsec) {
  return sec + nsec * 1e-9;
}

int main(int argc, char** argv) {
  IVocabularyPtr voc(createDefaultVocab());
  typedef ArcTpl<ViterbiWeight> Arc;
  if (argc < 3) {
    fprintf(stderr, "%s\n", usage);
    return 1;
  }
  IMutableHypergraph<Arc>* hyp1 = readHyp<Arc>(std::string(argv[1]), voc);
  IMutableHypergraph<Arc>* hyp2 = readHyp<Arc>(std::string(argv[2]), voc);
  sortArcs(hyp2);

  double tstart = 0, tend;
  fs::FstComposeOptions opt;
  opt.allowDuplicatePathsIf1Best = false;
  opt.pruneToNbest = argc > 3 && argv[3][0] == '1';
  int N = argc > 4 ? atoi(argv[4]) : 100;
  fprintf(stderr, "prune-to-nbest=%d\n", opt.pruneToNbest);
  BestPathOutToOptions best;
  best.nbest = 100;
  if (N)
    for (int dup = 0; dup < 2; ++dup) {
      for (int eps = 0; eps < 2; ++eps) {
        if (dup && eps) break;
        opt.allowDuplicatePaths = dup;
        opt.epsilonMatchingFilter = eps;
        MutableHypergraph<Arc> hyp3(kFsmOutProperties);
        fprintf(stderr, "Composing dup=%d eps=%d...\n", dup, eps);
        for (int i = 0; i <= N; ++i) {
          if (i == 1)  // warm up cache
            tstart = graehl::monotonic_time();
          fs::compose(*hyp1, *hyp2, &hyp3, opt);
        }
        tend = graehl::monotonic_time();
        double dt = tend - tstart;
        fprintf(stderr, "Elapsed dup=%d eps=%d time: %.6f seconds N=%d seconds-per=%g\n", dup, eps, dt, N,
                dt / N);
        std::cout << hyp3 << '\n';
        try {
          best.outputForId(hyp3, std::cerr);
        } catch (std::exception& e) {
          fprintf(stderr, "ERROR: %s\n", e.what());
        }
        std::cout << "\n\n";
      }
    }
  delete hyp1;
  delete hyp2;
  return EXIT_SUCCESS;
}
