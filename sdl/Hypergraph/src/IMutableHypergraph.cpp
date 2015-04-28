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
