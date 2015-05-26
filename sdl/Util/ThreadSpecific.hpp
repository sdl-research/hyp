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

    thread specific objects (and their pointers) must not be passed to other
    threads. if they exist on the thread stack then they're already safe to write/read
    for just that thread. if they exist on the heap then they're like a map
    keyed on thread-id - each thread has its own slot
*/

#ifndef THREADSPECIFICMAP_JG2012917_HPP
#define THREADSPECIFICMAP_JG2012917_HPP
#pragma once


#ifndef SDL_USE_PTHREAD_THREAD_SPECIFIC
#if defined(_MSC_VER)
#define SDL_USE_PTHREAD_THREAD_SPECIFIC 0
#else
#if SDL_VALGRIND
#define SDL_USE_PTHREAD_THREAD_SPECIFIC 0
#else
// this gives false-positive uninit conditional jump (because
// pthread_getspecific fools valgrind). //TODO: check performance to see if
// pthread or boost (which uses a threadlocal std::map<void *...> is faster
#define SDL_USE_PTHREAD_THREAD_SPECIFIC 1
#endif
#endif
#endif

#include <string>
#include <memory>
#include <boost/thread/tss.hpp>
#include <sdl/SharedPtr.hpp>
#include <sdl/IntTypes.hpp>
#include <sdl/Util/ThreadId.hpp>

#if SDL_USE_PTHREAD_THREAD_SPECIFIC
#include <pthread.h>
#endif
#include <stdint.h>
#include <sdl/Util/Errno.hpp>
#include <sdl/Util/Delete.hpp>
// warning: std::uintptr_t is in C++11 only. most compilers' stdint.h will have uintptr_t, though

namespace sdl {
namespace Util {

using boost::thread_specific_ptr;

namespace impl {
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
using ::pthread_getspecific;
using ::pthread_setspecific;
using ::pthread_key_create;
using ::pthread_key_delete;
using ::pthread_key_t;
#else
using boost::detail::get_tss_data;
using boost::detail::set_tss_data;
using boost::detail::tss_cleanup_function;
#endif
}

// Boost for Windows has a TSS bug. Probably fixed in 1.52
// https://svn.boost.org/trac/boost/ticket/5696
#if BOOST_VERSION < 105200 && defined(_WIN32)
#define SDL_BOOST_TSS_BUG 1
shared_ptr<impl::tss_cleanup_function> makeDummyTssCleanupFunction();
#else
#define SDL_BOOST_TSS_BUG 0
#endif

inline bool isThreadStackAddress(void const* p) {
  return false;
  // TODO: pthread_attr_getstack can tell us precisely, but is it faster than using tss?
}

template <class Int>
struct ThreadSpecificInt {
  typedef uintptr PtrInt;
  union KeyOrInt {
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
    ::pthread_key_t key;
#endif
    Int i;
  };
  KeyOrInt ki;
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
  void check(int pthread_errno, char const* callname = "ThreadSpecificInt pthread error") {
    if (pthread_errno) throw_errno(callname);
  }
#endif
  enum { isThreadStack = false };
  bool alreadyThreadSpecific() const { return isSingleThreadProgram() || isThreadStack; }

