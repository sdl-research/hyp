
#include <string>
#include <sstream>

#include <sdl/Hypergraph/HelperFunctions.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>

namespace sdl {
namespace Hypergraph {

template<class Arc>
IMutableHypergraph<Arc>* constructHypergraphFromString(std::string const& hgstr,
                                                       IVocabularyPtr const& pVoc) {
  IMutableHypergraph<Arc>* hg = new MutableHypergraph<Arc>();
  hg->setVocabulary(pVoc);
  std::stringstream ss(hgstr);
  parseText(ss, "<string-input>", hg);
  return hg;
}

}}
