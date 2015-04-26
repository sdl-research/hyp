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

    split space-separated words from string into vector<string> or vector<SplitRange>.

    note that it's an error to split into an iterator range into a temporary
    copy via splitSpaces - you must use NoTrim or InPlace

    the most reliable/desirable split in most cases is splitSpaces(words,
    string), because any \r\n \n issues are moot, and because we report no empty
    tokens, there's no use in trimming left/right trailing whitespace first. in
    SDL we don't allow any ascii whitespace in any token/symbol anyway.

    the other variants are useful when you have a string format that
    distinguishes between e.g. '\t' and ' ' - as is often the case in hadoop streaming/pipes
*/

#ifndef SPLIT_JG2013118_HPP
#define SPLIT_JG2013118_HPP
#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>  // is_any_of, is_from_range
#include <sdl/Util/Chomp.hpp>
#include <sdl/Util/Trim.hpp>
#include <sdl/Util/IsChar.hpp>
#include <boost/static_assert.hpp>

namespace sdl {
namespace Util {

typedef std::vector<SplitRange> SplitRanges;

/**
   splitSpaceKeepEmpties: we consider leading/trailing space significant (they
   correspond to empty fields, as do line-internal double-spaces).

   we chomp(str) first in case you have '\r' DOS newline crud, so this shouldn't
   be used if str has significant trailing \r or \n

   splits on any char in spaces
*/
template <class Strings>
void splitSpacesKeepEmpties(Strings& words, std::string const& line, char const* spaces) {
  SplitRange str((chomped(line)));
  boost::split(words, str, boost::is_any_of(spaces), boost::token_compress_off);
}

/// split on ascii whitespace
template <class Strings>
void splitSpacesKeepEmpties(Strings& words, std::string const& line) {
  SplitRange str((chomped(line)));
  boost::split(words, str, IsSpace(), boost::token_compress_off);
}

/// split on space char only
template <class Strings>
void splitSpaceKeepEmpties(Strings& words, std::string const& line, char space) {
  SplitRange str((chomped(line)));
  boost::split(words, str, IsChar(space), boost::token_compress_off);
}

/// split on ' ' char only
template <class Strings>
void splitSpaceKeepEmpties(Strings& words, std::string const& line) {
  SplitRange str((chomped(line)));
  boost::split(words, str, IsSpaceChar(), boost::token_compress_off);
}

template <class Strings, class String>
void splitChars(Strings& words, String const& str) {
  for(typename String::const_iterator i = str.begin(), e = str.end(); i!=e; ++i)
    words.push_back(typename Strings::value_type(1, *i));
}

/**
   split on ascii whitespace.
   we can't copy then return iterator_range pointing into copy, so no trim (or InPlace)
*/
template <class Strings, class String>
void splitSpaces(Strings& words, String const& str) {
  boost::split(words, str, IsSpace(), boost::token_compress_on);
}

/// split on anything in spaces
template <class Strings, class String>
void splitSpaces(Strings& words, String const& str, char const* spaces) {
  boost::split(words, str, boost::is_any_of(spaces), boost::token_compress_on);
}

/// split line (removing endl) on ' '. no empty tokens except possibly first and last
template <class Strings, class String>
void splitSpace(Strings& words, String const& c) {
  boost::split(words, c, IsSpaceChar(), boost::token_compress_on);
}

/// split line (removing endl) on 'space'. no empty tokens except possibly first and last
template <class Strings, class String>
void splitSpace(Strings& words, String const& c, char space) {
  boost::split(words, c, IsChar(space), boost::token_compress_on);
}

/// split line (removing endl) on space(c). no empty tokens
template <class Strings, class String, class Space>
void splitSpaceTrimmedOf(Strings& words, String const& line, Space const& space) {
  typedef boost::iterator_range<typename String::const_iterator> SplitRange;
  SplitRange str((trimmedOf(line, space)));
  boost::split(words, str, space, boost::token_compress_on);
}

/// split line (removing endl) on whitespace. no empty tokens
template <class Strings, class String>
void splitSpaceTrimmed(Strings& words, String const& line) {
  return splitSpaceTrimmedOf(words, line, IsSpaceChar());
}

/// split line (removing endl) on char. no empty tokens
template <class Strings, class String>
void splitSpaceTrimmed(Strings& words, String const& line, char space) {
  return splitSpaceTrimmedOf(words, line, IsChar(space));
}

/// return number of tokens that would result from boost::split w/ token_compress_on
template <class StringIter, class SpacePredicate>
inline std::size_t numSplit(StringIter i, StringIter e, SpacePredicate space) {
  std::size_t n = 0;
  bool counts = true;
  for (; i != e; ++i) {
    if (space(*i))
      counts = true;
    else {
      if (counts) ++n;
      counts = false;
    }
  }
  return n;
}

template <class String, class SpacePredicate>
inline std::size_t numSplit(String const& str, SpacePredicate space) {
  return numSplit(str.begin(), str.end(), space);
}

template <class String>
inline std::size_t numSplit(String const& str) {
  return numSplit(str.begin(), str.end(), IsSpace());
}

/// return whether at least 1 token would result from split on space
template <class StringIter, class SpacePredicate>
inline bool nonemptySplit(StringIter i, StringIter e, SpacePredicate space) {
  for (; i != e; ++i)
    if (!space(*i)) return true;
  return false;
}

template <class String, class SpacePredicate>
inline bool nonemptySplit(String const& str, SpacePredicate space) {
  return nonemptySplit(str.begin(), str.end(), space);
}

template <class String>
inline bool nonemptySplit(String const& str) {
  return nonemptySplit(str.begin(), str.end(), IsSpace());
}

template <class Strings>
inline std::size_t countEmpties(Strings const& strs) {
  std::size_t nempty = 0;
  for (typename Strings::const_iterator i = strs.begin(), e = strs.end(); i != e; ++i) {
    if (i->empty()) ++nempty;
  }
  return nempty;
}

struct IsEmpty {
  template <class String>
  bool operator()(String const& x) const {
    return x.empty();
  }
};

template <class Strings>
inline void removeEmpties(Strings& strs) {
  strs.erase(std::remove_if(strs.begin(), strs.end(), IsEmpty()), strs.end());
}


}}

#endif
