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

    managed array set to all 0 bytes.
*/

#ifndef ZEROINITIALIZEDARRAY_JG_2013_05_15_HPP
#define ZEROINITIALIZEDARRAY_JG_2013_05_15_HPP
#pragma once


#include <memory>
#include <sdl/Pool/pool.hpp>
#include <cstdlib>
#include <sdl/Util/Hash.hpp>

namespace sdl { namespace Util {


/**
   like boost::array but for memcmp-able pod that you want set to zero bytes (boost::array
   is fine for non-pod types)

   all 0 bits.
*/
template <class T, std::size_t NT>
struct ZeroInitializedArray
{
  enum { N = NT };
  T space[NT];

  typedef T const* const_iterator;
  typedef T* iterator;
  typedef T value_type;

  static inline std::size_t size() { return NT; }
  static inline std::size_t bytes() { return NT * sizeof(T); }

  T *begin() {
    return space;
  }
  T *end() {
    return space+N;
  }
  T const* begin() const {
    return space;
  }
  T const* end() const {
    return space+N;
  }

  template <class I>
  T &operator[](I i) { return space[i]; }

  template <class I>
  T const& operator[](I i) const { return space[i]; }

  operator T*() { return space; }
  operator T const*() const { return space; }

  void zero() {
    std::memset(space, 0, sizeof(space));
  }

  void clear() {
    zero();
  }

  friend inline std::ostream& operator<<(std::ostream &out, ZeroInitializedArray const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream &out, bool sparse = true) const {
    out << "[";
    for (std::size_t i = 0; i < N; ++i) {
      if (!sparse || space[i])
        out<<' '<<i<<'='<<space[i];
    }
    out << " ]";
  }

  int cmp(ZeroInitializedArray const& o) const {
    return std::memcmp(space, o.space, sizeof(space));
  }
  bool operator==(ZeroInitializedArray const& o) const {
    return cmp(o) == 0;
  }
  bool operator < (ZeroInitializedArray const& o) const {
    return cmp(o) < 0;
  }
  ZeroInitializedArray() { zero(); }

  friend inline std::size_t hash_value(ZeroInitializedArray const& self) {
    return self.hash();
  }
  std::size_t hash(std::size_t seed = 0) const {
    return MurmurHash64(begin(), bytes(), seed);
  }

  /// for MinusOneInitializedArray
  void setMinusOne() {
    std::memset(space, -1, sizeof(space));
  }
  ZeroInitializedArray(bool) { setMinusOne(); }
};

/// all 1 bits
template <class T, std::size_t N>
struct MinusOneInitializedArray : ZeroInitializedArray<T, N>
{
  MinusOneInitializedArray() : ZeroInitializedArray<T, N>(true) {}
};

/**
   for pod only (memset/memcpy).
*/
template <class T, class UserAllocator = Pool::default_user_allocator_new_delete>
struct ZeroInitializedHeapArray
{
  typedef T* const_iterator;
  typedef T* iterator;
  typedef T value_type;

  T *space;
  std::size_t N;

  std::size_t size() const { return N; }
  std::size_t bytes() const { return N * sizeof(T); }

  T *begin() const {
    return space;
  }
  T *end() const {
    return space+N;
  }
  template <class I>
  T &operator[](I i) const { return space[i]; }
  operator T*() const { return space; }

  /**
     init() methods may not be called except on default constructed array (i.e. doesn't free previous).
  */
  void init() {
    space = 0;
    N = 0;
  }

  /**
     don't call unless you free() or clear() first.
  */
  void init(std::size_t NT) {
    alloc_unconstructed(NT);
    zero();
  }

  ZeroInitializedHeapArray() { init(); }

  ZeroInitializedHeapArray(std::size_t NT) { init(NT); }

  ZeroInitializedHeapArray(ZeroInitializedHeapArray const& other) {
    space = alloc_unconstructed(other.N);
    memcpy_from(other.space);
  }

  ~ZeroInitializedHeapArray() {
    free();
  }

  void memcpy_from(void const* from) {
    std::memcpy(space, from, bytes());
  }

  void clear() {
    free();
    init();
  }

  void alloc_unconstructed(std::size_t NT) {
    N = NT;
    space = (T*)UserAllocator::malloc(bytes());
  }

  void zero() {
    if (space)
      std::memset(space, 0, bytes());
  }

  int cmp(ZeroInitializedHeapArray const& o) const {
    return N == o.N ? std::memcmp(space, o.space, bytes()) :
        N < o.N ? -1 : 1;
  }
  bool operator==(ZeroInitializedHeapArray const& o) const {
    return cmp(o) == 0;
  }
  bool operator < (ZeroInitializedHeapArray const& o) const {
    return cmp(o) < 0;
  }
  friend inline std::size_t hash_value(ZeroInitializedHeapArray const& self) {
    return self.hash();
  }
  std::size_t hash(std::size_t seed = 0) const {
    return MurmurHash64(begin(), bytes(), seed + N);
  }

 private:
  void free()
  {
    UserAllocator::free((char*)space);
  }
};


}}

#endif
