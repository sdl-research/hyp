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
