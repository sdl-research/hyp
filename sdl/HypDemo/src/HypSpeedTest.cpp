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
/**
 * @file Hypergraph demo C++ code
 */

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

using namespace sdl;
using namespace sdl::Hypergraph;
using namespace sdl::Vocabulary;

template<class Arc>
IMutableHypergraph<Arc>* readHyp(std::string const& fname,
			  IVocabularyPtr const& voc) {
  IMutableHypergraph<Arc>* hg = new MutableHypergraph<Arc>();
  hg->setVocabulary(voc);
  std::ifstream in(fname.c_str());
  parseText(in, fname, hg);
  return hg;
}

int main(int argc, char** argv) {
  IVocabularyPtr voc(createDefaultVocab());
  typedef ArcTpl<ViterbiWeight> Arc;
  IMutableHypergraph<Arc>* hyp1 = readHyp<Arc>(std::string(argv[1]), voc);
  IMutableHypergraph<Arc>* hyp2 = readHyp<Arc>(std::string(argv[2]), voc);
  sortArcs(hyp2);
  MutableHypergraph<Arc> hyp3;

  std::cerr << "Composing...";
  time_t start, end;
  time (&start);
  fs::compose(*hyp1, *hyp2, &hyp3, fs::FstComposeOptions());
  time (&end);
  fprintf(stderr, "Elasped time is %.4lf seconds.\n", difftime(end,start));

  std::cerr << "Writing" << std::endl;
  std::cout << hyp3 << std::endl;
  delete hyp1;
  delete hyp2;
  return EXIT_SUCCESS;
}
