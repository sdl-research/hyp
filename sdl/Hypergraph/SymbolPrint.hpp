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

    utilities for printing state labels

    this has some Hypergraph file format specific escaping but some of it could move to Vocabulary
*/

#ifndef HYP__HYPERGRAPH_SYMBOLPRINT_HPP
#define HYP__HYPERGRAPH_SYMBOLPRINT_HPP
#pragma once

#include <string>

#include <sdl/IVocabulary.hpp>
#include <sdl/Syms.hpp>
#include <sdl/Util/SizeIsUnsigned.hpp>
#include <sdl/Hypergraph/LabelPair.hpp>
#include <sdl/Util/Print.hpp>
#include <sdl/Util/PrintRange.hpp>

#include <sdl/Util/Enum.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Hypergraph/PrintOptions.hpp>
#include <sdl/Util/StringBuilder.hpp>
#include <sdl/Hypergraph/FwdDecls.hpp>

namespace sdl {
namespace Hypergraph {

/// (for debugging output where vocab isn't available)
void writeLabel(std::ostream& out, Sym sym);
void writeLabel(Util::StringBuilder& out, Sym sym);

/// prints quoted things exactly as parseable Hypergraph text format, so safer here (than in Vocabulary)
void writeLabel(std::ostream& out, std::string const& label, SymbolQuotation quote = kUnquoted);
void writeLabel(Util::StringBuilder& out, std::string const& label, SymbolQuotation quote = kUnquoted);

inline void writeLabel(std::ostream& out, Sym sym, IVocabulary const& voc, SymbolQuotation quote = kQuoted) {
  writeLabel(out, voc.str(sym), sym.isLexical() ? quote : kUnquoted);
}
inline void writeLabel(Util::StringBuilder& out, Sym sym, IVocabulary const& voc,
                       SymbolQuotation quote = kQuoted) {
  writeLabel(out, voc.str(sym), sym.isLexical() ? quote : kUnquoted);
}


inline void writeLabel(std::ostream& out, Sym sym, IVocabulary const* voc, SymbolQuotation quote = kQuoted) {
  if (voc && voc->containsSym(sym))
    writeLabel(out, sym, *voc, quote);
  else
    writeLabel(out, sym);
}
inline void writeLabel(Util::StringBuilder& out, Sym sym, IVocabulary const* voc,
                       SymbolQuotation quote = kQuoted) {
  if (voc && voc->containsSym(sym))
    writeLabel(out, sym, *voc, quote);
  else
    writeLabel(out, sym);
}


inline void writeLabel(std::ostream& out, Sym sym, IVocabularyPtr const& voc, SymbolQuotation quote = kQuoted) {
  writeLabel(out, sym, voc.get(), quote);
}
inline void writeLabel(Util::StringBuilder& out, Sym sym, IVocabularyPtr const& voc,
                       SymbolQuotation quote = kQuoted) {
  writeLabel(out, sym, voc.get(), quote);
}

template <class Arc>
void print(std::ostream& o, Sym sym, IHypergraph<Arc> const& hg, SymbolQuotation quote = kQuoted) {

  Hypergraph::writeLabel(o, sym, hg.getVocabulary(), quote);
}

}  // ns

/* these are in NS sdl so they can be found by Printer */
inline void print(std::ostream& o, Sym sym, IVocabulary& voc, Hypergraph::SymbolQuotation quote) {

  Hypergraph::writeLabel(o, sym, voc, quote);
}
inline void print(Util::StringBuilder& o, Sym sym, IVocabulary& voc, Hypergraph::SymbolQuotation quote) {

  Hypergraph::writeLabel(o, sym, voc, quote);
}

inline void print(std::ostream& o, Sym sym, IVocabularyPtr const& pVoc, Hypergraph::SymbolQuotation quote) {
  Hypergraph::writeLabel(o, sym, pVoc, quote);
}
inline void print(Util::StringBuilder& o, Sym sym, IVocabularyPtr const& pVoc,
                  Hypergraph::SymbolQuotation quote) {
  Hypergraph::writeLabel(o, sym, pVoc, quote);
}

inline void print(std::ostream& o, Hypergraph::LabelPair const& l, IVocabulary& v,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  using namespace Hypergraph;
  Sym const i = input(l), ol = output(l);
  writeLabel(o, i, v, quote);
  if (ol && i != ol) writeLabel(o << ' ', ol, v, quote);
}
inline void print(Util::StringBuilder& o, Hypergraph::LabelPair const& l, IVocabulary& v,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  using namespace Hypergraph;
  Sym const i = input(l), ol = output(l);
  writeLabel(o, i, v, quote);
  if (ol && i != ol) writeLabel(o << ' ', ol, v, quote);
}

inline void print(std::ostream& o, Hypergraph::LabelPair const& l, IVocabularyPtr const& v,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  print(o, l, *v, quote);
}
inline void print(Util::StringBuilder& o, Hypergraph::LabelPair const& l, IVocabularyPtr const& v,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  print(o, l, *v, quote);
}

inline void print(std::ostream& o, Hypergraph::LabelPair const& l, IVocabulary* v,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  print(o, l, *v, quote);
}
inline void print(Util::StringBuilder& o, Hypergraph::LabelPair const& l, IVocabulary* v,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  print(o, l, *v, quote);
}

inline void print(std::ostream& o, Syms const& s, IVocabulary& voc, char const* space,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  Util::Sep sp(space);
  for (Sym w : s) {
    if (w) {
      o << sp;
      print(o, w, voc, quote);
    }
  }
}
inline void print(Util::StringBuilder& o, Syms const& s, IVocabulary& voc, char const* space,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  Util::Sep sp(space);
  for (Sym w : s) {
    if (w) {
      o << sp;
      print(o, w, voc, quote);
    }
  }
}

inline void print(std::ostream& o, SymSlice const& syms, IVocabulary& voc, char const* space = " ",
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  Util::Sep sp(space);
  for (Psym i = syms.first; i < syms.second; ++i) {
    if (*i) {
      o << sp;
      print(o, *i, voc, quote);
    }
  }
}

inline void print(Util::StringBuilder& o, SymSlice const& syms, IVocabulary& voc, char const* space = " ",
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  Util::Sep sp(space);
  for (Psym i = syms.first; i < syms.second; ++i) {
    if (*i) {
      o << sp;
      print(o, *i, voc, quote);
    }
  }
}

inline void print(std::ostream& o, Syms const& s, IVocabularyPtr const& voc, char const* space,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  print(o, s, *voc, space, quote);
}
inline void print(Util::StringBuilder& o, Syms const& s, IVocabularyPtr const& voc, char const* space,
                  Hypergraph::SymbolQuotation quote = Hypergraph::kQuoted) {
  print(o, s, *voc, space, quote);
}


}

#endif
