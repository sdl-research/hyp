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

    string->symbol
*/

#ifndef SDL_VOCABULARY__TO_SYMBOLS
#define SDL_VOCABULARY__TO_SYMBOLS
#pragma once

#include <boost/range/algorithm/transform.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Syms.hpp>

namespace sdl {

struct AddSymbol {
  IVocabulary* pVoc;
  AddSymbol(IVocabulary& voc) : pVoc(&voc) {}
  AddSymbol(IVocabulary* pVoc) : pVoc(pVoc) {}
  AddSymbol(IVocabularyPtr const& pVoc) : pVoc(pVoc.get()) {}
  AddSymbol(AddSymbol const& o) : pVoc(o.pVoc) {}
  typedef Sym result_type;
  typedef std::string const& argument_type;
  Sym operator()(std::string const& str) const { return pVoc->add(str, kTerminal); }
};


struct GetSymbol {
  IVocabulary* pVoc;  // don't want to make copies of smart ptr
  GetSymbol(IVocabulary* pVoc) : pVoc(pVoc) {}
  GetSymbol(IVocabularyPtr const& pVoc) : pVoc(pVoc.get()) {}
  GetSymbol(GetSymbol const& o) : pVoc(o.pVoc) {}
  typedef std::string result_type;
  typedef Sym argument_type;
  std::string operator()(Sym symId) const { return pVoc->str(symId); }
};


/// copies range (e.g. vector) of strings to output iter transforming by
/// AddSymbol: if the vocab didn't have the symbols before, it certainly will
/// after.
template <class InStrings, class OutSymbolIter>
inline OutSymbolIter toSymbols(InStrings const& symStrs, OutSymbolIter o, IVocabularyPtr const& voc) {
  return boost::transform(symStrs, o, AddSymbol(voc));
}

/// as above but back_insert (push_back) to output container
template <class InStrings, class OutSymbols>
inline void appendSymbols(InStrings const& symStrs, OutSymbols& o, IVocabularyPtr const& voc) {
  toSymbols(symStrs, std::back_inserter(o), voc);
}

/// as above but return vec
template <class InStrings>
inline Syms toSymbols(InStrings const& symStrs, IVocabularyPtr const& voc) {
  Syms r;
  appendSymbols(symStrs, r, voc);
  return r;
}


}

#endif
