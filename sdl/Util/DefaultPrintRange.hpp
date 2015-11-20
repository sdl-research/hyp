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

    PrintAsRange: print string or container.

    TODO: remove?
*/

#ifndef DEFAULTPRINTRANGE_JG2012117_HPP
#define DEFAULTPRINTRANGE_JG2012117_HPP
#pragma once

#include <string>
#include <sdl/Printer.hpp>
#include <sdl/Util/PrintRange.hpp>

namespace sdl {
namespace Util {

template <class Range, class Enable = void>
struct PrintAsRange {
#ifdef _MSC_VER
#pragma warning(disable : 4180)
#endif
  template <class O>
  static void print(O& o, Range const& r) {
    o << r;
  }
};

template <>
struct PrintAsRange<std::string, void> {
  template <class O>
  static void print(O& o, std::string const& r) {
    o << r;
  }
};

template <class Range>
struct PrintAsRange<Range, typename VoidIf<typename Range::const_iterator>::type> {
  typedef void type;
  template <class O>
  static void print(O& o, Range const& r) {
    printRange(o, r);
  }
};


}}

#endif
