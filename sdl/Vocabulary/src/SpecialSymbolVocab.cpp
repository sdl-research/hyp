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
#include <sdl/Util/String.hpp>
#include <sdl/Util/StringBuilder.hpp>

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

/// can't use std::string const because SpecialSymbolList gets used to call this at static init
char const* const kXmtBlockStr("<xmt-block>");

char const* const kXmtEntityStr("<xmt-entity>");

static Sym addSdlNumBlocks(BasicVocabularyImpl &s, std::string const& pre) {
  std::string name(pre);
  std::size_t const szpre = name.size() - 1;
  name.resize(szpre);
  name.push_back('0');
  name.push_back('>');
  Sym const r = s.add(name);
  assert(SDL_NUM_BLOCKS % 10 == 0);
  for (unsigned tens = 0; tens < SDL_NUM_BLOCKS; ) {
    name.resize(szpre);
    graehl::utos_append(name, tens);
    name.push_back('>');
    tens += 10;
    char &ones = name[name.size() - 2];
    s.add(name);
    for (char c = '1'; c <= '9'; ++c) {
      ones = c;
      s.add(name);
    }
  }
  return r;
}


Sym SpecialSymbolVocab::add(std::string const& symbol, SymbolType symType) {
  assert(Sym::isSpecialType(symType));
  assert(symType == kSpecialTerminal);  // Currently all special symbols are terminals.
  if (symbol == kXmtBlockStr || symbol == kXmtEntityStr)
    return addSdlNumBlocks(*vocab, symbol);
  else
    return vocab->add(symbol, symType);
}

std::string const& SpecialSymbolVocab::str(Sym const& id) const {
  assert(id.isSpecial());
  assert(id.type() == kSpecialTerminal);  // Currently all special symbols are terminals.
  return vocab->str(id);
}

bool SpecialSymbolVocab::containsSym(Sym const& id) const {
  assert(id.isSpecial());
  assert(id.type() == kSpecialTerminal);  // currently all special symbols are terminals.
  return id < kSpecialEnd();
}


}}
