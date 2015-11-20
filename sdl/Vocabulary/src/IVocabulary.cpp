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

    TODO: would be faster to make Vocabulary::getVocab for kSpecialTerminal
    return the specialSymbols() BasicVocabularyImpl. then all this can go inline
*/

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

IVocabulary::IVocabulary() : threadlocal_(), nSpecials_(specialSymbols().size()) {
  setThreadSpecific();
}

IVocabulary::~IVocabulary() {}

std::string const& IVocabulary::strSpecial(Sym special) {
  assert(special.isSpecial());
  return specialSymbols().str(special);
}

bool IVocabulary::containsSymSpecial(Sym special) {
  assert(special.isSpecial());
  return specialSymbols().containsSym(special);
}

bool IVocabulary::containsSpecial(std::string const& word) {
  return specialSymbols().contains(word);
}

bool IVocabulary::containsSpecial(cstring_span<> word) {
  return specialSymbols().contains(word);
}

Sym IVocabulary::symSpecial(std::string const& word) {
  return specialSymbols().sym(word);
}

Sym IVocabulary::symSpecial(cstring_span<> word) {
  return specialSymbols().sym(word);
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
  std::size_t const before = sizeImpl();
  clearSinceFreeze();
  std::size_t const after = sizeImpl();
  assert(after <= before);
  bool const changed = after != before;
  if (changed)
    SDL_DEBUG(evict.Vocabulary, "evictThread vocabulary " << getName() << " - size " << before << " -> " << after);
  else
    SDL_DEBUG(evict.Vocabulary, "evictThread vocabulary " << getName() << " - no change");
  return changed;
}
