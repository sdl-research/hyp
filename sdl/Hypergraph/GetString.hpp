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

    helpers for symbol sequence or string from derivation (see bestPathString).
*/

#ifndef HYP__HYPERGRAPH__GETSTRING_HPP
#define HYP__HYPERGRAPH__GETSTRING_HPP
#pragma once

#include <sstream>
#include <utility>
#include <functional>

#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm/reverse.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Hypergraph/SymbolPrint.hpp>
//#include <sdl/Hypergraph/HypergraphPrint.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/Derivation.hpp>
#include <sdl/Hypergraph/PrintOptions.hpp>
#include <sdl/Util/StringBuilder.hpp>
#include <sdl/Hypergraph/SplitState.hpp>
#include <sdl/Hypergraph/DerivationStringOptions.hpp>

namespace sdl {
namespace Hypergraph {

// TODO: names are bad: shouldn't it be vice versa? getStringWt just adds the weight
template <class Arc>
std::string getString(IHypergraph<Arc> const& hg,
                      DerivationStringOptions const& opt = DerivationStringOptions(kUnquoted)) {
  return getStringWt(hg, opt).first;
}


/// Syms: vector<Sym>
template <class Arc>
Syms& symsFromStatesAppend(Syms& result, StateString const& ss, IHypergraph<Arc> const& hg,
                           WhichSymbolOptions const& opt = WhichSymbolOptions()) {
  for (StateString::const_iterator i = ss.begin(), e = ss.end(); i != e; ++i) {
    Sym sym = hg.label(*i, opt.labelType);
    if (sym && opt.shows(sym)) result.push_back(sym);
  }
  return result;
}

template <class Arc>
struct AppendSymsFromStates {
  Syms& result;
  IHypergraph<Arc> const& hg;
  WhichSymbolOptions const& opt;
  AppendSymsFromStates(Syms& result, IHypergraph<Arc> const& hg,
                       WhichSymbolOptions const& opt)
      : result(result), hg(hg), opt(opt) {}
  void push_back(StateId s) {
    Sym sym(hg.label(s, opt.labelType));
    if (sym && opt.shows(sym)) result.push_back(sym);
  }
};

template <class Arc>
Syms& symsFromDerivAppend(Syms& syms, typename Derivation<Arc>::child_type const& pDerivation,
                          IHypergraph<Arc> const& hg, WhichSymbolOptions const& opt = WhichSymbolOptions()) {
  if (pDerivation) {
    AppendSymsFromStates<Arc> append(syms, hg, opt);
    pDerivation->appendStates(append, hg.final(), opt.leafOnly);
  }
  return syms;
}

/// Syms: vector<Sym>
template <class Arc>
Syms symsFromStates(StateString const& ss, IHypergraph<Arc> const& hg,
                    DerivationStringOptions const& opt = DerivationStringOptions()) {
  Syms result;
  symsFromStatesAppend(result, ss, hg, opt);
  return result;
}

template <class Arc>
Syms symsFromDeriv(typename Derivation<Arc>::child_type const& pDerivation, IHypergraph<Arc> const& hg,
                   DerivationStringOptions const& opt = DerivationStringOptions()) {
  Syms result;
  symsFromDerivAppend(result, pDerivation, hg, opt);
  return result;
}

inline std::string textFromSyms(Syms const& str, IVocabularyPtr const& pVoc, char const* space = " ",
                                SymbolQuotation quote = kQuoted) {
  Util::StringBuilder out;
  print(out, str, pVoc, space, quote);
  return out.str();
}
inline Util::StringBuilder& textFromSyms(Util::StringBuilder& out, Syms const& str, IVocabularyPtr const& pVoc,
                                         char const* space = " ", SymbolQuotation quote = kQuoted) {
  print(out, str, pVoc, space, quote);
  return out;
}

inline std::string textFromSyms(Syms const& str, IVocabulary& voc, char const* space = " ",
                                SymbolQuotation quote = kQuoted) {
  Util::StringBuilder out;
  print(out, str, voc, space, quote);
  return out.str();
}

inline Util::StringBuilder& textFromSyms(Util::StringBuilder& out, Syms const& str, IVocabulary& voc,
                                         char const* space = " ", SymbolQuotation quote = kQuoted) {
  print(out, str, voc, space, quote);
  return out;
}


inline std::string textFromSyms(Syms const& str, IVocabulary& voc, DerivationStringOptions const& opt) {
  return textFromSyms(str, voc, opt.space.c_str(), opt.quote);
}
inline Util::StringBuilder& textFromSyms(Util::StringBuilder& out, Syms const& str, IVocabulary& voc,
                                         DerivationStringOptions const& opt) {
  print(out, str, voc, opt.space.c_str(), opt.quote);
  return out;
}
inline Util::StringBuilder& textFromSyms(Util::StringBuilder& out, SymSlice const& str, IVocabulary& voc,
                                         DerivationStringOptions const& opt) {
  print(out, str, voc, opt.space.c_str(), opt.quote);
  return out;
}


template <class Arc>
std::string textFromDeriv(typename Derivation<Arc>::child_type const& pDerivation, IHypergraph<Arc> const& hg,
                          DerivationStringOptions const& opt = DerivationStringOptions()) {
  return textFromSyms(symsFromDeriv(pDerivation, hg, opt), *hg.getVocabulary(), opt);
}
template <class Arc>
Util::StringBuilder& textFromDeriv(Util::StringBuilder& out,
                                   typename Derivation<Arc>::child_type const& pDerivation,
                                   IHypergraph<Arc> const& hg,
                                   DerivationStringOptions const& opt = DerivationStringOptions()) {
  return textFromSyms(out, symsFromDeriv(pDerivation, hg, opt), *hg.getVocabulary(), opt);
}


template <class Arc>
std::string textFromStates(StateString const& ss, IHypergraph<Arc> const& hg,
                           DerivationStringOptions const& opt = DerivationStringOptions()) {
  return textFromSyms(symsFromStates(ss, hg, opt), *hg.getVocabulary(), opt);
}
template <class Arc>
Util::StringBuilder& textFromStates(Util::StringBuilder& out, StateString const& ss, IHypergraph<Arc> const& hg,
                                    DerivationStringOptions const& opt = DerivationStringOptions()) {
  return textFromSyms(out, symsFromStates(ss, hg, opt), *hg.getVocabulary(), opt);
}


template <class Arc>
std::pair<std::string, typename Arc::Weight>
getStringWt(IHypergraph<Arc> const& hg, DerivationStringOptions const& opt = DerivationStringOptions(),
            char const* fallbackIfNoDerivation = "[no derivation exists]") {
  typedef typename Arc::Weight Weight;
  typedef std::pair<std::string, Weight> Ret;
  typename Derivation<Arc>::child_type deriv = singleDerivation(hg);
  if (!deriv) return Ret(fallbackIfNoDerivation, Weight::zero());
  return Ret(textFromSyms(symsFromDeriv(deriv, hg, opt), hg.getVocabulary(), opt.space.c_str(), opt.quote),
             deriv->weight());
}

struct PrintSyms {
  IVocabulary& voc;
  Syms const& syms;
  SymbolQuotation quote;
  PrintSyms(Syms const& syms, IVocabulary& voc, SymbolQuotation quote = kQuoted)
      : voc(voc), syms(syms), quote(quote) {}
  PrintSyms(Syms const& syms, IVocabularyPtr const& voc, SymbolQuotation quote = kQuoted)
      : voc(*voc), syms(syms), quote(quote) {}
  template <class Arc>
  PrintSyms(Syms const& syms, IHypergraph<Arc> const& voc, SymbolQuotation quote = kQuoted)
      : voc(*voc.getVocabulary()), syms(syms), quote(quote) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintSyms const& self) {
    self.print(out);
    return out;
  }
  void append(Util::StringBuilder& b) const { textFromSyms(b, syms, voc, quote); }
  void print(std::ostream& out) const {
    Util::StringBuilder b;
    append(b);
    b.print(out);
  }
};


}}

#endif
