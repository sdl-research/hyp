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
  static Sym const ID;
  static std::string const TOKEN;
};

/**
   this is the *only* place SpecialSymbolVocab is used during static init. the
   specialSymbolsForceInit() call is idempotent (return a ref to the initialized
   specialSymbols singleton).
*/
template <typename Symbol>
Sym const SpecialSymbolTemplate<Symbol>::ID
    = specialSymbolsForceInit().add(Symbol::str(), Symbol::type());

template <typename Symbol>
std::string const SpecialSymbolTemplate<Symbol>::TOKEN = Symbol::str();


}}

#endif
