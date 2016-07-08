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

    defines integral types in sdl::xmt. this should be as convenient and more
    correct than the malpractice of relying on C-include no-namespace typedefs
    (that may not be the same on unix and windows)
*/

#ifndef INTTYPES_JG_2013_05_31_HPP
#define INTTYPES_JG_2013_05_31_HPP
#pragma once

#include <graehl/shared/is_null.hpp>

#if CPP11
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifndef UINT64_DIFFERENT_FROM_SIZE_T
#define UINT64_DIFFERENT_FROM_SIZE_T 0
#endif

namespace sdl {

typedef unsigned SymInt;

#if CPP11
typedef std::uint64_t uint64;
typedef std::uint32_t uint32;
typedef std::uint16_t uint16;
typedef std::uint8_t uint8;

typedef std::int64_t int64;
typedef std::int32_t int32;
typedef std::int16_t int16;
typedef std::int8_t int8;
#else
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
#endif

typedef std::size_t Size;

#if CPP11
typedef std::uintptr_t uintptr;
#else
typedef uintptr_t uintptr;
#endif

/// \return number of bytes needed to represent val with implicit 0 bits (most
/// significant bits, that is)
template <typename Type>
inline unsigned getNumBytes(Type val) {
  if (sizeof(val) == 8)
    return (val & 0xffffffff00000000ull)
               ? (val & 0xffff000000000000ull ? (val & 0xff00000000000000ull ? 8 : 7)
                                              : (val & 0xff0000000000ull ? 6 : 5))
               : (val & 0xffff0000u ? (val & 0xff000000u ? 4 : 3) : (val & 0xff00u ? 2 : 1));
  else if (sizeof(val) == 4)
    return val & 0xffff0000u ? (val & 0xff000000u ? 4 : 3) : (val & 0xff00u ? 2 : 1);
  else if (sizeof(val) == 2)
    return val & 0xff00u ? 2 : 1;
  else {
    unsigned n = 0;
    do {
      val >>= 8;
      n++;
    } while (val);
    return n;
  }
}


}

#endif
