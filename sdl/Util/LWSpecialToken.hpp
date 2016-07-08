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

    special substrings of tokens in the form __LW_.+__,
    for instance __LW_AT__(tokenization) or __LW_NE__ (number translation).

*/

#ifndef ISLW_SPECIAL_JG2012928_HPP
#define ISLW_SPECIAL_JG2012928_HPP
#pragma once

#include <string>

namespace sdl {
namespace Util {


namespace {
std::string const startLWSpecial = "__LW_";
std::string const endLWSpecial = "__";
const unsigned nStartLWSpecial = 5, nEndLWSpecial = 2;
}

/**
   \return true iff symbol starts with __LW and ends with __
*/
inline bool startsLW(std::string const& utf8str) {
  // although size is not the number of Unicodes, it lets us check that we end with __, due to the properties
  // of ascii in utf8
  return (utf8str.size() >= 8 && utf8str[0] == '_' && utf8str[1] == '_' && utf8str[2] == 'L'
          && utf8str[3] == 'W' && utf8str[4] == '_' && utf8str.find(endLWSpecial, 6) != std::string::npos);
}

/** starting at nextStarts, return npos if no more special __LW_.__ substrings are found, else return position
 * of next char after end of next substring (after the __), with nextStarts updated to the position of the
 * start of that __LW.__. */
inline std::string::size_type nextLWends(std::string const& utf8str, std::string::size_type& nextStarts) {
  using std::string;
  nextStarts = utf8str.find(startLWSpecial, nextStarts);
  if (nextStarts == string::npos) return nextStarts;
  string::size_type nextEnd
      = utf8str.find(endLWSpecial, nextStarts + nStartLWSpecial
                                       + 1);  // +1 because there must be at least one char before the __
  if (nextEnd == string::npos) return nextEnd;
  return nextEnd + nEndLWSpecial;
}

inline bool containsLW(std::string const& utf8str) {
  using std::string;
  string::size_type nextStarts = 0;
  return nextLWends(utf8str, nextStarts) != string::npos;
}


struct AppendNonLWVisitor {
  std::string append;
  typedef std::string::size_type Index;
  void LW(std::string const& str, Index begin, Index end) {}

  void text(std::string const& str, Index begin, Index end) { append.append(str, begin, end - begin); }
};

/**
   left->right in str:

   visit.LW(str, startIndex, endIndex, true) for __LW_.__ regions
   visit.text(str, startIndex, endIndex, false) for regions that aren't __LW_.__
*/
template <class Visitor>
void segmentLW(Visitor& visitor, std::string const& str) {
  std::string::size_type specialStarts = 0, specialEnds, len = str.size(), lastEnds = 0;
  while ((specialEnds = nextLWends(str, specialStarts)) != std::string::npos) {
    if (specialStarts > lastEnds) visitor.text(str, lastEnds, specialStarts);
    visitor.LW(str, specialStarts, specialEnds);
    lastEnds = specialEnds;
    specialStarts = lastEnds;
  }
  if (lastEnds == 0) visitor.text(str, 0, len);
  if (lastEnds != len) visitor.text(str, lastEnds, len);
}

/**
   \return str less __LW_.__ special regions
*/
inline std::string stripLW(std::string const& str) {
  AppendNonLWVisitor result;
  segmentLW(result, str);
  return result.append;
}


}}

#endif
