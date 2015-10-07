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

template <class Array>
void bzeroArray(Array const& a) {
  std::memset(&a[0], 0, sizeof(a[0])*a.size());
}


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
    assert(sizeof(space) == NT * sizeof(T));
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

  ZeroInitializedArray() { zero(); }
  explicit ZeroInitializedArray(bool) { setMinusOne(); }
  ZeroInitializedArray(ZeroInitializedArray &&)=default;
  ZeroInitializedArray& operator=(ZeroInitializedArray &&)=default;
  ZeroInitializedArray(ZeroInitializedArray const&)=default;
  ZeroInitializedArray& operator=(ZeroInitializedArray const&)=default;
  friend inline void swap(ZeroInitializedArray & x1, ZeroInitializedArray & x2) {
    x1.swap(x2);
  }
  void swap(ZeroInitializedArray &o) {
    ZeroInitializedArray tmp;
    tmp = o;
    o = *this;
    *this = tmp;
  }
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

  bool empty() const { return !N; }
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
    copyImpl(other);
  }
  ZeroInitializedHeapArray& operator=(ZeroInitializedHeapArray const& other) {
    free();
    copyImpl(other);
    return *this;
  }

  void copyImpl(ZeroInitializedHeapArray const& other) {
    alloc_unconstructed(other.N);
    memcpy_from(other.space);
  }

  ZeroInitializedHeapArray(ZeroInitializedHeapArray &&o) {
    space = o.space;
    N = o.N;
    o.space = 0;
  }
  ZeroInitializedHeapArray& operator=(ZeroInitializedHeapArray &&o) {
    assert(this != &o);
    free();
    space = o.space;
    N = o.N;
    o.space = 0;
    return *this;
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

  void reinit(std::size_t NT) {
    if (N != NT) {
      free();
      alloc_unconstructed(NT);
    }
    zero();
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

  void swapSameSize(ZeroInitializedHeapArray &o) {
    T *tmp = o.space;o.space = space;space = tmp;
  }
  friend inline void swap(ZeroInitializedHeapArray & x1, ZeroInitializedHeapArray & x2) {
    x1.swap(x2);
  }
  void swap(ZeroInitializedHeapArray &o) {
    std::size_t tmp = N; o.N = N; N = tmp;
    swapSameSize(o);
  }

 private:
  void free()
  {
    UserAllocator::free((char*)space);
  }
};

/// uncopyable (but can swap/move). can't == < or zero without passing in size from outside
template <class T, class UserAllocator = Pool::default_user_allocator_new_delete>
struct UnsizedArray
{
  typedef T* const_iterator;
  typedef T* iterator;
  typedef T value_type;

  T *space;

  bool empty() const { return !space; }

  T *begin() const {
    return space;
  }

  template <class I>
  T &operator[](I i) const { return space[i]; }
  operator T*() const { return space; }

  /**
     init() methods may not be called except on default constructed array (i.e. doesn't free previous).
  */
  void init() {
    space = 0;
  }

  /**
     don't call unless you free() or clear() first.
  */
  void init(std::size_t N) {
    alloc_unconstructed(N);
    zero(N);
  }

  UnsizedArray() { init(); }

  explicit UnsizedArray(std::size_t N) { init(N); }

  ~UnsizedArray() {
    free();
  }

  void memcpy_from(void const* from, unsigned N) {
    std::memcpy(space, from, N * sizeof(T));
  }

  void clear() {
    free();
    init();
  }

  void alloc_unconstructed(std::size_t N) {
    space = (T*)UserAllocator::malloc(N * sizeof(T));
  }

  void zero(std::size_t N) {
    std::memset(space, 0, N * sizeof(T));
  }

  void reinit(std::size_t N) {
    free();
    alloc_unconstructed(N);
    zero(N);
  }

  void swapSameSize(UnsizedArray &o) {
    T *tmp = o.space;o.space = space;space = tmp;
  }
  friend inline void swap(UnsizedArray & x1, UnsizedArray & x2) {
    x1.swapSameSize(x2);
  }

  UnsizedArray(UnsizedArray &&o) {
    space = o.space;
    o.space = 0;
  }
  UnsizedArray& operator=(UnsizedArray &&o) {
    assert(this != &o);
    free();
    space = o.space;
    o.space = 0;
    return *this;
  }
  UnsizedArray(UnsizedArray const&o) = delete;
  UnsizedArray& operator=(UnsizedArray const&o) = delete;

private:
  void free()
  {
    UserAllocator::free((char*)space);
  }
};


}}

#endif
