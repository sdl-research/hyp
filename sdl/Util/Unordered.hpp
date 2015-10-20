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


#include <boost/functional/hash/hash.hpp>
#include <boost/unordered_map.hpp>
#include <sparsehash/dense_hash_map>
#include <sparsehash/dense_hash_set>

#include <boost/unordered_set.hpp>

#include <iterator>
#include <cstdlib>
#include <cstddef>
#include <sdl/Util/Compare.hpp>
#include <sdl/Util/Contains.hpp>
#include <sdl/Util/Map.hpp>
#include <cmath>
#include <sdl/Util/HashValue.hpp>

namespace sdl {

/// unordered_* are node pointer based (chaining on collision) hash tables and
/// if there are resizes should outperform for heavy non-moveable keys/values
/// after many insertions
using boost::unordered_map;
using boost::unordered_set;
using boost::unordered_multimap;
//TODO: benchmark std vs boost unordered_* or just use google dense_hash below:

/// dense_hash_map/set: m.set_empty_key(k); where k won't be used as a normal
/// key - otherwise like unordered_map (if you erase, then set_deleted_key(k2)
/// too). you may also have to specify default hash fn obj = boost::hash<Key>
/// also, it's sort-of ok to use the default identity hash for pointers since
/// google drops the low 3 bits already. i.e. don't use PointerHash w/
/// dense_hash...
template <class Key, class T, class HashFcn = boost::hash<Key>, class EqualKey = std::equal_to<Key>,
          class Alloc = google::libc_allocator_with_realloc<std::pair<const Key, T>>>
using hash_map = google::dense_hash_map<Key, T, HashFcn, EqualKey, Alloc>;

template <class Key, class HashFcn = boost::hash<Key>, class EqualKey = std::equal_to<Key>,
          class Alloc = google::libc_allocator_with_realloc<Key>>
using hash_set = google::dense_hash_set<Key, HashFcn, EqualKey, Alloc>;

/// dense_hash_map and dense_hash_set have faster repeated lookups; also uses
/// slightly more memory than unordered_ unless the (key, val) pair is small
/// (because of unordered_map chaining: sizeof(pointer) for empty slot vs
/// sizeof(entry))

namespace Util {


/// for repeated use of same table, clear but don't free memory prematurely
template <class HashContainer>
void clearNoResize(HashContainer& h) {
  h.clear();
}

template <class K, class V, class H, class E, class A>
void clearNoResize(hash_map<K, V, H, E, A>& h) {
  h.clear_no_resize();
}

template <class K, class H, class E, class A>
void clearNoResize(hash_set<K, H, E, A>& h) {
  h.clear_no_resize();
}

template <class HashContainer>
void setEmptyKey(HashContainer& h, typename HashContainer::key_type const& k) {
}

template <class K, class V, class H, class E, class A>
void setEmptyKey(hash_map<K, V, H, E, A>& h, K const& k) {
  h.set_empty_key(k);
}

template <class K, class H, class E, class A>
void setEmptyKey(hash_set<K, H, E, A>& h, K const& k) {
  h.set_empty_key(k);
}

template <class HashContainer>
void setEmptyKey(HashContainer& h) {
  setEmptyKey(h, typename HashContainer::key_type());
}

template <class HashMap>
HashMap *new_hashed() {
  HashMap *m = new HashMap;
  setEmptyKey(*m);
  return m;
}

template <class HashMap>
shared_ptr<HashMap> make_hashed() {
  shared_ptr<HashMap> m = make_shared<HashMap>();
  setEmptyKey(*m);
  return m;
}

template <class HashMap>
HashMap *new_hashed(typename HashMap::key_type const& k) {
  HashMap *m = new HashMap;
  setEmptyKey(*m, k);
  return m;
}

template <class HashMap>
shared_ptr<HashMap> make_hashed(typename HashMap::key_type const& k) {
  shared_ptr<HashMap> m = make_shared<HashMap>();
  setEmptyKey(*m, k);
  return m;
}

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

template <class K, class V, class H, class E, class A>
void reserveUnordered(hash_map<K, V, H, E, A>& umap, std::size_t n) {
  reserveUnorderedImpl(umap, n);
}

template <class K, class H, class E, class A>
void reserveUnordered(hash_set<K, H, E, A>& uset, std::size_t n) {
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
