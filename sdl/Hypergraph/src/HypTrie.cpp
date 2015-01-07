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
/** \file

    A dictionary (trie) builder. Essentially StatisticalTokenizerTrain without the named entity integration and different defaults.

*/


#define EXENAME HgTrie
#define USAGE "Build character trie"
#define VERSION "v1"

#define HG_MAIN
#include <sdl/Hypergraph/HypergraphMain.hpp>
#include <sdl/Hypergraph/StringUnionFromWordList.hpp>

namespace sdl {
namespace Hypergraph {

struct EXENAME : HypergraphMainBase {
  typedef HypergraphMainBase Base;
  StringUnionFromWordList stringUnionFromWordList;
  EXENAME() : Base(TRANSFORM_NAME(EXENAME), USAGE, VERSION)
  {
    // usage += stringUnionFromWordList.usageSuffix();

    StringUnionOptions &stringUnion = stringUnionFromWordList.stringUnion;
    WordListOptions &wordlist = stringUnionFromWordList.wordlist;

    stringUnion.endString="";
    stringUnion.unkUnigramWeight="";
    stringUnion.loop = false;
    stringUnion.whitespaceTokens.clear();

    wordlist.unigram_addk = 0;
    wordlist.chars = false;
    wordlist.counts = false;
    // wordlist.normalize.addk_num=0;
  }

  void declare_configurable() {
    this->configurable(&stringUnionFromWordList);
  }

  typedef ArcTpl<ViterbiWeight> Arc;
  typedef MutableHypergraph<Arc> HG;

  void run() {
    HG hg(kFsm|kStoreFirstTailOutArcs|kCanonicalLex);
    stringUnionFromWordList.build(this->in(), &hg);
    this->out() << hg;
  }
};

}}

INT_MAIN(sdl::Hypergraph::EXENAME)
