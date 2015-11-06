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
#include <sdl/Util/Split.hpp>
#include <graehl/shared/split.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Util/Sorted.hpp>

namespace sdl {
namespace Util {

typedef std::set<std::string> StringsSet;

StringsSet difference(StringsSet const& a, StringsSet const& b) {
  StringsSet a_minus_b;
  std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(a_minus_b, a_minus_b.end()));
  return a_minus_b;
}


void normalizeLine(std::string& line, bool sortWords, char wordsep) {
  chomp(line);
  if (sortWords) {
    graehl::join_to(line, Util::sorted(graehl::split(line, wordsep)), wordsep);
    SDL_DEBUG(Equal.getlineWords, "after sortWords: '" << line << "'");
  }
}

static void normalizeLines(Strings &in, StringsSet& out, bool sortWords, bool ignoreBlankLines,
                           char wordsep = ' ') {
  Strings::iterator begin = in.begin(), end = in.end(), i = begin, o = begin;
  for (; i!= end; ++i) {
    std::string &line = *i;
    normalizeLine(line, sortWords, wordsep);
    bool empty = ignoreBlankLines && line.empty();
    if (!empty) {
      out.insert(line);
      if (i != o) *o = std::move(*i);
      ++o;
    }
  }
  in.erase(o, end);
}

bool StringsUnorderedEqual(Strings lines1, Strings lines2, bool sortWords, char const* name1,
                           char const* name2, bool warn, bool ignoreBlankLines, bool sortLines) {
  StringsSet lines1Set, lines2Set;
  normalizeLines(lines1, lines1Set, sortWords, ignoreBlankLines);
  normalizeLines(lines2, lines2Set, sortWords, ignoreBlankLines);
#define SDL_ORIGINAL_ORDER_MSG() "original " \
      << name1 << ": {[\n" << printer(lines1, multiLineNoBrace()) << "\n]} original " << name2       \
      << ": {[\n" << printer(lines2, multiLineNoBrace()) << "]}"
#define SDL_EQUAL_MSG()                                                                                   \
  "NOT (unordered) EQUAL:\n "                                                                             \
      << name1 << ": {[(\n" << printer(lines1Set, multiLineNoBrace()) << "\n)]} " << name2 << ": {(" \
      << printer(lines2Set, multiLine()) << ")}\n\n difference " << name1 << " - " << name2 << ": {" \
      << printer(difference(lines1Set, lines2Set), multiLine()) << "} difference " << name2 << " - " \
      << name1 << ": {" << printer(difference(lines2Set, lines1Set), multiLine()) << "\n}" \
      << SDL_ORIGINAL_ORDER_MSG()
  if (lines1Set == lines2Set) {
    if (!sortLines && lines1 != lines2) {
      SDL_WARN(Util, "lines appear in different order:\n" << SDL_ORIGINAL_ORDER_MSG());
      return false;
    } else
      return true;
  } else {
    if (warn)
      SDL_WARN(Util, SDL_EQUAL_MSG());
    else
      SDL_INFO(Util, SDL_EQUAL_MSG());
    return false;
  }
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
std::string ReplacedIntegers(std::string const& str, std::string const& replaceIntegersBy) {
  static boost::regex integerRegexp("(?<![-0-9.])[[:digit:]]+(?:(?![.0-9[])|$)");
  using namespace boost::regex_constants;
  return boost::regex_replace(str, integerRegexp, replaceIntegersBy, format_literal | format_all);
}


}}
