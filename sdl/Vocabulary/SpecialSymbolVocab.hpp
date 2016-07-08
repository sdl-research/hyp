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
/** \file

    handle kSpecialTerminal Sym type - notice that this isn't an IVocabulary (so
    it's faster)
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

   Currently all special symbols are terminals.
*/
class SpecialSymbolVocab {
 public:
  static SpecialSymbolVocab& getInstance();

  virtual ~SpecialSymbolVocab();

  Sym add(std::string const& symbol);
  Sym addAssertId(std::string const& symbol, SymInt id);

  std::string const& str(Sym id) const;
  bool containsSym(Sym id) const;

  Sym sym(std::string const& symbol) const { return vocab->sym(symbol); }
  Sym sym(cstring_span<> symbol) const { return vocab->sym(symbol); }
  bool contains(cstring_span<> symbol) const { return vocab->contains(symbol); }
  bool contains(std::string const& symbol) const { return vocab->contains(symbol); }

  void accept(IVocabularyVisitor& visitor) { vocab->accept(visitor); }

  unsigned size() const { return vocab->size(); }

  // Disallowed because singleton:
  SpecialSymbolVocab(SpecialSymbolVocab const&) = delete;  // Copy construct
  SpecialSymbolVocab(SpecialSymbolVocab&&) = delete;  // Move construct
  SpecialSymbolVocab& operator=(SpecialSymbolVocab const&) = delete;  // Copy assign
  SpecialSymbolVocab& operator=(SpecialSymbolVocab&&) = delete;  // Move assign

 private:
  // Singleton: make constructor private
  explicit SpecialSymbolVocab();

  typedef BasicVocabularyImpl* BasicVocabPtr;
  // pimpl for safe static init (single threaded at this point) - note
  // specialSymbolsForceInit WILL be called in static init or else bad things.
  BasicVocabPtr vocab = NULL;
};

/// may be used after static initialization
// extern SpecialSymbolVocab specialSymbols;
inline SpecialSymbolVocab& specialSymbols() {
  return SpecialSymbolVocab::getInstance();
}

/// to be used during static initialization
#define SDL_SPECIAL_SYMBOL_FORCE_INIT_CPP 1
#if SDL_SPECIAL_SYMBOL_FORCE_INIT_CPP
SpecialSymbolVocab& specialSymbolsForceInit();
#else
inline SpecialSymbolVocab& specialSymbolsForceInit() {
  return SpecialSymbolVocab::getInstance();
}
#endif


}}

#endif
