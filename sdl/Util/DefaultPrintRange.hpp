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

    provide (via PrintRange.hpp) operator<<for std:: collections, maps, etc. -
    anything with a T::const_iterator and a boost::begin(T const&)

    also (more explicitly):
    struct mine : std::vector<int, flaot> {
      friend DEFINE_PRINTRANGE(mine)
    };
*/

#ifndef DEFAULTPRINTRANGE_JG2012117_HPP
#define DEFAULTPRINTRANGE_JG2012117_HPP
#pragma once

#include <sdl/Util/Print.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/VoidIf.hpp>
#include <string>

namespace sdl {
namespace Util {

template <class Range, class Enable = void>
struct PrintAsRange {
#ifdef _MSC_VER
# pragma warning(disable:4180)
#endif
  template <class O>
  static void print(O &o, Range const& r) { o << r; }
};

template <>
struct PrintAsRange<std::string, void> {
  template <class O>
  static void print(O &o, std::string const& r) { o << r; }
};

template <class Range>
struct PrintAsRange<Range, typename VoidIf<typename Range::const_iterator>::type> {
  typedef void type;
  template <class O>
  static void print(O &o, Range const& r) { printRange(o, r); }
};


}}

/**
   provide operator<<for std:: collections, maps, etc.
*/
template <class O, class Range>
typename sdl::Util::TypeIf<O&, typename sdl::Util::PrintAsRange<Range>::type>::type //O&, enabled if this is a const_iterator-having Range (but not string)
operator<<(O &o, Range const& r)
{
  sdl::Util::printRange(o, r);
  return o;
}

// use this to make default prints for specific container types e.g. for unit tests
#define DEFINE_PRINTRANGE(C) inline std::ostream & operator<<(std::ostream &o, C const& c) { sdl::Util::printRange(o, c); return o; }
#define DEFINE_PRINTPAIR_STR(C, sepStr) inline std::ostream & operator<<(std::ostream &o, C const& c) { sdl::Util::writePair(o, c, sepStr); return o; }

#define DEFINE_PRINTPAIR(C) DEFINE_PRINTPAIR_STR(C, "=")

#define SDL_PLACE_IN_STD(C) namespace std { C }

#define DEFINE_PRINTRANGE_STD(C) SDL_PLACE_IN_STD(DEFINE_PRINTRANGE(C))
#define DEFINE_PRINTPAIR_STD(C) SDL_PLACE_IN_STD(DEFINE_PRINTPAIR(C))
#define DEFINE_PRINTPAIR_SEP_STD(C, sepStr) SDL_PLACE_IN_STD(DEFINE_PRINTPAIR_STR(C, sepStr))
#define DEFINE_PRINTRANGE_PAIR_STD(C) SDL_PLACE_IN_STD(DEFINE_PRINTPAIR(C::value_type) DEFINE_PRINTRANGE(C))

#endif
