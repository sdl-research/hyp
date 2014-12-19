/** \file

    case-insensitive versions of posix ::strncmp and std::strcmp.
*/

#ifndef STRCASECMP_JG20121026_HPP
#define STRCASECMP_JG20121026_HPP
#pragma once

#include <cctype>

namespace sdl { namespace Util {

/**
   return >0 if char1>char2, ==0 if char1==char2, else <0.
*/
inline int charcmp(unsigned char1, unsigned char2) {
  return (int)(char1-char2);
}

/**
   like C std::strcmp - but case insensitive (only compare up to n=max # of chars; but 0 still early-terminates strings). return >0 if s1>s2, ==0 if s1==s2, else <0. (lexicographic)
*/
inline int strncasecmp(char const* s1, char const* s2, std::size_t n) {
  using std::tolower;
  while (tolower(*s1) == tolower(*s2)) {
    if (!n--) return 0;
    if (!(*s1 && *s2)) break;
    ++s1; ++s2;
  }
  return charcmp(*(unsigned char*)s1, *(unsigned char*)s2);
  /* alternative:
     #include <string.h>

     #ifdef _MSC_VER
     return ::_strnicmp(s1, s2, n);
     #else
     return ::strncasecmp(s1, s2, n);
     #endif
  */
}

/**
   for null-terminated c-strings. return >0 if s1>s2, ==0 if s1==s2, else <0. (lexicographic)
*/
inline int strcasecmp(char const* s1, char const* s2) {
  using std::tolower;
  while (tolower(*s1) == tolower(*s2)) {
    if (!(*s1 && *s2)) break;
    ++s1; ++s2;
  }
  return charcmp(*(unsigned char*)s1, *(unsigned char*)s2);
}

// just like the c-string methods, all of these compare only the characters up to the first 0 char (so only part of a std::string may be used if it has embedded 0 chars)
inline int strncasecmp(char const* s1, std::string const& s2, std::size_t n) {
  return strncasecmp(s1, s2.c_str(), n);
}

inline int strncasecmp(std::string const& s1, char const* s2, std::size_t n) {
  return strncasecmp(s1.c_str(), s2, n);
}

inline int strncasecmp(std::string const& s1, std::string const& s2, std::size_t n) {
  return strncasecmp(s1.c_str(), s2.c_str(), n);
}

inline int strcasecmp(char const* s1, std::string const& s2) {
  return strcasecmp(s1, s2.c_str());
}

inline int strcasecmp(std::string const& s1, char const* s2) {
  return strcasecmp(s1.c_str(), s2);
}

inline int strcasecmp(std::string const& s1, std::string const& s2) {
  return strcasecmp(s1.c_str(), s2.c_str());
}


}}

#endif
