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

    ascii space upper/lower etc. (see Utf8Case for unicode-aware).
*/

#ifndef SDL_ASCII_CASE_HPP
#define SDL_ASCII_CASE_HPP
#pragma once

#include <string>

namespace sdl {
namespace Util {

/**
   equivalent to std::isdigit except on msvc, which takes its own
   locale-dependent interpretation.
*/
inline bool isAsciiDigit(char c)
{
  return c>='0' && c<='9';
}

inline bool containsAsciiDigits(std::string const& str) {
  for (std::string::const_iterator i = str.begin(), e = str.end(); i!=e; ++i)
    if (isAsciiDigit(*i)) return true;
  return false;
}

inline void replaceAsciiDigits(std::string & str, char replaceBy='@') {
  for (std::string::iterator i = str.begin(), e = str.end(); i!=e; ++i)
    if (isAsciiDigit(*i)) *i = replaceBy;
}

inline std::string withReplacedAsciiDigits(std::string str, char replaceBy='@') {
  replaceAsciiDigits(str, replaceBy);
  return str;
}

/**
   equivalent to std::isalpha except on msvc, which takes its own
   locale-dependent interpretation.
*/
inline bool isAsciiAlpha(char c)
{
  return c>='a' && c<='z'
      || c>='A' && c<='Z';
}

inline char asciiUpcase(char c)
{
  if (c>='a'&&c<='z')
    c += ('A'-'a');
  return c;
}

inline char asciiDowncase(char c)
{
  if (c>='A'&&c<='Z')
    c -= ('A'-'a');
  return c;
}

struct AsciiUpcaseChar
{
  typedef char argument_type;
  typedef char result_type;
  char operator()(char c) const {
    return asciiUpcase(c);
  }
};

struct AsciiDowncaseChar
{
  typedef char argument_type;
  typedef char result_type;
  char operator()(char c) const {
    return asciiDowncase(c);
  }
};


inline std::string &inplaceAsciiAllUpcase(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(), AsciiUpcaseChar());
  return s;
}

inline std::string &inplaceAsciiAllDowncase(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(), AsciiDowncaseChar());
  return s;
}

inline std::string &inplaceAsciiFirstUpcase(std::string &s)
{
  if (!s.empty()) s[0] = asciiUpcase(s[0]);
  return s;
}

inline std::string &inplaceAsciiFirstDowncase(std::string &s)
{
  if (!s.empty()) s[0] = asciiDowncase(s[0]);
  return s;
}

inline std::string asciiAllUpcase(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(), AsciiUpcaseChar());
  return s;
}

inline std::string asciiAllDowncase(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(), AsciiDowncaseChar());
  return s;
}

inline std::string asciiFirstUpcase(std::string s)
{
  if (!s.empty()) s[0] = asciiUpcase(s[0]);
  return s;
}

inline std::string asciiFirstDowncase(std::string s)
{
  if (!s.empty()) s[0] = asciiDowncase(s[0]);
  return s;
}

}}

#endif
