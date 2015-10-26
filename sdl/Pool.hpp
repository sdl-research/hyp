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

    Pool pool(allocsz); pool.free(pool.malloc())
*/

#ifndef POOL_GRAEHL_2015_10_21_HPP
#define POOL_GRAEHL_2015_10_21_HPP
#pragma once

#include <sdl/Pool-fwd.hpp>
#include <sdl/Pool/pool.hpp>
#include <utility>

namespace sdl {

template <class T>
void deletePool(T* t, ChunkPool& pool) {
  t->~T();
  assert(pool.get_requested_size() >= sizeof(T));  // subclass could be larger
  pool.free(t);
}

template <class T>
void deletePool(T* t, ChunkPool* pool) {
  if (pool)
    deletePool(t, *pool);
  else
    delete t;
}

template <class I>
void deleteRangePool(I begin, I end, ChunkPool& pool) {
  for (; begin != end; ++begin) deletePool(*begin, pool);
}

template <class T, class... Args>
void constructFromPool(T*& t, ChunkPool& pool, Args&&... args) {
  assert(pool.get_requested_size() == sizeof(T));
  t = (T*)pool.malloc();
  new (t) T(std::forward<Args>(args)...);
}

template <class T>
struct Destroy {
  T* p_;
  operator T&() const { return *p_; }
  T* get() const { return p_; }
  T& operator*() const { return *p_; }
  T* operator->() const { return p_; }
  Destroy(T* p = 0) : p_(p) {}
  void destroy() {
    assert(p_);
    p_->~T();
    p_ = 0;
  }
  void release() { p_ = 0; }
  ~Destroy() {
    if (p_) p_->~T();
  }
};

template <class T>
struct Constructed : Destroy<T> {
  template <class... Args>
  Constructed(T* p = 0, Args&&... args)
      : Destroy<T>() {
    new (p) T(std::forward<Args>(args)...);
    this->p_ = p;  // exception safety for T ctor
  }
};

template <class T>
struct PoolDelete {
  T* p_;
  ChunkPool& pool_;
  operator T&() const { return *p_; }
  T* get() const { return p_; }
  T& operator*() const { return *p_; }
  T* operator->() const { return p_; }
  PoolDelete(ChunkPool& pool, T* p = 0) : p_(p), pool_(pool) {
    assert(pool.get_requested_size() >= sizeof(T));
  }
  void destroy() {
    destroyImpl();
    p_ = 0;
  }
  void release() { p_ = 0; }
  ~PoolDelete() {
    if (p_) destroyImpl();
  }

 private:
  void destroyImpl() const {
    assert(p_);
    p_->~T();
    pool_.free(p_);
  }
};

template <class T>
struct PoolConstructed : PoolDelete<T> {
  template <class... Args>
  PoolConstructed(ChunkPool& pool, Args&&... args)
      : PoolDelete<T>(pool) {
    assert(pool.get_requested_size() == sizeof(T));
    T* p = pool.malloc();
    new (p) T(std::forward<Args>(args)...);
    this->p_ = p;  // exception safety for T ctor
  }
};


/// shared_ptr deleters for memory allocated from pool. class template rather
/// than member so we can have shared_ptr<base>.
template <class T>
struct Destroyer {
  void operator()(void const* p) const { ((T*)p)->~T(); }
};

template <class T>
struct PoolFreeDeleter {
  ChunkPool& pool_;
  PoolFreeDeleter(ChunkPool& pool) : pool_(pool) {}
  void operator()(void const* p) const {
    ((T*)p)->~T();
    pool_.free((void*)p);
  }
};


}

#endif
