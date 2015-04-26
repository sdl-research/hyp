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
/**
   \file

   Formerly provided for using different std/tr1/boost unordered_ / hash_
   map/set; now fixes boost::unordered_ for convenience.

   overrides some Map.hpp methods that need special Unordered treatment

   TODO: just pick a good unordered_map and _set impl and include the source
   here. too much memory usage / speed variation across platforms.
*/

#ifndef SDL_UNORDERED_HPP
#define SDL_UNORDERED_HPP
#pragma once


#include <boost/config.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
// TODO: find something higher performance - some tests have shown older boost
// versions having awful perf for memory with either int or string keys (Google
// dense_hash_map?). also, if using new enough compiler, expect high performance
// std::unordered. or maybe boost is better now.
#include <boost/unordered_set.hpp>

#include <iterator>
#include <cstdlib>
#include <cstddef>
#include <sdl/Util/Compare.hpp>
#include <sdl/Util/Contains.hpp>
#include <sdl/Util/Map.hpp>
#include <cmath>  //ceil

#define DEFAULTHASH_CONCRETE(type, arg, expr)               \
  namespace boost {                                         \
  template <>                                               \
  struct hash<type> {                                       \
    std::size_t operator()(type const& arg) const { expr; } \
  };                                                        \
  }


namespace sdl {

using boost::unordered_map;
using boost::unordered_set;
using boost::unordered_multimap;

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

/**
   reserve enough so that adding to a total size of n won't invalidate iterators due to rehashing or growing.
*/
template <class MapNotUnordered>
void reserveUnordered(MapNotUnordered& map, std::size_t n) {
  map.reserve(n);
}

/**
   reserve enough so that adding to a total size of n won't invalidate iterators due to rehashing/growing.
   TODO: recognize C++11 and use .reserve(n) also
*/

template <class Unordered>
void reserveUnorderedImpl(Unordered& unordered, std::size_t n) {
  unordered.rehash((std::size_t)std::ceil(((double)n / unordered.max_load_factor())));
}

template <class K, class V, class H, class E, class A>
void reserveUnordered(unordered_multimap<K, V, H, E, A>& umultimap, std::size_t n) {
  reserveUnorderedImpl(umultimap, n);
}

template <class K, class V, class H, class E, class A>
void reserveUnordered(unordered_map<K, V, H, E, A>& umap, std::size_t n) {
  reserveUnorderedImpl(umap, n);
}

template <class K, class H, class E, class A>
void reserveUnordered(unordered_set<K, H, E, A>& uset, std::size_t n) {
  reserveUnorderedImpl(uset, n);
}

// the map-postincrement erase_if in Map.hpp doesn't work for unordered_map

/**
   eraseIfVal: erase all umap[k] such that EraseIfFn(umap[k]). might be slower than a filtered copy/swap.
*/
template <class EraseIfFn, class K, class V, class H, class E, class A>
void eraseIfVal(unordered_map<K, V, H, E, A>& umap, EraseIfFn const& f) {
  for (typename unordered_map<K, V, H, E, A>::iterator i = umap.begin(), e = umap.end();
       i != e;)  // no ++i - intentional
    if (f(i->second))
      i = umap.erase(i);
    else
      ++i;
}

/**
   erase all umap[k] such that EraseIfFn(umap[k]). might be slower than a filtered copy/swap.
*/
template <class EraseIfFn, class K, class V, class H, class E, class A>
void eraseIfVal(unordered_multimap<K, V, H, E, A>& umap, EraseIfFn const& f) {
  for (typename unordered_multimap<K, V, H, E, A>::iterator i = umap.begin(), e = umap.end();
       i != e;)  // no ++i - intentional
    if (f(i->second))
      i = umap.erase(i);
    else
      ++i;
}

/**
   eraseIf: erase if EraseIfFn(Map/Set::value_type const&) (doesn't just get the key for maps)
*/

template <class EraseIfFn, class K, class H, class E, class A>
void eraseIf(unordered_set<K, H, E, A>& unordered, EraseIfFn const& f) {
  for (typename unordered_set<K, H, E, A>::iterator i = unordered.begin(), e = unordered.end();
       i != e;)  // no ++i - intentional
    if (f(*i))
      i = unordered.erase(i);
    else
      ++i;
}


template <class EraseIfFn, class K, class V, class H, class E, class A>
void eraseIf(unordered_map<K, V, H, E, A>& unordered, EraseIfFn const& f) {
  for (typename unordered_map<K, H, E, A>::iterator i = unordered.begin(), e = unordered.end();
       i != e;)  // no ++i - intentional
    if (f(*i))
      i = unordered.erase(i);
    else
      ++i;
}

template <class EraseIfFn, class K, class V, class H, class E, class A>
void eraseIf(unordered_multimap<K, V, H, E, A>& unordered, EraseIfFn const& f) {
  for (typename unordered_multimap<K, V, H, E, A>::iterator i = unordered.begin(), e = unordered.end();
       i != e;)  // no ++i - intentional
    if (f(*i))
      i = unordered.erase(i);
    else
      ++i;
}

/// may be faster than regular iteration because iterator is no longer nested
template <class Unordered, class Visitor>
void visitUnordered(Unordered& unordered, Visitor const& v) {
  for (std::size_t b = 0, be = unordered.bucket_count(); b < be; ++b)
    for (typename Unordered::const_local_iterator i = unordered.cbegin(b), end = unordered.cend(b); i != end;)
      v(*i);
}


}}

#endif
