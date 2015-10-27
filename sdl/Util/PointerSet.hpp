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

    hashed set of pointers. we discard low bits (usually we're pointing at
    things that are 8 bytes or more, so we drop 3 bits at least)
*/

#ifndef POINTERSET_JG_2014_12_16_HPP
#define POINTERSET_JG_2014_12_16_HPP
#pragma once

#define SDL_POINTER_SET_GOOGLE_HASH 1

#include <sdl/Util/Unordered.hpp>
#include <sdl/Sym.hpp>
#include <graehl/shared/int_types.hpp>

#if defined(WIN32)
#if !defined(_INTPTR_T_DEFINED)
typedef INT_PTR intptr_t;
#define _INTPTR_T_DEFINED
#endif
#else
#include <stdint.h>
#endif

namespace sdl {
namespace Util {

struct DiscardConstantLowPointerBits {
  inline std::size_t operator()(intptr_t x) const { return x >> 3; }
  inline std::size_t operator()(std::size_t x) const { return x >> 3; }
};

#if SDL_POINTER_SET_GOOGLE_HASH
typedef hash_set<intptr_t, DiscardConstantLowPointerBits> PointerSet;
#else
typedef unordered_set<intptr_t, DiscardConstantLowPointerBits> PointerSet;
#endif

/// use exact value size info to get more-significant bits
template <class P>
struct PtrDiffHash {
  std::size_t operator()(P const* p) const { return p - (P const*)0; }
  std::size_t operator()(intptr_t p) const { return (P const*)p - (P const*)0; }
  std::size_t operator()(std::size_t p) const { return (P const*)p - (P const*)0; }
};

template <std::size_t sz>
struct DiscardPointerLsbsHash {
  static constexpr std::size_t ceil_log2_size = graehl::ceil_log2_const(sz);
  static_assert((1 << ceil_log2_size) >= sz && (1 << (ceil_log2_size + 1)) > sz, "error in ceil_log2_size(sz)");
  std::size_t operator()(void const* p) const {
    return operator()((std::size_t)p);
  }
  std::size_t operator()(std::size_t p) const { return p >> ceil_log2_size; }
  std::size_t operator()(intptr_t p) const { return operator()((std::size_t)p); }
};

struct EqualPointer {
  bool operator()(void const* a, void const* b) const {
    return a == b;
  }
  bool operator()(std::size_t a, std::size_t b) const {
    return a == b;
  }
  bool operator()(intptr_t a, intptr_t b) const {
    return a == b;
  }
};

template <class P>
using pointer_set = sdl::hash_set<void *, DiscardPointerLsbsHash<sizeof(P)>, EqualPointer>;

template <class P, class Val>
using pointer_hash_map = sdl::hash_map<void *, Val, DiscardPointerLsbsHash<sizeof(P)>, EqualPointer>;

template <class P, class Val>
using pointer_unordered_map = sdl::unordered_map<void *, Val, DiscardPointerLsbsHash<sizeof(P)>, EqualPointer>;


}}

#endif
