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

    equality/hash for never-null pointers (by their pointed-to contents).
*/

#ifndef NONNULLPOINTEE_JG_2013_03_06_HPP
#define NONNULLPOINTEE_JG_2013_03_06_HPP
#pragma once

#include <boost/functional/hash.hpp>

namespace sdl { namespace Util {

/**
   compare pointer-likes by their pointed-to contents.
*/
struct NonNullPointeeEqual {
  typedef bool result_type;

  template <class Ptr1, class Ptr2>
  bool operator()(Ptr1 p1, Ptr2 p2) const {
    return *p1 == *p2;
  }
};

/**
   if value == is expensive, check pointers first.
*/
struct NonNullPointeeEqualExpensive {
  typedef bool result_type;

  template <class Ptr1, class Ptr2>
  bool operator()(Ptr1 p1, Ptr2 p2) const {
    return p1 == p2 || *p1 == *p2;
  }
};

/**
   Hash pointer-likes by their pointed-to contents.
*/
template <class T>
struct NonNullPointeeHash : boost::hash<T> {
  typedef std::size_t result_type;

  result_type operator()(T *p) const {
    return boost::hash<T>()(*p);
  }
  result_type operator()(T const* p) const {
    return boost::hash<T>()(*p);
  }
  template <class Ptr>
  result_type operator()(Ptr const& p) const {
    return boost::hash<T>()(*p);
  }
};

struct NonNullPointeeHashValue {
  typedef std::size_t result_type;

  template <class T>
  result_type operator()(T *p) const {
    return hash_value(*p);
  }
  template <class T>
  result_type operator()(T const* p) const {
    return hash_value(*p);
  }
};


}}

#endif
