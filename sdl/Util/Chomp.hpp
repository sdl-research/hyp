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

    erase a single final newline (dos: "\r\n" or unix: "\n") from a string, if any.

    see LineOptions.hpp for a configurable class controlling chomp/trim/nfc
*/

#ifndef SDL_CHOMP_JG2012125_HPP
#define SDL_CHOMP_JG2012125_HPP
#pragma once

#include <sdl/Util/LogHelper.hpp>
#include <sdl/StringConsumer.hpp>
#include <boost/range/iterator_range.hpp>
#include <algorithm>
#include <iostream>
#include <string>

namespace sdl {
namespace Util {

typedef boost::iterator_range<std::string::const_iterator> SplitRange;

inline void eraseLastChar(std::string& str) {
  str.pop_back();
}

/**
   if str was from std::getline then it already has the trailing '\n' removed
   but in case of Windows + binary mode input we want to remove the '\r' in that
   case anyway.

   if str is the whole line incl '\n' then we remove the trailing "\n" or "\r\n"
*/
inline std::string& chomp(std::string& str) {
  if (!str.empty()) {
    std::string::iterator i = str.end();
    char last = *--i;
    if (last == '\r')
      eraseLastChar(str);
    else if (last == '\n') {
      if (i > str.begin() && i[-1] == '\r') eraseLastChar(str);
      eraseLastChar(str);
    }
  }
  return str;
}

/**
   when str was already std::getline it has no trailing '\n'
*/
inline std::string& chompGetline(std::string& str) {
  std::string::size_type sz = str.size();
  if (sz && str[sz - 1] == '\r') eraseLastChar(str);
  return str;
}

template <class CharIter>
inline CharIter chompEnd(CharIter begin, CharIter i) {
  if (i == begin) return i;
  if (*--i == '\n') {
    // TODO: test
    if (i != begin && i[-1] == '\r') --i;
    return i;
  } else
    return ++i;
}

template <class String>
inline typename String::const_iterator chompEnd(String const& str) {
  return chompEnd(str.begin(), str.end());
}

template <class CharIter>
inline unsigned chompLen(CharIter begin, unsigned len) {
  if (!len) return len;
  if (begin[--len] == '\n') {
    if (len && begin[len - 1] == '\r') --len;
    return len;
  } else
    return len + 1;
}

template <class CharIter, class Space>
inline CharIter trimEndOf(CharIter begin, CharIter i, Space const& space) {
  if (i == begin)
    return i;
  else {
    while (space(*--i))
      if (i == begin) return i;
    return ++i;
  }
}

template <class CharIter>
inline CharIter trimEndOfChar(CharIter begin, CharIter i, char space) {
  if (i == begin)
    return i;
  else {
    while (*--i == space)
      if (i == begin) return i;
    return ++i;
  }
}

template <class CharIter, class Space>
inline CharIter trimBeginOf(CharIter i, CharIter end, Space const& space) {
  if (i != end)
    while (space(*i))
      if (++i == end) return i;
  return i;
}

template <class CharIter>
inline CharIter trimBeginOfChar(CharIter i, CharIter end, char space) {
  if (i != end)
    while (*i == space)
      if (++i == end) return i;
  return i;
}

template <class CharIter, class Space>
inline boost::iterator_range<CharIter> trimOf(CharIter begin, CharIter end, Space const& space) {
  end = trimEndOf(begin, end, space);
  return boost::iterator_range<CharIter>(trimBeginOf(begin, end, space), end);
}


template <class CharIter>
inline boost::iterator_range<CharIter> trimOfChar(CharIter begin, CharIter end, char space) {
  end = trimEndOfChar(begin, end, space);
  return boost::iterator_range<CharIter>(trimBeginOfChar(begin, end, space), end);
}


/// like chomp but doesn't modify str. range is valid only as long as str
template <class String>
inline boost::iterator_range<typename String::const_iterator> chomped(String const& str) {
  return boost::iterator_range<typename String::const_iterator>(str.begin(), chompEnd(str));
}


template <class String, class Space>
inline boost::iterator_range<typename String::const_iterator> endTrimmedOf(String const& str, Space const& space) {
  typename String::const_iterator begin = str.begin();
  return trimEndOf(begin, chompEnd(begin, str.end()), space);
}

template <class String>
inline boost::iterator_range<typename String::const_iterator> endTrimmed(String const& str, char space = ' ') {
  typename String::const_iterator begin = str.begin();
  return trimEndOfChar(begin, chompEnd(begin, str.end()), space);
}

template <class String, class Space>
inline boost::iterator_range<typename String::const_iterator> trimmedOf(String const& str, Space const& space) {
  typename String::const_iterator begin = str.begin();
  return trimOf(begin, chompEnd(begin, str.end()), space);
}

template <class String>
inline boost::iterator_range<typename String::const_iterator> trimmed(String const& str, char space = ' ') {
  typename String::const_iterator begin = str.begin();
  return trimOfChar(begin, chompEnd(begin, str.end()), space);
}


}}

#endif
