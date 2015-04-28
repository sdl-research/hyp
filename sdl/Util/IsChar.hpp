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

    ascii whitespace predicates.
*/

#ifndef ISCHAR_JG_2014_01_09_HPP
#define ISCHAR_JG_2014_01_09_HPP
#pragma once

#include <cctype>

namespace sdl { namespace Util {

struct IsChar
{
  template <class Config>
  void configure(Config &config) {
    config.is("IsChar");
    config("char", &x)
        ("character to split on e.g. ' '");
  }
  char x;
  IsChar(char x = '\0') : x(x) {}
  typedef bool result_type;
  bool operator()(char c) const
  {
    return c == x;
  }
};

struct IsSpaceChar
{
  typedef bool result_type;
  bool operator()(char c) const
  {
    return c == ' ';
  }
};

struct IsSpace
{
  typedef bool result_type;
  bool operator()(char c) const
  {
    // Cast required so we don't pass negative values for c >= 128
    // (when the caller actually passes "unsigned char")
    return std::isspace((unsigned char) c);
  }
};


}}

#endif
