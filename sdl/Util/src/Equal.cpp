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
#include <sdl/Util/Equal.hpp>
#include <boost/regex.hpp>

namespace sdl {
namespace Util {


typedef std::set<std::string> LinesUnordered;

LinesUnordered difference(LinesUnordered const& a, LinesUnordered const& b) {
  LinesUnordered a_minus_b;
  std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(a_minus_b, a_minus_b.end()));
  return a_minus_b;
}

bool StreamLinesUnorderedEqual(std::istream& stream1, std::istream& stream2, char const* first_name,
                                      char const* second_name, bool warn) {
  LinesUnordered stream1_lines, stream2_lines;
#define SDL_EQUAL_MSG()                                                                                 \
  "NOT (unordered) EQUAL:\n " << first_name << ": {[(\n" << print(stream1_lines, multiLineNoBrace())    \
                              << "\n)]} " << second_name << ": {(" << print(stream2_lines, multiLine()) \
                              << ")}\n\n difference " << first_name << " - " << second_name << ": {"    \
                              << print(difference(stream1_lines, stream2_lines), multiLine())           \
                              << "} difference " << second_name << " - " << first_name << ": {"         \
                              << print(difference(stream2_lines, stream1_lines), multiLine())

  std::copy(linesNoTrailingSpacesBegin(stream1), linesNoTrailingSpacesEnd(),
            std::inserter(stream1_lines, stream1_lines.end()));
  std::copy(linesNoTrailingSpacesBegin(stream2), linesNoTrailingSpacesEnd(),
            std::inserter(stream2_lines, stream2_lines.end()));

  if (stream1_lines == stream2_lines)
    return true;
  else {
    if (warn)
      SDL_WARN(Util, SDL_EQUAL_MSG());
    else
      SDL_INFO(Util, SDL_EQUAL_MSG());
    return false;
  }
}

bool StreamLinesEqual(std::istream& stream1, std::istream& stream2, char const* first_name,
                      char const* second_name) {
  std::vector<std::string> stream1_lines, stream2_lines;
  std::copy(linesNoTrailingSpacesBegin(stream1), linesNoTrailingSpacesEnd(), std::back_inserter(stream1_lines));
  std::copy(linesNoTrailingSpacesBegin(stream2), linesNoTrailingSpacesEnd(), std::back_inserter(stream2_lines));
  return stream1_lines == stream2_lines;
}

/**
   replace runs of digits, so long as they are not:

   immediately preceeded or followed by a decimal point '.',

   preceded by '-'

   followed by end of line or '['


   perl regex cribsheet:

   (?!abc) matches zero characters only if they are not followed by the expression "abc".

   (?<!pattern) consumes zero characters, only if pattern could not be matched
   against the characters preceding the current position (pattern must be of
   fixed length) (negative lookbehind: Patterns which start with negative
   lookbehind assertions may match at the beginning of the string being
   searched.)

*/
std::string ReplaceIntegersCopy(std::string const& str, std::string const& replaceIntegersBy) {
  static boost::regex integerRegexp("(?<![-0-9.])[[:digit:]]+(?:(?![.0-9[])|$)");
  using namespace boost::regex_constants;
  return boost::regex_replace(str, integerRegexp, replaceIntegersBy, format_literal | format_all);
}


}}
