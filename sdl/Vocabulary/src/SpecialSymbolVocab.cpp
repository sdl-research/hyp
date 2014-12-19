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

#include <cassert>
#include <sdl/Vocabulary/SpecialSymbolVocab.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>

namespace sdl {
namespace Vocabulary {

// SpecialSymbolVocab specialSymbols;

void SpecialSymbolVocab::init() {
  if (!vocab) {
    assert(!ntVocab);
    vocab = new BasicVocabularyImpl(kSpecialTerminal);
    ntVocab = new BasicVocabularyImpl(kSpecialNonterminal);
  }
}

SpecialSymbolVocab::~SpecialSymbolVocab() {
  delete vocab;
  delete ntVocab;
}

/**
   this is to be used during static init
*/
SpecialSymbolVocab& specialSymbolsForceInit() {
  specialSymbols().init();
  return specialSymbols();
}

std::string const& SpecialSymbolVocab::str(Sym const& id) const {
  assert(id.isSpecial());
  assert(id.type() == kSpecialTerminal);  // Currently all special symbols are terminals.
  return
      // the 1000 block start symbols and 1000 constraint substitute tokens are
      // not explicitly in the vocab
      (isBlockOpenOrSubstituteSymbol(id)
           ? (isConstraintSubstituteSym(id) ? CONSTRAINT_SUBSTITUTE::TOKEN : BLOCK_START::TOKEN)
           : vocab->str(id));
}

bool SpecialSymbolVocab::containsSym(Sym const& id) const {
  assert(id.isSpecial());
  assert(id.type() == kSpecialTerminal);  // currently all special symbols are terminals.
  return id < kSpecialEnd();
}


}}