  ThreadSpecificInt() { init(Int()); }
  ThreadSpecificInt(ThreadSpecificInt const& o) { init(o); }
  ThreadSpecificInt(Int i) { init(i); }
  ~ThreadSpecificInt() {
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
    check(::pthread_key_delete(ki.key), "pthread_key_delete");
#endif
  }
  operator Int() const { return get(); }
  void operator=(Int i) { set(i); }
  void operator=(ThreadSpecificInt const& i) { set(i); }
  Int get() const {
    if (alreadyThreadSpecific()) return ki.i;
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
    return static_cast<Int>((PtrInt)::pthread_getspecific(ki.key));
// will return NULL=0 if val not set on this thread
#else
    return static_cast<Int>((PtrInt)impl::get_tss_data(this));
#endif
  }
  void set(Int i) {
    if (alreadyThreadSpecific())
      ki.i = i;
    else
      setThreadSpecific(i);
  }
  void setThreadSpecific(Int i) {
    BOOST_STATIC_ASSERT(sizeof(Int) <= sizeof(PtrInt));
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
    (void)::pthread_setspecific(ki.key, (void const*)static_cast<PtrInt>(i));
#else
#if !SDL_BOOST_TSS_BUG
    impl::set_tss_data(this, shared_ptr<impl::tss_cleanup_function>(), (void*)static_cast<PtrInt>(i), false);
#else
    // Boost (before 1.52) bug on Windows: The cleanup function must be provided.
    impl::set_tss_data(this, makeDummyTssCleanupFunction(), (void*)static_cast<PtrInt>(i), false);
#endif
#endif
  }
  typedef ThreadSpecificInt<Int> Self;
  // swaps values for this thread only
  friend inline void swap(Self& a, Self& b) {
    Int tmp = a;
    a = b;
    b = tmp;
  }

 private:
  void init(Int i) {
    maybeCheckSingleThread();
    // isThreadStack = isThreadStackAddress(this); //TODO
    if (alreadyThreadSpecific())
      ki.i = i;
    else {
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
      check(::pthread_key_create(&ki.key, NULL), "pthread_key_create");
// Upon key creation, the value NULL shall be associated with the new key in all active threads. Upon thread
// creation, the value NULL shall be associated with all defined keys in the new thread.
#endif
      setThreadSpecific(i);
    }
  }
};

typedef ThreadSpecificInt<bool> ThreadSpecificBool;
typedef ThreadSpecificInt<unsigned> ThreadSpecificUnsigned;
typedef ThreadSpecificInt<std::size_t> ThreadSpecificSize;

/** new Value() per thread. */
template <class Value, class Allocator = std::allocator<Value> >
struct ThreadSpecific : Allocator {
  Value* local_;
  enum { isThreadStack = false };
  bool alreadyThreadSpecific() const { return isSingleThreadProgram() || isThreadStack; }
  static void cleanFun(Value* p) {
    if (p) {
      p->~Value();
      Allocator().deallocate(p, 1);
    }
  }
  typedef boost::thread_specific_ptr<Value> PVal;
  ThreadSpecific(Allocator allocator = Allocator()) : Allocator(allocator), pVal(cleanFun), local_() {}
  ~ThreadSpecific() {
    if (alreadyThreadSpecific())
      delete local_;
    else {
      assert(!local_);
    }
  }
  PVal pVal;
  operator Value&() { return get(); }
  Value& get() {
    if (alreadyThreadSpecific()) {
      if (local_) return *local_;
      local_ = (Value*)Allocator::allocate(1);
      new (local_) Value();
      return *local_;
    }
    Value* v = pVal.get();
    if (v) return *v;
    v = (Value*)Allocator::allocate(1);
    new (v) Value();
    pVal.reset(v);
    return *v;
  }
  /// \return whether v needs to be constructed (will only be true once per
  /// thread) - sets v to point at thread specific value
  bool isUnconstructed(Value*& v) {
    if (alreadyThreadSpecific()) {
      if (local_) {
        v = local_;
        return false;
      }
      v = (Value*)Allocator::allocate(1);
      local_ = v;
      return true;
    }
    v = pVal.get();
    if (v) return false;
    v = (Value*)Allocator::allocate(1);
    pVal.reset(v);
    return true;
  }
  Value* getPtr() const { return alreadyThreadSpecific() ? local_ : pVal.get(); }
  Value const& get() const {
    Value* v = getPtr();
    assert(v != NULL);
    return *v;
  }
  bool got() const { return getPtr(); }
  void maybeAssign(ThreadSpecific const& o) {
    if (o.got()) get() = o.get();
  }

 private:
  ThreadSpecific(ThreadSpecific const& o);
};


}}

#endif
