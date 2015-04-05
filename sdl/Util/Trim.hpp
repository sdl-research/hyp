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

    remove line-initial and/or line-final ascii whitespace.

    see also: boost::trim

    boost::trim_if (str, boost::is_any_of(spaces))

    note: if you're just going to split removing empty fields anyway, why not
    Util/Split.hpp splitSpace (where all whitespace is a separator)
*/

#ifndef SDL_UTIL_TRIM_HPP
#define SDL_UTIL_TRIM_HPP
#pragma once

#include <string>
#include <algorithm>
#include <functional>
#include <cctype>

namespace sdl {
namespace Util {

/** predicate, true iff non-isspace char.

    needed instead of std::not1<IsSpace> because windows compile couldn't handle it */
struct NotSpace
{
  typedef bool result_type;
  bool operator()(char c) const
  {
#ifdef _WIN32
    return (c & 0x80) || !std::isspace(c);
#else
    return !std::isspace(c);
#endif
  }
};

/// trim from start
static inline std::string &leftTrim(std::string &s) {
  s.erase(s.begin(), std::find_if (s.begin(), s.end(), NotSpace()));
  return s;
}

/**

   \return maximum iter on [begin, end) with pred(*iter)==true, else return end. this is like find_if (c.rbegin(), c.rend(), pred)+1).base() except you get end rather than begin if not found.
*/

template <class Iter, class Pred>
inline Iter findLast(Iter begin, Iter const& end, Pred pred)
{
  Iter i = end;
  while (i>begin) {
    --i;
    if (pred(*i))
      return i;
  }
  return end;
}

/// trim from end
static inline std::string &rightTrim(std::string &s) {
  std::string::iterator end = s.end();
  std::string::iterator i = findLast(s.begin(), s.end(), NotSpace());
  if (i==end)
    s.clear();
  else
    s.erase(++i, end);
  return s;
}

/// trim from both ends
static inline std::string &trim(std::string &s) {
  return leftTrim(rightTrim(s));
}


}}

#endif
