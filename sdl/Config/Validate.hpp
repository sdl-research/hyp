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
#ifndef VALIDATE_JG2012725_HPP
#define VALIDATE_JG2012725_HPP
#pragma once

/** \file

    Usage: config("option", &val).validate(Config::boundedRange(0,10)) means only 0...9 are allowed.
*/

#include <sdl/graehl/shared/validate.hpp>

namespace sdl {
namespace Config {

template <class Val>
void call_validate(Val& val) {
  ::adl::adl_validate(val);
}

// for strings or paths: .validate(Config::Exists())
typedef configure::exists Exists;
typedef configure::file_exists FileExists;
typedef configure::dir_exists DirExists;

using configure::one_of;

// for numerics:
template <class I>
configure::bounded_range_validate<I> boundedRange(I const& begin, I const& end,
                                                  std::string const& desc = "value out of bounds") {
  return configure::bounded_range_validate<I>(begin, end, desc);
}

template <class I>
configure::bounded_range_inclusive_validate<I> boundedRangeInclusive(I const& begin, I const& end,
                                                                     std::string const& desc
                                                                     = "value out of bounds") {
  return configure::bounded_range_inclusive_validate<I>(begin, end, desc);
}


}}

#endif
