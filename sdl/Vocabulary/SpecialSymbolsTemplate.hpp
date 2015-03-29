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
#if __cplusplus >= 201103L || CPP11
  static constexpr Sym ID = {Symbol::id};
#else
  static Sym const ID;  // = { Symbol::id };
#endif
};
#if __cplusplus >= 201103L || CPP11
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
