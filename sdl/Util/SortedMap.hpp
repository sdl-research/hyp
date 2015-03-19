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
