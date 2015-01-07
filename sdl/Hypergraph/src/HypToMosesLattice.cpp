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
#include <sdl/LexicalCast.hpp>
#include <sdl/Util/InitLoggerFromConfig.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/Input.hpp>
#include <sdl/Util/Locale.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/IHypergraphsIteratorTpl.hpp>
#include <sdl/Hypergraph/PrintHgAsMosesLattice.hpp>
#include <sdl/Hypergraph/Properties.hpp>

#include <sdl/SharedPtr.hpp>
#include <sdl/Util/ProgramOptions.hpp>

#include <log4cxx/xml/domconfigurator.h>

using namespace sdl;

namespace po = boost::program_options;

Util::DefaultLocaleFastCout initCout;

int main(int argc, char** argv) {
  std::string logConfigFile;
  po::options_description generic("Generic options");
  sdl::AddOption opt(generic);
  opt("help", "produce help message");
  opt("input-file",
      "input file in plain text, hypergraphs are separated "
      "by \"-----\"");
  opt("log-config", po::value(&logConfigFile), "log4cxx config file");
  po::positional_options_description p;
  p.add("input-file", 1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(generic).positional(p).run(), vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << "Converts SDL lattice format to PLF format (used by Moses)\n";
    std::cout << generic << '\n';
    return EXIT_SUCCESS;
  }
  Util::defaultLocale();
  Util::Input input;
  if (vm.count("input-file")) {
    std::string const& file = vm["input-file"].as<std::string>();
    input.init(file);
  }
  Util::initLoggerFromConfig(logConfigFile, "HypToMosesLattice", Util::kLogInfo);
  IVocabularyPtr pVoc = Vocabulary::createDefaultVocab();
  std::istream& instream = input.getStream();
  typedef Hypergraph::ArcTpl<Hypergraph::ViterbiWeightTpl<float> > Arc;
  PerProcessVocabulary voc(pVoc);
  Hypergraph::IHypergraphsIteratorTpl<Arc>* pHgIter
      = Hypergraph::IHypergraphsIteratorTpl<Arc>::create(instream, Hypergraph::kDashesSeparatedHg,
                                                         ptrNoDelete(voc));
  pHgIter->setHgProperties(Hypergraph::kStoreFirstTailOutArcs);
  while (!pHgIter->done()) {
    Hypergraph::IHypergraph<Arc>* pHg = pHgIter->value();
    pHgIter->next();
    Hypergraph::printHgAsMosesLattice(*pHg);
  }
}
