// Copyright 2014 SDL plc
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

    objects causing initialization of constant special-symbols string/id

    TODO: use compile-time constants
*/

#ifndef SPECIALSYMBOLSTEMPLATE_HPP_
#define SPECIALSYMBOLSTEMPLATE_HPP_
#pragma once

#include <sdl/Sym.hpp>
#include <sdl/Vocabulary/SpecialSymbolVocab.hpp>

namespace sdl {
namespace Vocabulary {


template <typename Symbol>
struct SpecialSymbolTemplate {
  static std::string const TOKEN;
  static Sym const ID_LINKED;
#if __cplusplus >= 201103L
  static constexpr Sym ID = {Symbol::id};
#else
  static Sym const ID;  // = { Symbol::id };
#endif
};
#if __cplusplus >= 201103L
template <typename Symbol>
Sym constexpr SpecialSymbolTemplate<Symbol>::ID;
#else
template <typename Symbol>
Sym const SpecialSymbolTemplate<Symbol>::ID = {Symbol::id};
#endif

/**
   this is the *only* place SpecialSymbolVocab is used during static init. the
   specialSymbolsForceInit() call is idempotent (return a ref to the initialized
   specialSymbols singleton).
*/
template <typename Symbol>
Sym const SpecialSymbolTemplate<Symbol>::ID_LINKED
    = specialSymbolsForceInit().addAssertId(Symbol::str(), Symbol::id);

template <typename Symbol>
std::string const SpecialSymbolTemplate<Symbol>::TOKEN(Symbol::str());


}}

#endif
