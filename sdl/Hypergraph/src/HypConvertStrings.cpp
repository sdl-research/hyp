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
#define USAGE_HypConvertStrings                                                                              \
  "Convert a single line to an FSA accepting its words, (or if -c, unicode chars). If multiple lines, NULL " \
  "byte separates outputs"
#define TRANSFORM_MAIN
#include <sdl/Hypergraph/HypergraphMain.hpp>
#include <sdl/Hypergraph/LineToHypergraph.hpp>

#include <sdl/Util/PrintRange.hpp>
#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/LineToHypergraph.hpp>

#include <sdl/IVocabulary.hpp>

#include <sdl/Vocabulary/HelperFunctions.hpp>

#include <sdl/Util/Input.hpp>

#include <sdl/Util/ProgramOptions.hpp>
#include <sdl/Util/Nfc.hpp>


namespace sdl {
namespace Hypergraph {

struct HypConvertStrings : HypergraphMainBase {
  HypConvertStrings() : HypergraphMainBase("ConvertStrings", USAGE_HypConvertStrings) {}

  void declare_configurable() { this->configurable(&opt); }

  LineToHypergraph opt;
  void run() {
    IVocabularyPtr pVoc = Vocabulary::createDefaultVocab();

    typedef ViterbiWeightTpl<float> Weight;
    typedef ArcTpl<Weight> Arc;

    std::string line;
    bool first = true;
    std::size_t linei = 0;
    MutableHypergraph<Arc> hg;
    hg.setVocabulary(pVoc);
    opt.hgProperties = kStoreInArcs;
    while (Util::getlineNfc(in(), line)) {
      if (!first) std::cout << '\0';  // TODO: deprecated
      opt.toHypergraph(line, &hg, ++linei);
      std::cout << hg;
      first = false;
    }
    ++linei;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(ConvertStrings)
