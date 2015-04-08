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

    traits for sorted vs hashed (unordered) maps.

    usage:

    template <class Map>
    bool fnForSortedMapOnly(Map &map, typename SortedMap<Map>::type *enable=0);

    template <class Map>
    bool fnForHashedMapOnly(Map &map, typename HashedMap<Map>::type *enable=0);
*/

#ifndef SORTEDMAP_JG2012117_HPP
#define SORTEDMAP_JG2012117_HPP
#pragma once

#include <sdl/Util/VoidIf.hpp>

namespace sdl {
namespace Util {


template <class Map, class Enable = void>
struct HasKeyType {
  enum { value = 0 };
};

template <class Map>
struct HasKeyType<Map, typename VoidIf<typename Map::key_type>::type> {
  enum { value = 1 };
  typedef void type;
};

template <class Map, class Enable = void>
struct NotHasKeyType {
  enum { value = 1 };
  typedef void type;
};

template <class Map>
struct NotHasKeyType<Map, typename VoidIf<typename Map::key_type>::type> {
  enum { value = 0 };
};

template <class Map, class Enable = void>
struct SortedMap {
  enum { value = 1 };
  typedef void type;
};

template <class HashMap>
struct SortedMap<HashMap, typename VoidIf<typename HashMap::hasher>::type> {
  enum { value = 0 };
};

template <class Map, class Enable = void>
struct HashedMap {
  enum { value = 0 };
};

template <class HashMap>
struct HashedMap<HashMap, typename VoidIf<typename HashMap::hasher>::type> {
  enum { value = 1 };
  typedef void type;
};


}}

#endif
