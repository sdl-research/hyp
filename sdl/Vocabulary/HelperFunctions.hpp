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

    things that could be in IVocabulary but aren't.
*/

#ifndef SDL_VOCABULARY_HELPERFUNCTIONS_HPP
#define SDL_VOCABULARY_HELPERFUNCTIONS_HPP
#pragma once

#include <ostream>

#include <sdl/IVocabulary.hpp>
#include <sdl/Util/ObjectCount.hpp>
#include <sdl/Util/ThreadId.hpp>
#include <sdl/Vocabulary/PrintSyms.hpp>
#include <sdl/Vocabulary/ToTerminals.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>

namespace sdl {
namespace Vocabulary {

struct VocabularyResidentLeakCheck : Util::LeakCheckBase {
  IVocabulary const& vocab;
  VocabularyResidentLeakCheck(IVocabulary const& vocab) : vocab(vocab) {
    init("VocabularyResidentLeakCheck");
  }
  void print(std::ostream& o) const { o << "Resident:" << vocab; }
  std::size_t size() const { return vocab.residentSize(); }
};

struct VocabularyUnkLeakCheck : Util::LeakCheckBase {
  IVocabulary const& vocab;
  VocabularyUnkLeakCheck(IVocabulary const& vocab) : vocab(vocab) { init("VocabularyUnkLeakCheck"); }
  void print(std::ostream& o) const { o << "Unk:" << vocab; }
  std::size_t size() const { return vocab.countSinceFreeze(); }
};

/// oddly, # read only symbols changes after init - maybe grammar wasn't fully loaded yet?
struct VocabularyLeakCheck : Util::LeakCheckBase {
  IVocabulary const& vocab;
  VocabularyLeakCheck(IVocabulary const& vocab) : vocab(vocab) { init("VocabularyLeakCheck"); }
  void print(std::ostream& o) const { o << "Size:" << vocab; }
  std::size_t size() const { return vocab.size(); }
};

/**
   Creates default (Resident) vocabulary.
 */
IVocabularyPtr createDefaultVocab();

/**
   Removes block (or JUMP_WALL) symbols from ngram
 */
void removeBlockSymbols(Syms const& ngram, Syms& result);

SymsIndex countBlockSymbols(Syms const& ngram);

// TODO: can rule out variables/NT if we think that will ever happen
inline bool isRuleSrcSymbol(Sym sym) {
  return sym && sym.id_ && !Vocabulary::isBlockSymbolOrJumpWall(sym);
}

inline SymsIndex countRuleSrcSymbols(Syms const& ngram) {
  SymsIndex r = 0;
  for (Syms::const_iterator i = ngram.begin(), e = ngram.end(); i != e; ++i)
    r += Vocabulary::isRuleSrcSymbol(*i);
  return r;
}


}}

#endif
