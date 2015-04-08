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

    pointer (to >1 byte size object) or integer in a single pointer-sized int

    lsb = least significant bit. you can hide those bits in a pointer to a value of
    size>1, which will necessarily be aligned and even.

    otherwise a bool flag takes at least 4 bytes, effectively. an alternative which
    is less crazy, but harder to use, is a parallel bit vector (two arrays with same
    integer index).
*/

#ifndef POINTERWITHFLAG_LW2012419_HPP
#define POINTERWITHFLAG_LW2012419_HPP
#pragma once


#include <stdint.h> // intptr_t is here until C++11
#include <climits>
#include <cassert>

namespace sdl { namespace Util {

///requires sizeof(T)>1 !
template <class T>
bool lsb(T *t) {
  return (intptr_t)t & 1;
}

inline void * pointerForInteger(std::size_t i) {
  return (void *)((i << 1) | 1);
}

template <class T>
T * specificPointerForInteger(std::size_t i) {
  return (T*)((i << 1) | 1);
}

template <class Int>
inline bool msb(Int i) {
  return i & ((Int)0x80u << (CHAR_BIT * (sizeof(Int) - 1)));
}

template <class T>
std::size_t integerFromPointer(T *t) {
#ifdef NDEBUG
  return (std::size_t)t >> 1;
#else
  assert(lsb(t));
  std::size_t const i = (std::size_t)t >> 1;
  assert(pointerForInteger(i) == t);
  assert(!msb(i));
  return i;
#endif
}

template <class T>
T *withoutLsb(T *t) {
  return (T*)((intptr_t)t & ~(intptr_t)1);
}

template <class T>
T *withLsb(T *t, intptr_t lsb = 1) {
  return (T*)((intptr_t)t | lsb);
}

template <class T>
T *withLsbs(T *t, unsigned lsbs) {
  return (T*)((intptr_t)t | lsbs);
}

template <class T>
unsigned lsbs(T *t, unsigned mask) {
  return (intptr_t)t & mask;
}

//TODO: use fancy boost enable stuff, or template on T rather than T*? this is an exact duplicate of above but with T const*
// I don't just template for T t because this may be inappropriate for integral or other value types

///requires sizeof(T)>1 !
template <class T>
bool lsb(T const* t) {
  return (intptr_t)t & 1;
}

template <class T>
T const* withoutLsb(T const* t) {
  return (T*)((intptr_t)t & ~(intptr_t)1);
}

template <class T>
T const* withLsb(T const* t, intptr_t lsb = 1) {
  return (T*)((intptr_t)t | lsb);
}

template <class T>
T const* withLsbs(T const* t, unsigned lsbs) {
  return (T*)((intptr_t)t | lsbs);
}

template <class T>
unsigned lsbs(T const* t, unsigned mask) {
  return (intptr_t)t & mask;
}


}}

#endif
