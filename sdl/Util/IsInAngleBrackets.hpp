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
