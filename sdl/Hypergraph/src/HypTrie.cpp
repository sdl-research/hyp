/** \file

    A dictionary (trie) builder. Essentially StatisticalTokenizerTrain without the named entity integration
   and different defaults.

*/


#define USAGE_HypTrie "Build character trie"

#define TRANSFORM_MAIN
#include <sdl/Hypergraph/HypergraphMain.hpp>
#include <sdl/Hypergraph/StringUnionFromWordList.hpp>

namespace sdl {
namespace Hypergraph {

struct HypTrie : HypergraphMainBase {
  HypTrie() : HypergraphMainBase("Trie", USAGE_HypTrie) {
    StringUnionOptions& stringUnion = stringUnionFromWordList.stringUnion;
    WordListOptions& wordlist = stringUnionFromWordList.wordlist;

    stringUnion.endString = "";
    stringUnion.unkUnigramWeight = "";
    stringUnion.loop = false;
    stringUnion.whitespaceTokens.clear();

    wordlist.unigram_addk = 0;
    wordlist.chars = false;
    wordlist.counts = false;
  }

  void declare_configurable() { this->configurable(&stringUnionFromWordList); }

  typedef ArcTpl<ViterbiWeight> Arc;
  typedef MutableHypergraph<Arc> HG;

  void run() {
    HG hg(kFsmOutProperties);
    stringUnionFromWordList.build(this->in(), &hg);
    this->out() << hg;
  }

  StringUnionFromWordList stringUnionFromWordList;
};


}}

HYPERGRAPH_NAMED_MAIN(Trie)
