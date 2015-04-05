// Copyright 2014-2015 SDL plc
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
#include <sdl/SharedPtr.hpp>
#include <sdl/LexicalCast.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include <sdl/Vocabulary/BasicVocabularyImpl.hpp>
#include <sdl/Vocabulary/ResidentVocabulary.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Syms.hpp>

namespace sdl {
namespace Vocabulary {

IVocabularyPtr createDefaultVocab() {
  return IVocabularyPtr(new ResidentVocabulary());
}

boost::char_separator<char> const kSpaceDelimiter(" ");

// TODO: do we want to use Util::Tokenizer<Unicodes> instead?
void terminalTextToSignature(std::string const& text, Syms* syms, IVocabulary& vocab) {
  typedef boost::tokenizer<boost::char_separator<char> > Tokens;
  Tokens tokens(text, kSpaceDelimiter);
  // create mapping and store symbol
  for (Tokens::const_iterator i = tokens.begin(), e = tokens.end(); i != e; ++i)
    syms->push_back(vocab.add(*i, kTerminal));
}

void terminalTextToSignature(std::string const& text, Syms* syms, IVocabularyPtr const& pVocabulary) {
  terminalTextToSignature(text, syms, *pVocabulary);
}

void terminalStringsToSignature(std::vector<std::string> const& strs, Syms* syms, IVocabulary& vocab) {
  forall (std::string const& word, strs) { syms->push_back(vocab.add(word, kTerminal)); }
}

void removeBlockSymbols(Syms const& ngram, Syms& result) {
  for (Syms::const_iterator i = ngram.begin(), e = ngram.end(); i != e; ++i)
    if (!Vocabulary::isBlockSymbol(*i)) result.push_back(*i);
}

Syms removeBlockSymbols(Syms const& ngram) {
  Syms result;
  removeBlockSymbols(ngram, result);
  return result;
}


}}
