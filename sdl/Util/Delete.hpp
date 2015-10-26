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

    RAII scope-guarded exception safe automatic malloc/free, and delete
    of arrays and single values
*/

#ifndef DELETE_JG_2013_04_23_HPP
#define DELETE_JG_2013_04_23_HPP
#pragma once

#include <sdl/SharedPtr.hpp>
#include <cassert>
#include <cstdlib>
#include <new>
#include <vector>
#include <sdl/Util/SizeIsUnsigned.hpp>
#include <memory>
#include <cstring>
#include <sdl/Pool/poolfwd.hpp>

namespace sdl {
namespace Util {

struct DeleteFn {
  template <class Val>
  void operator()(Val const* val) const {
    delete val;
  }
};

struct DestroyFreeFn {
  template <class Val>
  void operator()(Val const* val) const {
    val->~Val();
    std::free((void*)val);
  }
};

struct FreeFn {
  template <class Val>
  void operator()(Val const* val) const {
    std::free((void*)val);
  }
};

struct DeleteArrayFn {
  template <class Val>
  void operator()(Val const* val) const {
    delete[] val;
  }
};

/**
   similar to boost::scoped_array but calls std::free on dtor.

   can be return by value (copy ctor prevents origin from freeing - like
   auto_ptr)
*/
struct AutoFree {
  operator void*() const { return p_; }
  operator void*&() { return p_; }
  mutable void* p_;
  AutoFree() : p_() {}
  explicit AutoFree(std::size_t mallocBytes) : p_(std::malloc(mallocBytes)) {}
#if !SDL_SIZE_IS_UINT32
  explicit AutoFree(unsigned mallocBytes) : p_(std::malloc(mallocBytes)) {}
#endif
  explicit AutoFree(int mallocBytes) : p_(std::malloc((unsigned)mallocBytes)) {}
  explicit AutoFree(char* p) : p_((void*)p) {}
  explicit AutoFree(char const* p) : p_((void*)p) {}
  explicit AutoFree(void* p) : p_(p) {}
  explicit AutoFree(void const* p) : p_((void*)p) {}
  template <class Data>
  AutoFree(Data *&a, unsigned n)
      : p_(a = (Data*)std::malloc(n * sizeof(Data))) {
  }
  AutoFree(AutoFree&& o) {
    p_ = o.p_;
    o.p_ = 0;
  }
  AutoFree& operator=(AutoFree&& o) {
    assert(this != &o);
    assert(!p_ || p_ != o.p_);
    std::free(p_);
    p_ = o.p_;
    o.p_ = 0;
    return *this;
  }

  AutoFree(AutoFree const& other) = delete;
  AutoFree& operator=(AutoFree& o) = delete;

  ~AutoFree() { std::free(p_); }

  /**
     called repeatedly w/ the same mallocBytes = idempotent. called w/ larger mallocBytes than the last real
     allocation = bug
  */
  void ensure_malloc(std::size_t mallocBytes) {
    if (!p_) malloc(mallocBytes);
  }

