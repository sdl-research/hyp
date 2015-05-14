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
#include <sdl/Types.hpp>
#include <sdl/Util/IsChar.hpp>

namespace sdl {
namespace Util {

/// trim from start
inline std::string& leftTrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), NotSpace()));
  return s;
}

inline Pchar leftTrimBegin(Slice s) {
  return std::find_if(s.first, s.second, NotSpace());
}

inline std::string::const_iterator leftTrimBegin(std::string const& s) {
  return std::find_if(s.begin(), s.end(), NotSpace());
}

/**

   \return maximum iter on [begin, end) with pred(*iter)==true, else return end. this is like find_if
   (c.rbegin(), c.rend(), pred)+1).base() except you get end rather than begin if not found.
*/

template <class Iter, class Pred>
inline Iter findLast(Iter begin, Iter const& end, Pred pred) {
  Iter i = end;
  while (i > begin) {
    --i;
    if (pred(*i)) return i;
  }
  return end;
}


template <class Iter, class Pred>
inline Iter findEnd(Iter begin, Iter const& end, Pred pred) {
  Iter i = end;
  while (i > begin) {
    --i;
    if (pred(*i)) return ++i;
  }
  return end;
}

inline std::string::const_iterator rightTrimEnd(std::string const& s) {
  return findEnd(s.begin(), s.end(), NotSpace());
}

/// trim from end
static inline std::string& rightTrim(std::string& s) {
  std::string::iterator end = s.end();
  std::string::iterator i = findLast(s.begin(), s.end(), NotSpace());
  if (i == end)
    s.clear();
  else
    s.erase(++i, end);
  return s;
}

/// trim from both ends
static inline std::string& trim(std::string& s) {
  return leftTrim(rightTrim(s));
}


}}

#endif
