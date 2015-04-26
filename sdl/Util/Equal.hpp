// Copyright 2014 SDL plc
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
#include <sdl/LexicalCast.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/AsciiCase.hpp>
#include <sdl/Util/Chomp.hpp>
#include <boost/regex.hpp>

namespace sdl {
namespace Util {

namespace detail {

/**
   strong typedef for different istream_iterator<string> behavior (get lines vs space separated tokens).
*/
struct Line : std::string {
  friend std::istream& operator>>(std::istream& in, Line& line) { return std::getline(in, line); }
};

struct LineNoTrailingSpaces : std::string {
  friend std::istream& operator>>(std::istream& in, LineNoTrailingSpaces& line) {
    std::getline(in, line);
    chomp(line);
    return in;
  }
};
}

typedef std::istream_iterator<detail::Line> LinesIterator;

inline LinesIterator linesBegin(std::istream& in) {
  return LinesIterator(in);
}

inline LinesIterator linesEnd() {
  return LinesIterator();
}

typedef std::istream_iterator<detail::LineNoTrailingSpaces> LinesNoTrailingSpacesIterator;

inline LinesNoTrailingSpacesIterator linesNoTrailingSpacesBegin(std::istream& in) {
  return LinesNoTrailingSpacesIterator(in);
}

inline LinesNoTrailingSpacesIterator linesNoTrailingSpacesEnd() {
  return LinesNoTrailingSpacesIterator();
}

/**
   Approximate equal, For floating-point comparisons
*/
template <class T>
bool floatEqual(T a, T b, T epsilon = 1e-6) {
  return a == b || std::fabs(a - b) < epsilon;
  // first check seems silly but helps with +inf depending on -ffast-math flags
}

template <class T>
bool floatEqualWarn(T a, T b, T epsilon = (T)1e-6, char const* first_name = "GOT",
                    char const* second_name = "REF") {
  bool const eq = floatEqual(a, b, epsilon);
  if (!eq)
    SDL_WARN(Util.floatEqual, '(' << a << " = " << first_name << ") != (" << second_name << " = " << b
                                  << ") with tolerance epsilon=" << epsilon);
  return eq;
}

typedef std::set<std::string> LinesUnordered;

inline LinesUnordered difference(LinesUnordered const& a, LinesUnordered const& b) {
  LinesUnordered a_minus_b;
  std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(a_minus_b, a_minus_b.end()));
  return a_minus_b;
}

/**
   \return whether sets of lines are the same.
*/
inline bool StreamLinesUnorderedEqual(std::istream& stream1, std::istream& stream2,
                                      char const* first_name = "GOT", char const* second_name = "REF") {
  LinesUnordered stream1_lines, stream2_lines;
  std::copy(linesNoTrailingSpacesBegin(stream1), linesNoTrailingSpacesEnd(),
            std::inserter(stream1_lines, stream1_lines.end()));
  std::copy(linesNoTrailingSpacesBegin(stream2), linesNoTrailingSpacesEnd(),
            std::inserter(stream2_lines, stream2_lines.end()));

  bool ok = stream1_lines == stream2_lines;

  if (!ok) {
    SDL_WARN(Util, "NOT (unordered) EQUAL:\n "
                   << first_name << ": {[(\n" << print(stream1_lines, multiLineNoBrace()) << "\n)]} "
                   << second_name << ": {(" << print(stream2_lines, multiLine()) << ")}\n\n difference "
                   << first_name << " - " << second_name << ": {"
                   << print(difference(stream1_lines, stream2_lines), multiLine()) << "} difference "
                   << second_name << " - " << first_name << ": {"
                   << print(difference(stream2_lines, stream1_lines), multiLine()));
  }

  return ok;
}

inline bool StreamLinesUnorderedEqual(std::istream& stream1, std::string const& str2,
                                      char const* first_name = "GOT", char const* second_name = "REF") {
  std::istringstream stream2(str2);
  return StreamLinesUnorderedEqual(stream1, stream2, first_name, second_name);
}

inline bool LinesUnorderedEqual(std::string const& str1, std::string const& str2,
                                char const* first_name = "GOT", char const* second_name = "REF") {
  std::istringstream stream1(str1);
  std::istringstream stream2(str2);
  return StreamLinesUnorderedEqual(stream1, stream2, first_name, second_name);
}

template <class Val1>
inline bool LinesUnorderedEqual(Val1 const& val1, std::string const& ref, char const* first_name = "GOT",
                                char const* second_name = "REF") {
  std::stringstream stream1;
  stream1 << val1;
  return StreamLinesUnorderedEqual(stream1, ref, first_name, second_name);
}

template <class Val1, class Val2>
inline bool PrintedLinesUnorderedEqual(Val1 const& val1, Val2 const& val2, char const* first_name = "GOT",
                                       char const* second_name = "REF") {
  std::stringstream stream1, stream2;
  stream1 << val1;
  stream2 << val2;
  return StreamLinesUnorderedEqual(stream1, stream2, first_name, second_name);
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

   (the last is a hacks for hypergraph output)

   perl regex:

   (?!abc) matches zero characters only if they are not followed by the expression "abc".

   (?<!pattern) consumes zero characters, only if pattern could not be matched
   against the characters preceding the current position (pattern must be of
   fixed length) (negative lookbehind: Patterns which start with negative
   lookbehind assertions may match at the beginning of the string being
   searched.)
*/

inline std::string ReplaceIntegersCopy(std::string const& str, std::string const& replaceIntegersBy = "#") {
  static boost::regex integerRegexp("(?<![-0-9.])[[:digit:]]+(?:(?![.0-9[])|$)");
  using namespace boost::regex_constants;
  return boost::regex_replace(str, integerRegexp, replaceIntegersBy, format_literal | format_all);
}

inline void ReplaceIntegers(std::string& str, std::string const& replaceIntegersBy = "#") {
  str = ReplaceIntegersCopy(str, replaceIntegersBy);
}

inline bool LinesUnorderedEqualIgnoringDigits(std::string str1, std::string str2,
                                              char const* first_name = "GOT", char const* second_name = "REF",
                                              char replaceDigitsBy = '#') {
  if (LinesUnorderedEqual(str1, str2, first_name, second_name)) return true;
  SDL_TRACE(Util.Equal, "not exactly equal - trying with digits replaced: [(test)" << str1 << " != " << str2
                                                                                   << "(reference)]\n");
  ReplaceDigits(str1, replaceDigitsBy);
  ReplaceDigits(str2, replaceDigitsBy);
  bool ok = LinesUnorderedEqual(str1, str2, first_name, second_name);
  if (ok)
    SDL_WARN(Util.Equal, " OK - Equal after digit replacement.");
  else
    SDL_WARN(Util.Equal, "not exactly equal even after digit replacement and ignoring line order: [(test)"
                         << str1 << "  !=  " << str2 << " (reference)]\n");
  return ok;
}

inline bool LinesUnorderedEqualIgnoringIntegers(std::string str1, std::string str2,
                                                char const* first_name = "GOT",
                                                char const* second_name = "REF",
                                                std::string const& replaceIntegersBy = "#") {
  if (LinesUnorderedEqual(str1, str2, first_name, second_name)) return true;
  SDL_TRACE(Util.Equal, "not exactly equal - trying with digits replaced: [(test)" << str1 << " != " << str2
                                                                                   << "(reference)]\n");
  ReplaceIntegers(str1, replaceIntegersBy);
  ReplaceIntegers(str2, replaceIntegersBy);
  bool ok = LinesUnorderedEqual(str1, str2, first_name, second_name);
  if (!ok)
    SDL_WARN(Util.Equal, "not exactly equal even after integer replacement and ignoring line order: [(test)"
                         << str1 << "  !=  " << str2 << " (reference)]\n");
  else
    SDL_WARN(Util.Equal, " OK - Equal after integer replacement.");
  return ok;
}

template <class Obj>
bool PrintedLinesUnorderedEqualIgnoringDigits(Obj const& obj, std::string const& ref,
                                              char const* first_name = "GOT", char const* second_name = "REF",
                                              char replaceDigitsBy = '#') {
  std::stringstream ss;
  ss << obj;
  return LinesUnorderedEqualIgnoringDigits(ss.str(), ref, first_name, second_name, replaceDigitsBy);
}

template <class Obj>
bool PrintedLinesUnorderedEqualIgnoringIntegers(Obj const& obj, std::string const& ref,
                                                char const* first_name = "GOT",
                                                char const* second_name = "REF",
                                                std::string const& replaceIntegersBy = "#") {
  std::stringstream ss;
  ss << obj;
  return LinesUnorderedEqualIgnoringIntegers(ss.str(), ref, first_name, second_name, replaceIntegersBy);
}

template <class Obj, class Obj2>
bool PrintedLinesUnorderedEqualIgnoringIntegers(Obj const& obj, Obj2 const& obj2,
                                                char const* first_name = "GOT",
                                                char const* second_name = "REF",
                                                std::string const& replaceIntegersBy = "#") {
  std::stringstream ss;
  ss << obj;
  std::stringstream ss2;
  ss2 << obj2;
  return LinesUnorderedEqualIgnoringIntegers(ss.str(), ss2.str(), first_name, second_name, replaceIntegersBy);
}

/**
   \return true if the object (printed using operator<<) equals the
   string.
*/
template <class T>
bool isStrEqual(T const& obj, std::string const& str, char const* nameObj = "actual",
                char const* nameStr = "expected") {
  std::stringstream ss;
  ss << obj;
  bool ok = str == ss.str();
  if (!ok) {
    SDL_WARN(Util, "[" << nameObj << "] NOT EQUAL [" << nameStr << "]:\n"
                       << "  " << ss.str() << " [" << nameObj << "]\n"
                       << "  " << str << " [" << nameStr << ']');
  }
  return ok;
}

template <class T>
bool byStrEqual(T const& obj, T const& obj2, char const* name = "", char const* name2 = "") {
  std::stringstream ss;
  ss << obj2;
  return isStrEqual(obj, ss.str(), name, name2);
}

/**
   \return whether lists of lines are the same ignoring leading/trailing space differences
*/
inline bool StreamLinesEqual(std::istream& stream1, std::istream& stream2, char const* first_name = "GOT",
                             char const* second_name = "REF") {
  std::vector<std::string> stream1_lines, stream2_lines;
  std::copy(linesNoTrailingSpacesBegin(stream1), linesNoTrailingSpacesEnd(), std::back_inserter(stream1_lines));
  std::copy(linesNoTrailingSpacesBegin(stream2), linesNoTrailingSpacesEnd(), std::back_inserter(stream2_lines));

  bool ok = stream1_lines == stream2_lines;

  return ok;
}

template <class Val1, class Val2>
inline bool PrintedLinesEqual(Val1 const& val1, Val2 const& val2, char const* first_name = "GOT",
                              char const* second_name = "REF") {
  std::stringstream stream1, stream2;
  stream1 << val1;
  stream2 << val2;
  return StreamLinesUnorderedEqual(stream1, stream2, first_name, second_name);
}
}
}


#define SDL_ARE_EQUAL_STR(obj, obj2) sdl::Util::byStrEqual(obj, obj2, #obj, #obj2)
#define SDL_REQUIRE_EQUAL_STR(obj, obj2) BOOST_REQUIRE(SDL_ARE_EQUAL_STR(obj, obj2))
#define SDL_CHECK_EQUAL_STR(obj, obj2) BOOST_CHECK(SDL_ARE_EQUAL_STR(obj, obj2))

#endif
