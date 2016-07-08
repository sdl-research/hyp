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

    std::hash for member std::size_t hash_value() const isn't possible - so
    we'll use boost::hash instead.
*/

#ifndef HASHVALUE_GRAEHL_2015_10_16_HPP
#define HASHVALUE_GRAEHL_2015_10_16_HPP
#pragma once

#include <sdl/Util/TypeTraits.hpp>
#include <boost/functional/hash/hash.hpp>
#include <functional>

namespace sdl {
namespace Util {


// non-commutative
inline void hashvalCombine(std::size_t& s, std::size_t v) {
  s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
}

template <class B>
inline std::size_t hashPairInt(std::size_t s, B b) {
  hashvalCombine(s, b);
  return s;
}

template <class I>
inline std::size_t hashRange(I i, I end, std::size_t s = 0) {
  boost::hash<typename std::iterator_traits<I>::value_type> h;
  for (; i != end; ++i) hashvalCombine(s, h(*i));
  return s;
}

template <class C>
inline std::size_t hashCont(C const& c) {
  return hashRange(c.begin(), c.end(), c.size());
}

struct HashCont {
  template <class C>
  std::size_t operator()(C const& c) const {
    return hashCont(c);
  }
};

struct PointedHashCont {
  template <class C>
  std::size_t operator()(C const* c) const {
    return hashCont(*c);
  }
};

struct PointedEqual {
  template <class A, class B>
  bool operator()(A const* a, B const* b) const {
    return *a == *b;
  }
  typedef bool return_type;
};

struct PointedLess {
  template <class A, class B>
  bool operator()(A const* a, B const* b) const {
    return *a < *b;
  }
  typedef bool return_type;
};


}}

#endif
