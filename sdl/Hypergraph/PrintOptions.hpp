// Copyright 2014 SDL plc
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

    options for printing of symbols, sentences, derivations, etc.

    (see SymbolPrint.hpp for code that prints using these)
*/

#ifndef HYPERGRAPH_PRINT__JG_2013_06_13_HPP
#define HYPERGRAPH_PRINT__JG_2013_06_13_HPP
#pragma once

#include <sdl/Sym.hpp>
#include <sdl/Util/Enum.hpp>

namespace sdl {
namespace Hypergraph {

SDL_ENUM(SymbolQuotation, 2, (Unquoted, Quoted))
SDL_ENUM(SymbolTypes, 2, (AllSymbolTypes, Lexical_Only))
SDL_ENUM(SymbolEpsilonSkip, 2, (Keep_Epsilon, Skip_Epsilon))
SDL_ENUM(SymbolBlockSkip, 2, (Keep_Block, Skip_Block))

}}

#endif
