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

    handle kSpecialTerminal and kSpecialNonterminal Sym types - notice that
    this isn't an IVocabulary (so it's faster)
*/

#ifndef SPECIALSYMBOLVOCABULARYIMPL_H_
#define SPECIALSYMBOLVOCABULARYIMPL_H_
#pragma once

#include <sdl/Vocabulary/BasicVocabularyImpl.hpp>
#include <sdl/Util/Singleton.hpp>

namespace sdl {
namespace Vocabulary {

/**
   Vocabulary for special symbols (implemented as a singleton).
   This class closely follows the IVocabulary interface although it does not inherit from it.
*/
class SpecialSymbolVocab {
 public:
  explicit SpecialSymbolVocab() {
    init();
  }

  /**
     idempotent, but should be called before threads start. this happens in
     static initialization (in initializing the SpecialSymbolsList.ipp constant
     symbols)
  */
  void init();

  virtual ~SpecialSymbolVocab();

  Sym add(std::string const& symbol, SymbolType symType);

  std::string const& str(Sym const& id) const;

  inline Sym sym(std::string const& symbol, SymbolType symType) const {
    assert(Sym::isSpecialType(symType));
    return (symType == kSpecialTerminal ? vocab : ntVocab)->sym(symbol);
  }

  bool containsSym(Sym const& id) const;

  inline bool contains(std::string const& symbol, SymbolType symType) const {
    assert(Sym::isSpecialType(symType));
    return (symType == kSpecialTerminal ? vocab : ntVocab)->contains(symbol);
  }

  inline void accept(IVocabularyVisitor& visitor) {
    vocab->accept(visitor);
    ntVocab->accept(visitor);
  }

  inline unsigned getNumSymbols(SymbolType type) const {
    return (type == kSpecialTerminal ? vocab : ntVocab)->getNumSymbols();
  }

  inline unsigned getNumLexicals() {
    return vocab->getNumSymbols();
  }

  inline std::size_t getSize() const {
    return vocab->getNumSymbols() + ntVocab->getNumSymbols();
  }

 private:
  typedef BasicVocabularyImpl *BasicVocabPtr;
  // not a scoped_ptr because we want static init to give us 0 for idempotent init()
  BasicVocabPtr vocab, ntVocab;
};

/// may be used after static initialization
// extern SpecialSymbolVocab specialSymbols;
inline SpecialSymbolVocab& specialSymbols() {
  static SpecialSymbolVocab voc; //TODO: not threadsafe in MSVC10
  return voc;
}

/// to be used during static initialization
SpecialSymbolVocab& specialSymbolsForceInit();


}}

#endif
