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

    mostly for unit testing - compare objects via string representation.
*/

#ifndef SDL_UTIL_EQUAL_HPP
#define SDL_UTIL_EQUAL_HPP
#pragma once

#include <cmath>
#include <set>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <utility>
#include <sdl/LexicalCast.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/AsciiCase.hpp>
#include <sdl/Util/Math.hpp>
#include <sdl/Util/Chomp.hpp>
#include <graehl/shared/split.hpp>

namespace sdl {
namespace Util {

template <class T>
bool floatEqualWarn(T v1, T v2, T epsilon = (T)1e-6, char const* name1 = "GOT", char const* name2 = "REF") {
  bool const eq = floatEqual(v1, v2, epsilon);
  if (!eq)
    SDL_WARN(Util.floatEqual, '(' << v1 << " = " << name1 << ") != (" << name2 << " = " << v2
                                  << ") with tolerance epsilon=" << epsilon);
  return eq;
}

using graehl::chomped_lines;

/**
   \return whether sets of lines are the same.
*/
bool StringsUnorderedEqual(std::vector<std::string> lines1, std::vector<std::string> lines2,
                           bool sortWords = false, char const* name1 = "GOT", char const* name2 = "REF",
                           bool warn = true, bool ignoreBlankLines = true, bool sortLines = true);

template <class S1, class S2>
inline bool LinesEqual(S1&& s1, S2&& s2, bool sortLines = false, bool sortWords = false,
                       char const* name1 = "GOT", char const* name2 = "REF", bool warn = true,
                       bool ignoreBlankLines = true) {
  return StringsUnorderedEqual(chomped_lines(std::forward<S1>(s1)), chomped_lines(std::forward<S2>(s2)),
                               sortWords, name1, name2, warn, ignoreBlankLines, sortLines);
}

inline bool LinesUnorderedEqual(std::istream& stream1, std::istream& stream2, bool sortWords = false,
                                char const* name1 = "GOT", char const* name2 = "REF", bool warn = true,
                                bool ignoreBlankLines = true) {
  return StringsUnorderedEqual(chomped_lines(stream1), chomped_lines(stream2), sortWords, name1, name2, warn,
                               ignoreBlankLines);
}

inline bool LinesUnorderedEqual(std::istream& stream1, std::string const& lines2, bool sortWords = false,
                                char const* name1 = "GOT", char const* name2 = "REF", bool warn = true,
                                bool ignoreBlankLines = true) {
  return StringsUnorderedEqual(chomped_lines(stream1), graehl::chomped_lines(lines2), sortWords, name1, name2,
                               warn);
}

inline bool LinesUnorderedEqual(std::string const& str1, std::string const& str2, bool sortWords = false,
                                char const* name1 = "GOT", char const* name2 = "REF", bool warn = true,
                                bool ignoreBlankLines = true) {
  return StringsUnorderedEqual(chomped_lines(str1), chomped_lines(str2), sortWords, name1, name2, warn,
                               ignoreBlankLines);
}

template <class Val1, class If1 = typename not_istream_or_string<Val1>::type>
bool LinesUnorderedEqual(Val1 const& val1, std::string const& val2, bool sortWords = false,
                         char const* name1 = "GOT", char const* name2 = "REF", bool warn = true,
                         bool ignoreBlankLines = true) {
  std::stringstream stream1;
  stream1 << val1;
  return StringsUnorderedEqual(chomped_lines(stream1), chomped_lines(val2), sortWords, name1, name2, warn,
                               ignoreBlankLines);
}

template <class Val1, class Val2, class If1 = typename not_istream_or_string<Val1>::type,
          class If2 = not_istream_or_string_t<Val2>>
bool LinesUnorderedEqual(Val1 const& val1, Val2 const& val2, bool sortWords = false,
                         char const* name1 = "GOT", char const* name2 = "REF", bool warn = true,
                         bool ignoreBlankLines = true) {
  std::stringstream stream1;
  stream1 << val1;
  std::stringstream stream2;
  stream1 << val2;
  return StringsUnorderedEqual(chomped_lines(stream1), chomped_lines(stream2), sortWords, name1, name2, warn,
                               ignoreBlankLines);
}

inline void ReplaceDigits(std::string& str, char replaceDigitsBy = '#') {
  for (std::string::iterator i = str.begin(), e = str.end(); i != e; ++i)
    if (isAsciiDigit(*i)) *i = replaceDigitsBy;
}

/**
   return str with runs of digits replaced by 'replaceIntegersBy', so long as
   the digits are not:

   immediately preceeded or followed by a decimal point '.',

   preceded by '-'

   followed by end of line or '['

   (the last is a hack for comparing hypergraph output)
*/
std::string ReplacedIntegers(std::string const& str, std::string const& replaceIntegersBy = "#");

inline bool LinesUnorderedEqualIgnoringDigits(std::string str1, std::string str2, bool sortWords = false,
                                              char const* name1 = "GOT", char const* name2 = "REF",
                                              bool warn = true, bool ignoreBlankLines = true,
                                              char replaceDigitsBy = '#') {
  if (LinesUnorderedEqual(str1, str2, sortWords, name1, name2, false)) return true;
  SDL_TRACE(Util.Equal, "not exactly equal - trying with digits replaced: [(test)" << str1 << " != " << str2
                                                                                   << "(reference)]\n");
  ReplaceDigits(str1, replaceDigitsBy);
  ReplaceDigits(str2, replaceDigitsBy);
  bool ok = LinesUnorderedEqual(str1, str2, sortWords, name1, name2, warn, ignoreBlankLines);
  if (ok)
    SDL_WARN(Util.Equal, " OK-Equal after digit replacement.");
  else
    SDL_WARN(Util.Equal, "not exactly equal even after digit replacement and ignoring line order: [(test)"
                             << str1 << "  !=  " << str2 << " (reference)]\n");
  return ok;
}

inline bool LinesUnorderedEqualIgnoringIntegers(std::string i1, std::string i2, bool sortWords = false,
                                                char const* name1 = "GOT", char const* name2 = "REF",
                                                bool warn = true, bool ignoreBlankLines = true,
                                                std::string const& replaceIntegersBy = "#") {
  if (LinesUnorderedEqual(i1, i2, sortWords, name1, name2, false, ignoreBlankLines)) return true;
  SDL_TRACE(Util.Equal, "not exactly equal - trying with digits replaced: [(test)" << i1 << " != " << i2
                                                                                   << "(reference)]\n");
  std::string str1 = ReplacedIntegers(i1, replaceIntegersBy);
  std::string str2 = ReplacedIntegers(i2, replaceIntegersBy);
  bool ok = LinesUnorderedEqual(str1, str2, sortWords, name1, name2, warn, ignoreBlankLines);
  if (!ok)
    SDL_WARN(Util.Equal, "not exactly equal even after integer replacement and ignoring line order: [(test)\n"
                             << str1 << "\n  !=  \n" << str2 << "\n (reference)]:\n\n" << i1
                             << "\n (original test).");
  else
    SDL_WARN(Util.Equal, " OK-Equal after integer replacement.");
  return ok;
}

template <class Val1>
bool LinesUnorderedEqualIgnoringDigits(Val1 const& val1, std::string const& ref, bool sortWords = false,
                                       char const* name1 = "GOT", char const* name2 = "REF", bool warn = true,
                                       bool ignoreBlankLines = true, char replaceDigitsBy = '#',
                                       typename not_istream_or_string<Val1>::type* = 0) {
  std::stringstream ss;
  ss << val1;
  return LinesUnorderedEqualIgnoringDigits(ss.str(), ref, sortWords, name1, name2, warn, ignoreBlankLines,
                                           replaceDigitsBy);
}

template <class Val1>
bool LinesUnorderedEqualIgnoringIntegers(Val1 const& val1, std::string const& ref, bool sortWords = false,
                                         char const* name1 = "GOT", char const* name2 = "REF",
                                         bool warn = true, bool ignoreBlankLines = true,
                                         std::string const& replaceIntegersBy = "#",
                                         typename not_istream_or_string<Val1>::type* = 0) {
  std::stringstream ss;
  ss << val1;
  return LinesUnorderedEqualIgnoringIntegers(ss.str(), ref, sortWords, name1, name2, warn, ignoreBlankLines,
                                             replaceIntegersBy);
}

template <class Val1, class Val2>
bool LinesUnorderedEqualIgnoringIntegers(Val1 const& val1, Val2 const& val2, bool sortWords = false,
                                         char const* name1 = "GOT", char const* name2 = "REF",
                                         bool warn = true, bool ignoreBlankLines = true,
                                         std::string const& replaceIntegersBy = "#",
                                         typename not_istream_or_string<Val1>::type* = 0,
                                         typename not_istream_or_string<Val2>::type* = 0) {
  std::stringstream ss;
  ss << val1;
  std::stringstream ss2;
  ss2 << val2;
  return LinesUnorderedEqualIgnoringIntegers(ss.str(), ss2.str(), sortWords, name1, name2, warn,
                                             ignoreBlankLines, replaceIntegersBy);
}

/**
   \return true if the object (printed using operator<<) equals the
   string.
*/
template <class T>
bool isStrEqual(T const& val1, std::string const& val2, char const* name1 = "GOT",
                char const* name2 = "REF") {
  std::stringstream ss;
  ss << val1;
  bool ok = val2 == ss.str();
  if (!ok) {
    SDL_WARN(Util, "[" << name1 << "] NOT EQUAL [" << name2 << "]:\n"
                       << "  " << ss.str() << " [" << name1 << "]\n"
                       << "  " << val2 << " [" << name2 << ']');
  }
  return ok;
}

template <class T>
bool byStrEqual(T const& val1, T const& val2, char const* name1 = "", char const* name2 = "") {
  std::stringstream ss;
  ss << val2;
  return isStrEqual(val1, ss.str(), name1, name2);
}

void normalizeLine(std::string& line, bool sortWords, char wordsep = ' ');


}}

#endif
