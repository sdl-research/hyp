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

    uniform API for map or set containing key (see also Add.hpp, Map.hpp, dynamic_bitset.hpp)
*/

#ifndef SDL_CONTAINS_JEG20111026_HPP
#define SDL_CONTAINS_JEG20111026_HPP
#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <sdl/Util/SmallVector.hpp>
#include <sdl/Util/SortedMap.hpp>

namespace sdl {
namespace Util {

template <class C, class T>
inline bool contains(std::basic_string<C, T> const& s, C c) {
  return s.find(c) != std::basic_string<C, T>::npos;
}

template <class V>
inline bool contains(std::vector<V> const& s, V const& k) {
  return std::find(s.begin(), s.end(), k) != s.end();
}
template <class V>
inline bool contains(std::list<V> const& s, V const& k) {
  return std::find(s.begin(), s.end(), k) != s.end();
}
template <class V>
inline bool contains(std::deque<V> const& s, V const& k) {
  return std::find(s.begin(), s.end(), k) != s.end();
}

template <class Set>
inline typename boost::enable_if<HasKeyType<Set>, bool>::type  // bool
    contains(Set const& s, typename Set::key_type const& k) {
  return s.find(k) != s.end();
}

template <class V, unsigned MaxInline, class Size>
inline bool contains(small_vector<V, MaxInline, Size> const& c, V const& v) {
  return c.contains(v);
}

template <class ContainerMap, class Map>
bool containsMap(ContainerMap const& container, Map const& contained) {
  for (typename Map::const_iterator i = contained.begin(), e = contained.end(); i != e; ++i) {
    if (!Util::contains(container, i->first)) return false;
  }
  return true;
}

template <class ContainerMap, class Map>
bool containsSameValues(ContainerMap const& container, Map const& contained) {
  for (typename Map::const_iterator i = contained.begin(), e = contained.end(); i != e; ++i) {
    typename ContainerMap::const_iterator i2 = container.find(i->first);
    if (i2 == container.end()) return false;
    if (i2->second != i->second) return false;
  }
  return true;
}

// NOTE: for sorted same-order maps you can do better (just use default ==). but this works for unordered vs
// ordered, etc.
template <class Map1, class Map2>
bool equalMap(Map1 const& map1, Map2 const& map2) {
  return containsMap(map2, map1) && containsSameValues(map1, map2);
}

template <class K, class V, class Less>
bool equalMap(std::map<K, V, Less> const& map1, std::map<K, V, Less> const& map2) {
  return map1 == map2;
}

template <class Substr, class Container>
bool containsStarting(Substr const& substr, Container const& container,
                      typename Container::size_type containerStartingIndex) {
  typedef typename Container::size_type Size;
  if (container.size() < containerStartingIndex + (Size)substr.size()) return false;
  typename Container::const_iterator i = container.begin() + containerStartingIndex;
  for (typename Substr::const_iterator subi = substr.begin(), subend = substr.end(); subi != subend;
       ++subi, ++i)
    if (*subi != *i) return false;
  return true;
}


}}

#endif
