/** \file

    A dictionary (trie) builder. Essentially StatisticalTokenizerTrain without the named entity integration and different defaults.

*/


#define TRANSFORM HypTrie
#define USAGE "Build character trie"
#define VERSION "v1"

#define HG_MAIN
#include <sdl/Hypergraph/HypergraphMain.hpp>
#include <sdl/Hypergraph/StringUnionFromWordList.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : HypergraphMainBase {
  typedef HypergraphMainBase Base;
  StringUnionFromWordList stringUnionFromWordList;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION)
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

INT_MAIN(sdl::Hypergraph::TRANSFORM)
