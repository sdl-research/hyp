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

    some primitive types.

    and Slice, which is a pair of char ptrs (see also Util/Fields.hpp) into a
    std::string, c-string, vector<char> etc.
*/

#ifndef SDL_TYPES_HPP
#define SDL_TYPES_HPP
#pragma once


#if defined(__GNUC__) && defined(__LP64__) /* only under 64 bit gcc */
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
// http://www.win.tue.nl/~aeb/linux/misc/gcc-semibug.html
#endif

#include <sdl/Function.hpp>

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <cstring>
#include <sdl/IntTypes.hpp>
#include <sdl/LexicalCast.hpp>
#include <sdl/Array.hpp>
#include <sdl/Position.hpp>
#include <algorithm>

// Default precision of floating point values is 32-bit (float).
#ifndef SDL_FLOAT
#define SDL_FLOAT 32
#endif

namespace sdl {

#if SDL_64BIT_FEATURE_ID
typedef uint64 FeatureId;
#else
typedef uint32 FeatureId;
#endif

typedef std::vector<std::string> Strings;

typedef std::size_t SegmentId;
SegmentId const kNullSegmentId((SegmentId)-1);

typedef char const* Pchar;

// TODO: use boost::string_ref or std::experimental::string_span instead in C++11
typedef std::pair<Pchar, Pchar> Slice;  // see Util/Fields for a richer interface

inline std::size_t len(Slice const& slice) {
  return (std::size_t)(slice.second - slice.first);
}

inline bool empty(Slice const& slice) {
  return slice.second == slice.first;
}

/// not valid after str is destroyed
inline Slice toSlice(std::string const& str) {
  return Slice(arrayBegin(str), arrayEnd(str));
}

inline Slice toSlice(std::vector<char> const& str) {
  return Slice(arrayBegin(str), arrayEnd(str));
}

inline std::string& assignSlice(std::string& to, Slice from) {
  to.assign(from.first, from.second - from.first);
  return to;
}

inline Slice toSlice(Pchar str, std::size_t len) {
  return Slice(str, str + len);
}

inline Slice toSlice(Pchar str) {
  return Slice(str, str + std::strlen(str));
}

inline Slice toSlice(Pchar data, std::size_t begin, std::size_t end) {
  return Slice(data + begin, data + end);
}

inline Slice toSlice(std::string const& str, std::size_t begin, std::size_t end) {
  Pchar data = arrayBegin(str);
  return Slice(data + begin, data + end);
}

typedef uint32 Unicode;  // a unicode codepoint (character, not necessarily a single glyph, e.g. roman numeral
// three (looks like iii) is a single codepoint
typedef Unicode const* Punicode;
typedef std::pair<Punicode, Punicode> UnicodeSlice;
typedef std::vector<Unicode> Unicodes;

typedef wchar_t const* Pwchar;
typedef std::pair<Pwchar, Pwchar> WideSlice;

#if SDL_FLOAT == 32
typedef float SdlFloat;
// For debug messages:
inline std::string getSdlFloatName() {
  return "float";
}

#elif SDL_FLOAT == 64
typedef double SdlFloat;
// For debug messages:
inline std::string getSdlFloatName() {
  return "double";
}

#else
#error "SDL supports single (float; SDL_FLOAT=32) or double (double; SDL_FLOAT=64) precision only."
#endif

typedef SdlFloat LmCostVal;
typedef SdlFloat Cost;

/**
   Cast any float type to SdlFloat. Just a convenience
   function to avoid having to type static_cast<SdlFloat>(f) all
   over the place (type floatCast(f) instead).
*/
template <class FloatT>
inline SdlFloat floatCast(FloatT const& f) {
  return static_cast<SdlFloat>(f);
}

// TODO: prefer Slice = pair<char const*, std::size_t> ? faster equalContents for hash maps
inline bool equalContents(Slice const& a, Slice const& b) {
  std::size_t const lena = (std::size_t)(a.second - a.first);
  std::size_t const lenb = (std::size_t)(b.second - b.first);
  return lena == lenb && !std::memcmp(a.first, b.first, lena);
}

inline std::size_t hashContents(Slice a) {
  // from g_str_hash X31_HASH
  std::size_t h = 0;
  for (; a.first < a.second; ++a.first)
    h = 31 * h + *a.first++;  // 31 * h should optimize to ( h << 5 ) - h if faster
  return h;
}

struct SliceContents {
  bool operator()(Slice const& a, Slice const& b) const { return equalContents(a, b); }
  std::size_t operator()(Slice const& a) const { return hashContents(a); }
};
}


namespace std {
// ADL won't find in namespace sdl::xmt (typedef)
inline std::ostream& operator<<(std::ostream& out, sdl::Slice const& word) {
  out.write(word.first, word.second - word.first);
  return out;
}


}

#endif
