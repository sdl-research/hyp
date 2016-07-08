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

    provide a nicer interface than std::pair for LabelPair.
*/

#ifndef HYP__HYPERGRAPH_LABELPAIR_HPP
#define HYP__HYPERGRAPH_LABELPAIR_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <boost/functional/hash.hpp>
#include <utility>

namespace sdl {
namespace Hypergraph {

typedef std::pair<Sym, Sym> LabelPair;

inline Sym input(LabelPair const& l) {
  return l.first;
}
inline Sym output(LabelPair const& l) {
  return l.second;
}
inline Sym& input(LabelPair& l) {
  return l.first;
}
inline Sym& output(LabelPair& l) {
  return l.second;
}
inline Sym effectiveOutput(LabelPair const& l) {
  return l.second ? l.second : l.first;
}
inline void setOutputAsInput(LabelPair& l) {
  l.second = NoSymbol;
}
inline void setInputAsOutput(LabelPair& l) {
  l.first = l.second;
  l.second = NoSymbol;
}


namespace {
LabelPair kEpsilonLabelPair(EPSILON::ID, EPSILON::ID);
// TODO: EPSILON::ID is static init and non-const; let's make special symbol ::ID a true constant for
// link-safety
LabelPair kNullLabelPair(NoSymbol, NoSymbol);
}

/// use instead of kEpsilonLabelPair for static usage
inline LabelPair const& getEpsilonLabelPair() {
  assert(kEpsilonLabelPair.first == EPSILON::ID);
  assert(kEpsilonLabelPair.second == EPSILON::ID);
  return kEpsilonLabelPair;
}

inline bool isEpsilonLabelPair(LabelPair const& l) {
  return l.first == EPSILON::ID && l.second == EPSILON::ID;
}

inline Sym outputElse(LabelPair const& l) {
  Sym o = output(l);
  return o == NoSymbol ? input(l) : o;
}

/// \return a==b by input and effectiveOutput
inline bool compatible(LabelPair const& a, LabelPair const& b) {
  return a.first == b.first
         && (a.second == b.second || (a.second ? (!b.second && b.first == a.second) : a.first == b.second));
}


}}

#endif
