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

    probably more efficient than stringstream; also gives contiguous
    vector<char> result without necessarily building a std::string object first
    (so can use e.g. xmt/Types.hpp Slice)

    StringBuilder s("1");
    s.append(" plus ")('2').
    s.str(); // "1 plus 2"

*/

#ifndef STRINGBUILDER_JG2012711_HPP
#define STRINGBUILDER_JG2012711_HPP
#pragma once

#include <graehl/shared/string_to.hpp>
#include <graehl/shared/string_builder.hpp>

namespace sdl {
namespace Util {

using graehl::string_to;
using graehl::to_string_impl;

template <class V>
inline std::string toString(V const& val) {
  return graehl::to_string_impl(val);
}

typedef graehl::string_buffer StringBuffer;
/**
   vector of char with methods for converting to string and appending
   (conversion to string may be faster than ostream <<).
*/
typedef graehl::string_builder StringBuilder;

/**
   WordSpacer sp('.');
   StringBuilder().range(vector, sp);
*/
typedef graehl::word_spacer WordSpacer;


}}

#endif
