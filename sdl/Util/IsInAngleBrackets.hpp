#ifndef SDL_UTIL_ISINANGLEBRACKETS_HPP
#define SDL_UTIL_ISINANGLEBRACKETS_HPP
#pragma once

#include <string>

namespace sdl {
namespace Util {

inline
bool isInAngleBrackets(std::string const& str) {
  std::string::size_type len = str.length();
  return len > 2
      && str[0] == '<'
      && str[len - 1] == '>';
}

}}

#endif
