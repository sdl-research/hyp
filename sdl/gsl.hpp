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

    GSL: stuff that will be in C++14 and later (string_span, span, owner,
    not_null, final_act. Expects(..) Ensures(..) pre/postcond assert
*/

#ifndef GSL_GRAEHL_2015_10_24_HPP
#define GSL_GRAEHL_2015_10_24_HPP
#pragma once

#include <graehl/shared/is_container.hpp>
#include <gsl.h>

namespace sdl {

// using namespace gsl;

/// span<T> is like (T*, size_t). .data() and .size()
using gsl::span;

/// string_span<> is span<char> (default template arg char)
using gsl::string_span;

/// cstring_span<> is span<char const>
using gsl::cstring_span;

/// get a cstring_span from a char const* (if you construct directly from compile time string constant then
/// you get the trailing '0' included in size).
using gsl::ensure_z;

/// std::string from string_span: to_string(cstring_span<>) (ADL)

/// owner<T>-move assign to transfer
using gsl::owner;

/// not_null<T>-like T* but can't be 0
using gsl::not_null;
}

namespace graehl {

template <class T>
struct is_nonstring_container<gsl::span<T>, void> : std::true_type {};


}

#endif
