#include <sdl/Hypergraph/IMutableHypergraph.hpp>

namespace sdl {
namespace Hypergraph {

/**
   add existing labels to new vocab, and setVocabulary(vocab).
*/
void IMutableHypergraphBase::translateToVocabulary(IVocabularyPtr const& pNewVocab) {
  if (!pNewVocab) return;
  IVocabulary& newVocab = *pNewVocab;
  IVocabulary const& oldVocab = *hgGetVocabulary();
  if (&newVocab == &oldVocab) return;
  for (StateId s = 0, n = this->hgGetNumStates(); s < n; ++s) {
    if (hgOutputLabelFollowsInput(s)) {
      Sym const in = hgGetInputLabel(s);
      if (in) setInputLabel(s, newVocab.add(in, oldVocab));
    } else {
      LabelPair inOut = hgGetLabelPair(s);
      assert(inOut.first);
      assert(inOut.second);
      newVocab.translateSymbol(inOut.first, oldVocab);
      newVocab.translateSymbol(inOut.second, oldVocab);
      setLabelPair(s, inOut);
    }
  }
  setVocabulary(pNewVocab);  // done at end because releasing shared_ptr may destroy old vocab
}


}}
