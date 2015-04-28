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

    a Label is a pair of input, output Sym. enum options type for selecting
    which labels to operate on (some algorithms are fixed for now to only
    operate on input labels or output labels, though).
*/

#ifndef HYP__HYPERGRAPH__LABEL_HPP
#define HYP__HYPERGRAPH__LABEL_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Hypergraph/LabelPair.hpp>
#include <sdl/Util/Enum.hpp>

namespace sdl {
namespace Hypergraph {

/**
   in this order, No_Label is 0, and Input_Output_Label == 3 == 1|2 = Input | Output.

   this is important; don't change the order
*/
SDL_ENUM(LabelType, 4, (No_Label, Input, Output, Input_Output_Label))

inline LabelPair makeLabelPair(Sym i = EPSILON::ID, Sym o = NoSymbol) {
  return LabelPair(i, o);
}

inline LabelPair labelPair(Sym i = EPSILON::ID, Sym o = NoSymbol) {
  return LabelPair(i, o);
}

static inline void appendSyms(Syms& labels, Sym sym, bool skipEpsilon = true) {
  if (!(skipEpsilon && sym == EPSILON::ID)) labels.push_back(sym);
}

static inline void appendSyms(Syms& labels, LabelPair const& labelPair, bool skipEpsilon = true,
                              LabelType inputOrOutput = kOutput) {
  if (inputOrOutput & kInput) appendSyms(labels, labelPair.first, skipEpsilon);
  if (inputOrOutput & kOutput) appendSyms(labels, labelPair.second, skipEpsilon);
}

inline bool hasLexicalLabel(LabelPair const& labelPair) {
  return labelPair.first.isLexical() || labelPair.second.isLexical();
}


}}

#endif
