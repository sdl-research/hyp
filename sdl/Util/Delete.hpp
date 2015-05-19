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
#include <cstdlib>
#include <new>
#include <vector>
#include <sdl/Util/SizeIsUnsigned.hpp>
#include <memory>
#include <cstring>

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
  operator void*() const { return toFree; }
  operator void*&() { return toFree; }
  mutable void* toFree;
  AutoFree() : toFree() {}
  explicit AutoFree(std::size_t mallocBytes) : toFree(std::malloc(mallocBytes)) {}
#if !SDL_SIZE_IS_UINT32
  explicit AutoFree(unsigned mallocBytes) : toFree(std::malloc(mallocBytes)) {}
#endif
  explicit AutoFree(int mallocBytes) : toFree(std::malloc((unsigned)mallocBytes)) {}
  explicit AutoFree(char* toFree) : toFree((void*)toFree) {}
  explicit AutoFree(char const* toFree) : toFree((void*)toFree) {}
  explicit AutoFree(void* toFree) : toFree(toFree) {}
  explicit AutoFree(void const* toFree) : toFree((void*)toFree) {}
  AutoFree(AutoFree const& other) : toFree(other.toFree) { other.release(); }
  /**
     called repeatedly w/ the same mallocBytes = idempotent. called w/ larger mallocBytes than the last real
     allocation = bug
  */
  void ensure_malloc(std::size_t mallocBytes) {
    if (!toFree) malloc(mallocBytes);
  }

  void* malloc(std::size_t mallocBytes) {
    init(std::malloc(mallocBytes));
    return toFree;
  }
  void init(void* malloced) { toFree = malloced; }
  void initCopying(void* data, std::size_t n) { std::memcpy(malloc(n), data, n); }
  void set(void* malloced) {
    std::free(toFree);
    toFree = malloced;
  }
  void reset(void* malloced) {
    std::free(toFree);
    toFree = malloced;
  }
  void free() const {
    std::free(toFree);
    toFree = 0;
  }
  /**
     \return toFree, which is no longer owned by this AutoFree.
  */
  void* release() const {
    void* r = toFree;
    toFree = 0;
    return r;
  }
  ~AutoFree() { std::free(toFree); }
};

template <class T>
struct AutoDestroy {
  AutoDestroy(T *p) : p_(p) {}
  T *p_;
  void destroy() {
    assert(p_);
    p_->~T();
    p_ = 0;
  }
  ~AutoDestroy() {
    if (p_)
      p_->~T();
  }
};

template <class T>
struct AutoDelete {
  operator T*() const { return toDelete; }
  T* get() const { return toDelete; }
  T& operator*() const { return *toDelete; }
  T* operator->() const { return toDelete; }
  T* toDelete;
  void reset(T* r = 0) {
    delete toDelete;
    toDelete = r;
  }
  T* release() {
    T* r = toDelete;
    toDelete = 0;
    return r;
  }
  AutoDelete(T* toDelete = 0) : toDelete(toDelete) {}
  AutoDelete(T const* toDelete) : toDelete((T*)toDelete) {}
  ~AutoDelete() { delete toDelete; }
  void clone(T const& r) {
    reset(new T(r));
  }
#if __cplusplus >= 201103L
  AutoDelete(AutoDelete &&o) {
    toDelete = o.toDelete;
    o.toDelete = 0;
  }
  AutoDelete& operator=(AutoDelete &&o) {
    assert(this != &o);
    reset(o.toDelete);
    o.toDelete = 0;
    return *this;
  }
  AutoDelete(AutoDelete const&o) = delete;
  AutoDelete& operator=(AutoDelete const&o) = delete;
#else
 private:
  AutoDelete(AutoDelete const&) { std::abort(); }
  void operator=(AutoDelete const&o) { std::abort(); }
#endif
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
    if (existing_) this->toDelete = 0;
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

 private:
  AutoDeleteAll(AutoDeleteAll const&) {}
};

template <class T>
struct AutoDeleteArray {
  operator T*() const { return toDelete; }
  T* begin() const { return toDelete; }
  T* toDelete;
  void reset(T* array) { toDelete = array; }
  AutoDeleteArray(T* toDelete = 0) : toDelete(toDelete) {}
  explicit AutoDeleteArray(std::size_t n) : toDelete(new T[n]()) {}
  AutoDeleteArray(std::size_t n, T const& x) : toDelete(new T[n]) {
    std::uninitialized_fill(toDelete, toDelete + n, x);
  }
  ~AutoDeleteArray() { delete[] toDelete; }

 private:
  AutoDeleteArray(AutoDeleteArray const&) {}
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
