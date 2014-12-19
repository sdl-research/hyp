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

    UninitializedArray: for fixed size pod arrays

    UnconstructedArray: for object arrays. you call placement new. destructors are
    called for you - you must notify the array about each element you construct if
    you want that (if POD, don't bother)
*/

#ifndef UNINITIALIZEDARRAY_JG_2013_07_09_HPP
#define UNINITIALIZEDARRAY_JG_2013_07_09_HPP
#pragma once

#include <memory>
#include <cstdlib>
#include <sdl/Pool/pool.hpp>

namespace sdl {
namespace Util {

/**
   does not construct or zero. simply guards a UserAllocator::malloc / free. space[i<N] may be used as a pod
   array
*/
template <class T, class UserAllocator = Pool::default_user_allocator_new_delete>
struct UninitializedArray {
  typedef T* const_iterator;
  typedef T* iterator;
  std::size_t size() const { return N; }
  typedef T value_type;

  T* space;
  std::size_t N;

  T* begin() const { return space; }
  T* end() const { return space + N; }
  template <class I>
  T& operator[](I i) const {
    return space[i];
  }
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
    N = NT;
    space = (T*)UserAllocator::malloc(sizeof(T) * N);
  }

  UninitializedArray() { init(); }

  UninitializedArray(std::size_t NT) { init(NT); }


  ~UninitializedArray() { free(); }

  void clear() {
    free();
    init();
  }

 private:
  void free() { UserAllocator::free((char*)space); }
};


/**
   for object arrays. you call placement new. destructors are called for you -
   you must notify the array about each element you construct if you want that (if
   POD, don't bother)
*/
template <class V>
struct UnconstructedArray : UninitializedArray<V> {
  typedef UninitializedArray<V> Base;
  UnconstructedArray(std::size_t N) : Base(N), nConstructed() {}
  /**
     for exception safety, call this *after* you construct array[i] - must be called from i=0 ... N-1.
  */
  void constructed(std::size_t i) {
    assert(i == nConstructed);
    nConstructed = i + 1;
  }

  ~UnconstructedArray() {
    for (std::size_t i = 0; i < nConstructed; ++i) this->space[i].~V();
  }

  std::size_t nConstructed;

  void clear() {
    destroy();
    nConstructed = 0;
    Base::clear();
  }

 private:
  void destroy() {
    for (std::size_t i = 0; i < nConstructed; ++i) this->space[i].~V();
  }
};


}}

#endif
