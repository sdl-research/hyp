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

 print Syms using IVocabulary.
*/

#ifndef PRINTSYMS_JG_2015_06_18_HPP
#define PRINTSYMS_JG_2015_06_18_HPP
#pragma once

#include <sdl/Syms.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/StringBuilder.hpp>

namespace sdl {
namespace Vocabulary {


/**
   Simple function that looks up Syms in the vocab
   and prints them.

   \param begin The begin of the sequence, e.g., begin() in
   std::vector<Sym> or Syms.

   \param pVoc The Vocabulary to map from Sym to the
   string symbols; may be empty (in which case the Syms
   are printed)
 */
template <class ForwardIterator>
void lookupAndPrintSymbols(ForwardIterator begin, ForwardIterator end, IVocabulary const* pVoc,
                           std::ostream& out) {
  Util::Sep space(" ");
  for (ForwardIterator symbolIdIter = begin; symbolIdIter != end; ++symbolIdIter) {
    if (pVoc)
      out << space << pVoc->str(*symbolIdIter);
    else
      out << space << *symbolIdIter;
  }
}

template <class ForwardIterator>
void lookupAndPrintSymbols(ForwardIterator begin, ForwardIterator end, IVocabularyPtr const& pVoc,
                           std::ostream& out) {
  lookupAndPrintSymbols(begin, end, pVoc.get(), out);
}

/**
   Function overload for convenience.
 */
template <class Container>
void lookupAndPrintSymbols(Container const& container, IVocabulary const* pVoc, std::ostream& out) {
  lookupAndPrintSymbols(container.begin(), container.end(), pVoc, out);
}

template <class Container>
void lookupAndPrintSymbols(Container const& container, IVocabularyPtr const& pVoc, std::ostream& out) {
  lookupAndPrintSymbols(container.begin(), container.end(), pVoc, out);
}

/// for use in debugging and logging
template <class ForwardIterator>
std::string lookupSymbolsToString(ForwardIterator begin, ForwardIterator end, IVocabulary const* pVoc) {
  std::stringstream ss;
  lookupAndPrintSymbols(begin, end, pVoc, ss);
  return ss.str();
}

template <class ForwardIterator>
std::string lookupSymbolsToString(ForwardIterator begin, ForwardIterator end, IVocabularyPtr const& pVoc) {
  std::stringstream ss;
  lookupAndPrintSymbols(begin, end, pVoc, ss);
  return ss.str();
}
}
}

namespace sdl {
struct PrintSyms {
  Syms const& syms;
  IVocabulary const* voc;
  explicit PrintSyms(Syms const& syms, IVocabulary const* voc = 0) : syms(syms), voc(voc) {}
  PrintSyms(Syms const& syms, IVocabulary const& voc) : syms(syms), voc(&voc) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintSyms const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { Vocabulary::lookupAndPrintSymbols(syms, voc, out); }
};
}

#endif
