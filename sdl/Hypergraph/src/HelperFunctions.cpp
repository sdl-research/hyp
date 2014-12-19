
#include <string>
#include <sstream>






namespace Hypergraph {

template<class Arc>


  IMutableHypergraph<Arc>* hg = new MutableHypergraph<Arc>();
  hg->setVocabulary(pVoc);

  parseText(ss, "<string-input>", hg);
  return hg;
}


