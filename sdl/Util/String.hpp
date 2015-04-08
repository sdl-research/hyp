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

    some frequently used string algs.
*/

#ifndef STRING_JG20121212_HPP
#define STRING_JG20121212_HPP
#pragma once

#include <fstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Util {

/**
   \return length of prefix ending with second space or first space -> end of string. 0 if no space found.
*/
inline std::string::size_type len_split2(std::string const& str, char space = ' ') {
  std::string::size_type sp = str.find(space);
  if (sp == std::string::npos) return 0;
  sp = str.find(space, ++sp);
  return sp == std::string::npos ? str.size() : sp;
}

/**
   \return true if str contains space, and if so \out first part before, rest part after.
*/
inline bool split2(std::string const& str, std::string& first, std::string& rest, char space = ' ') {
  using std::string;
  string::size_type sp = str.find(space);
  if (sp == string::npos)
    return false;
  else {
    string::const_iterator begin = str.begin();
    string::const_iterator mid = begin + sp;
    first.assign(begin, mid);
    rest.assign(mid + 1, str.end());
    return true;
  }
}

/**
   \return true and strip (whitespace* prefix) if str starts with that, else return false
*/
inline bool stripPrefixTrim(std::string& str, std::string const& prefix,
                            std::string const& whitespace = "\n\r\t ") {
  std::string::size_type b;
  b = str.find_first_not_of(whitespace);
  std::string::iterator nonspaceBegin = str.begin() + b;
  using namespace boost;
  if (algorithm::starts_with(make_iterator_range(nonspaceBegin, str.end()), prefix)) {
    str.erase(str.begin(), nonspaceBegin + prefix.size());
    return true;
  } else
    return false;
}

/**
   \return true and strip prefix if str starts with that, else return false
*/
inline bool stripPrefix(std::string& str, std::string const& prefix) {
  if (boost::algorithm::starts_with(str, prefix)) {
    str.erase(str.begin(), str.begin() + prefix.size());
    return true;
  } else
    return false;
}

/**
   \return str.substr(str.size()-suffix.size()) == suffix

   (without copying)
*/
template <class String>
inline bool endsWith(String const& str, String const& suffix) {
  return boost::algorithm::ends_with(str, suffix);
}

template <class String>
inline bool startsWith(String const& str, String const& prefix) {
  return boost::algorithm::starts_with(str, prefix);
}

inline bool endsWith(std::string const& str, std::string const& suffix) {
  return boost::algorithm::ends_with(str, suffix);
}

inline bool startsWith(std::string const& str, std::string const& prefix) {
  return boost::algorithm::starts_with(str, prefix);
}

inline bool endsWith(std::string const& str, char const* suffix) {
  return boost::algorithm::ends_with(str, suffix);
}

inline bool startsWith(std::string const& str, char const* prefix) {
  return boost::algorithm::starts_with(str, prefix);
}

/**
   \return true and strip suffix if str ends with that, else return false
*/
inline bool stripSuffix(std::string& str, std::string const& suffix) {
  if (endsWith(str, suffix)) {
    str.erase(str.end() - suffix.size(), str.end());
    return true;
  } else
    return false;
}


}}

#endif