  void* malloc(std::size_t mallocBytes) {
    init(std::malloc(mallocBytes));
    return p_;
  }
  void init(void* malloced) { p_ = malloced; }
  void initCopying(void* data, std::size_t n) { std::memcpy(malloc(n), data, n); }
  void set(void* malloced) {
    std::free(p_);
    p_ = malloced;
  }
  void reset(void* malloced) {
    std::free(p_);
    p_ = malloced;
  }
  void free() const {
    std::free(p_);
    p_ = 0;
  }
  /**
     \return p_, which is no longer owned by this Aup_.
  */
  void* release() const {
    void* r = p_;
    p_ = 0;
    return r;
  }
  void nodelete() const { p_ = 0; }
};

struct AutoFreeAll : std::vector<void*> {
  typedef std::vector<void*> Base;
  void hold(void* p) { Base::push_back(p); }
  void* releaseOne() {
    void* r = Base::back();
    Base::pop_back();
    return r;
  }
  void deleteAndClear() {
    for (Base::const_iterator i = Base::begin(), e = Base::end(); i != e; ++i) std::free(*i);
    releaseAll();
  }
  void releaseAll() { Base::clear(); }
  AutoFreeAll() {}
  AutoFreeAll(void* toFree) : Base(1, toFree) {}
  AutoFreeAll(std::size_t n) : Base(n) {}
  ~AutoFreeAll() {
    for (Base::const_iterator i = Base::begin(), e = Base::end(); i != e; ++i) std::free(*i);
  }
  AutoFreeAll(AutoFreeAll const&) = delete;
};


template <class T>
struct AutoDelete {
  operator T*() const { return p_; }
  T* get() const { return p_; }
  T& operator*() const { return *p_; }
  T* operator->() const { return p_; }
  T* p_;
  void reset(T* r = 0) {
    delete p_;
    p_ = r;
  }
  void nodelete() { p_ = 0; }
  T* release() {
    T* r = p_;
    p_ = 0;
    return r;
  }
  T* set(T* r) {
    reset(r);
    return r;
  }
  AutoDelete() : p_() {}
  explicit AutoDelete(T* p) : p_(p) {}
  explicit AutoDelete(T const* p) : p_(const_cast<T*>(p)) {}
  ~AutoDelete() { delete p_; }
  AutoDelete(AutoDelete&& o) {
    p_ = o.p_;
    o.p_ = 0;
  }
  AutoDelete& operator=(AutoDelete&& o) {
    assert(this != &o);
    assert(!p_ || p_ != o.p_);
    delete p_;
    p_ = o.p_;
    o.p_ = 0;
    return *this;
  }
  AutoDelete(AutoDelete const& o) = delete;
  AutoDelete& operator=(AutoDelete const& o) = delete;
  void clone(T const& r) { reset(new T(r)); }
};

template <class T, class Pool = Pool::pool<>>
struct AutoDeletePool {
  Pool& pool_;
  T* p_;
  void destroyFree(T* p) {
    if (p) {
      p->~T();
      pool_.free(p);
    }
  }
  void reset(T* r = 0) {
    destroyFree(p_);
    p_ = r;
  }
  void nodelete() { p_ = 0; }
  T* release() {
    T* r = p_;
    p_ = 0;
    return r;
  }
  AutoDeletePool(Pool& pool, T const* p) : pool_(pool), p_(const_cast<T*>(p)) {}
  ~AutoDeletePool() { destroyFree(p_); }
};

/// use a default-constructed T unless an existing T* was provided
template <class T>
struct ExistingOrTemporary : AutoDelete<T> {
  bool existing_;
  /// new T if existing is NULL
  ExistingOrTemporary(T* existing = 0) : AutoDelete<T>(existing ? existing : new T), existing_(existing) {}
  /// takes a possibly-already-created temporary that's to be used, or if it's null, an existing item
  ExistingOrTemporary(T& fallback, T* temporary)
      : AutoDelete<T>(temporary ? temporary : &fallback), existing_(!temporary) {}
  ~ExistingOrTemporary() {
    if (existing_) this->nodelete();
  }
};

template <class T>
struct AutoDeleteAll : std::vector<T*> {
  typedef std::vector<T*> Base;
  void hold(T* p) { Base::push_back(p); }
  T* releaseOne() {
    T* r = Base::back();
    Base::pop_back();
    return r;
  }
  void deleteAndClear() {
    for (typename Base::const_iterator i = Base::begin(), e = Base::end(); i != e; ++i) delete *i;
    releaseAll();
  }
  void releaseAll() { Base::clear(); }
  AutoDeleteAll() {}
  AutoDeleteAll(T* toDelete) : Base(1, toDelete) {}
  AutoDeleteAll(std::size_t n) : Base(n) {}
  ~AutoDeleteAll() {
    for (typename Base::const_iterator i = Base::begin(), e = Base::end(); i != e; ++i) delete *i;
  }
  AutoDeleteAll(AutoDeleteAll const&) = delete;
};

template <class T>
struct DestroyAll : std::vector<T*> {
  typedef std::vector<T*> Base;
  void hold(T* p) { Base::push_back(p); }
  T* releaseOne() {
    T* r = Base::back();
    Base::pop_back();
    return r;
  }
  void destroyAll() {
    destroyAllImpl();
    releaseAll();
  }
  void releaseAll() { Base::clear(); }
  DestroyAll() {}
  DestroyAll(T* toDestroy) : Base(1, toDestroy) {}
  ~DestroyAll() { destroyAllImpl(); }
  DestroyAll(DestroyAll const&) = delete;
 private:
  void destroyAllImpl() {
    for (typename Base::const_iterator i = Base::begin(), e = Base::end(); i != e; ++i) (*i)->~T();
  }
};

template <class T>
struct AutoDeleteArray {
  operator T*() const { return p_; }
  T* begin() const { return p_; }
  T* p_;
  void init(std::size_t n, T const& x) {
    p_ = new T[n];
    std::uninitialized_fill(p_, p_ + n, x);
  }
  void reset(T* array) { p_ = array; }
  AutoDeleteArray(T* p = 0) : p_(p) {}
  explicit AutoDeleteArray(std::size_t n) : p_(new T[n]()) {}
  AutoDeleteArray(std::size_t n, T const& x) { init(n, x); }
  ~AutoDeleteArray() { delete[] p_; }

  AutoDeleteArray(AutoDeleteArray const&) = delete;
};

/**
   for pointer p in range [i, end), delete p
*/
template <class Iter>
void deleteRange(Iter i, Iter end) {
  for (; i != end; ++i) delete *i;
}

/**
   for container of ptr - delete all ptrs.
*/
template <class Container>
void deleteRange(Container const& c) {
  deleteRange(c.begin(), c.end());
}

template <class Container>
void deleteAndClear(Container& c) {
  deleteRange(c.begin(), c.end());
  c.clear();
}

/**
   for maps to ptr
*/
template <class Iter>
void deleteSecondRange(Iter i, Iter end) {
  for (; i != end; ++i) delete i->second;
}

/**
   for maps to ptr - delete all ptrs
*/
template <class Container>
void deleteSecondRange(Container const& c) {
  deleteSecondRange(c.begin(), c.end());
}

/**
   delete i->first
*/
template <class Iter>
void deleteFirstRange(Iter i, Iter end) {
  for (; i != end; ++i) delete i->first;
}

/**
   delete i->first
*/
template <class Container>
void deleteFirstRange(Container const& c) {
  deleteFirstRange(c.begin(), c.end());
}

template <class Val>
void reconstruct(Val& v) {
  v.~Val();
  new (&v) Val;
}

template <class Val, class A1>
void reconstruct(Val& v, A1& a1) {
  v.~Val();
  new (&v) Val(a1);
}

template <class Val, class A1>
void reconstruct(Val& v, A1 const& a1) {
  v.~Val();
  new (&v) Val(a1);
}


}}

#endif
