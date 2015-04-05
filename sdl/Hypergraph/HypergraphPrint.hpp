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
/** \file

    print labels given hg's vocab - formerly part of SymbolPrint but split to
    remove IHypergraph dependency.
*/

#ifndef HYPERGRAPHPRINT_JG_2013_04_24_HPP
#define HYPERGRAPHPRINT_JG_2013_04_24_HPP
#pragma once

#include <sdl/Hypergraph/SymbolPrint.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>

namespace sdl { namespace Hypergraph {

template <class A>
void print(std::ostream &out, Syms const& string, IHypergraph<A> const& vocHg
           , char const* space=" ", SymbolQuotation quote = kQuoted)
{
  print(out, string, vocHg.getVocabulary(), space, quote);
}
template <class A>
void print(Util::StringBuilder &out, Syms const& string, IHypergraph<A> const& vocHg
           , char const* space=" ", SymbolQuotation quote = kQuoted)
{
  print(out, string, vocHg.getVocabulary(), space, quote);
}


template <class A>
void print(std::ostream &out, LabelPair const& labelPair, IHypergraph<A> const& vocHg
           , SymbolQuotation quote = kQuoted) {
  print(out, labelPair, vocHg.getVocabulary(), quote);
}
template <class A>
void print(Util::StringBuilder &out, LabelPair const& labelPair, IHypergraph<A> const& vocHg
           , SymbolQuotation quote = kQuoted) {
  print(out, labelPair, vocHg.getVocabulary(), quote);
}


}}

#endif
