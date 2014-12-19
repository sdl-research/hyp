






#define EXENAME HgTrie
#define USAGE "Build character trie"
#define VERSION "v1"

#define HG_MAIN




namespace Hypergraph {

struct EXENAME : HypergraphMainBase {
  typedef HypergraphMainBase Base;

  EXENAME() : Base(TRANSFORM_NAME(EXENAME), USAGE, VERSION)
  {
    // usage += stringUnionFromWordList.usageSuffix();












    // wordlist.normalize.addk_num=0;
  }








  void run() {
    HG hg(kFsm|kStoreFirstTailOutArcs|kCanonicalLex);


  }
};




