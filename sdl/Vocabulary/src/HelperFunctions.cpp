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
#include <sdl/SharedPtr.hpp>
#include <sdl/LexicalCast.hpp>

#include <sdl/Vocabulary/BasicVocabularyImpl.hpp>
#include <sdl/Vocabulary/ResidentVocabulary.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Syms.hpp>
#include <sdl/Util/Fields.hpp>

namespace sdl {
namespace Vocabulary {

IVocabularyPtr createDefaultVocab() {
  return IVocabularyPtr(new ResidentVocabulary());
}

void splitToTerminals(std::string const& text, Syms& syms, IVocabulary& vocab) {
  Util::FieldGenerator words(text, ' ');
  for (; words; words.got()) {
    Slice word(words.get());
    if (empty(word)) continue;
    syms.push_back(vocab.addTerminal(word));
  }
}

void splitIntegerSyms(std::string const& text, Syms& syms) {
  Util::FieldGenerator words(text, ' ');
  for (; words; words.got()) {
    Util::Field word(words.get());
    if (empty(word)) continue;
    word.toNumber(&syms.push_back_uninitialized()->id_);
  }
}

void mapToTerminals(std::vector<std::string> const& strs, Syms& syms, IVocabulary& vocab) {
  for (std::vector<std::string>::const_iterator i = strs.begin(), e = strs.end(); i != e; ++i)
    syms.push_back(vocab.addTerminal(*i));
}

void removeBlockSymbols(Syms const& ngram, Syms& result) {
  for (Syms::const_iterator i = ngram.begin(), e = ngram.end(); i != e; ++i)
    if (!Vocabulary::isBlockSymbol(*i)) result.push_back(*i);
}


}}
