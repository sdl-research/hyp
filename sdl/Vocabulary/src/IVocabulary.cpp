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
#include <limits>
#include <sstream>
#include <stdexcept>
#include <sdl/Util/LogHelper.hpp>

#include <sdl/Sym.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/BasicVocabularyImpl.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>

using namespace sdl;
using namespace sdl::Vocabulary;


Sym IVocabulary::getVariableId(unsigned index) const {
  return Sym::getVariableId(index);
}

std::string const& IVocabulary::str(Sym sym) const {
  if (sym.isSpecial()) return specialSymbols().str(sym);  // TODO: why?
  assert(containsSymImpl(sym));
  return strImpl(sym);
}

Sym IVocabulary::sym(std::string const& symbol, SymbolType symType) const {
  if (symType == kSpecialTerminal) return specialSymbols().sym(symbol);  // TODO: why?
  return symImpl(symbol, symType);
}

unsigned IVocabulary::getNumSymbols(SymbolType symType) const {
  if (symType == kSpecialTerminal) return specialSymbols().getNumSymbols();
  return getNumSymbolsImpl(symType);
}

std::size_t IVocabulary::getSize() const {
  return getSizeImpl() + specialSymbols().getSize();
}

bool IVocabulary::containsSym(Sym sym) const {
  if (sym.isSpecial()) return specialSymbols().containsSym(sym);  // TODO: why?
  return containsSymImpl(sym);
}

bool IVocabulary::contains(std::string const& symbol, SymbolType symType) const {
  if (symType == kSpecialTerminal) return specialSymbols().contains(symbol);  // TODO: why?
  return containsImpl(symbol, symType);
}

void IVocabulary::print(std::ostream& out) const {
  out << "{" << category() << " ";
  if (!name_.empty()) out << name_ << " ";
  if (Util::isDebugBuild()) out << " @" << this;
  WordCount const unk = countSinceFreeze();
  WordCount const ro = readOnlySize();
  WordCount const resident = residentSize();
  out << " #unk=" << unk;
  out << " #read-only=" << ro;
  if (resident != unk) out << " #resident=" << resident;
  out << " #frozen=" << (size() - unk - ro);
  out << "}";
}

bool IVocabulary::evictThread(Occupancy const&) {
  SDL_DEBUG(evict.Vocabulary, "vocabulary evictThread " << getName());
  std::size_t const before = getSizeImpl();
  clearSinceFreeze();
  std::size_t const after = getSizeImpl();
  assert(after <= before);
  bool const changed = after != before;
  if (changed)
    SDL_DEBUG(evict.Vocabulary, "evictThread vocabulary " << getName() << " - size " << before << " -> "
                                                          << after);
  else
    SDL_DEBUG(evict.Vocabulary, "evictThread vocabulary " << getName() << " - no change");
  return changed;
}
