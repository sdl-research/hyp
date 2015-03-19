/** \file

    hashed set of pointers.
*/

#ifndef POINTERSET_JG_2014_12_16_HPP
#define POINTERSET_JG_2014_12_16_HPP
#pragma once

#include <sdl/Util/Unordered.hpp>
#include <sdl/Sym.hpp>

#if defined(WIN32)
#if !defined(_INTPTR_T_DEFINED)
typedef INT_PTR intptr_t;
#define _INTPTR_T_DEFINED
#endif
#else
#include <stdint.h>
#endif

namespace sdl { namespace Util {

typedef unordered_set<intptr_t> PointerSet;

template <class P>
struct PtrDiffHash {
  std::size_t operator()(P const* p) const { return p - (P const*)0; }
};

}}

#endif
