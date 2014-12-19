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
