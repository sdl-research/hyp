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

#ifndef THREADSPECIFIC_JG2012917_HPP
#define THREADSPECIFIC_JG2012917_HPP
#pragma once

#ifndef SDL_USE_PTHREAD_THREAD_SPECIFIC
#if defined(_MSC_VER)
#define SDL_USE_PTHREAD_THREAD_SPECIFIC 0
#elif SDL_VALGRIND
#define SDL_USE_PTHREAD_THREAD_SPECIFIC 0
#else
/**
   uninit warning if 1:
~ThreadSpecificInt (ThreadSpecific.hpp:100) - Util::ThreadSpecificBool originalAlreadyIncluded
~IWordToPhrase (WordToPhrase.hpp:125)
~IStringToString (StringToString.hpp:37)
~Recase (Lowercase.hpp:26)
~configure_info_for (configure_is.hpp:100)
std::string configure::configure_usage<sdl::Lowercase::Recase>() (configure_is.hpp:123)
*/
#define SDL_USE_PTHREAD_THREAD_SPECIFIC 0
#endif
#endif

#include <sdl/Util/Delete.hpp>
#include <sdl/Util/Errno.hpp>
#include <sdl/Util/ThreadId.hpp>
#include <sdl/IntTypes.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/tss.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <stdint.h>
// warning: std::uintptr_t is in C++11 only. most compilers' stdint.h will have uintptr_t, though
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
#include <pthread.h>
#else
#include <boost/shared_ptr.hpp>
#endif

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

inline bool isThreadStackAddress(void const* p) {
  return false;
  // TODO: pthread_attr_getstack can tell us precisely, but is it faster than using tss?
}

template <class Int>
struct ThreadSpecificInt : boost::noncopyable {
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
  static_assert(sizeof(Int) <= sizeof(PtrInt), "ThreadSpecificInt must be no larger than void*");
  void set(Int i) {
    if (alreadyThreadSpecific())
      ki.i = i;
    else
      setThreadSpecific(i);
  }
  void setThreadSpecific(Int i) {
#if SDL_USE_PTHREAD_THREAD_SPECIFIC
    (void)::pthread_setspecific(ki.key, (void const*)static_cast<PtrInt>(i));
#else
    // TODO: why not sdl::shared_ptr (std)
    impl::set_tss_data(this, boost::shared_ptr<impl::tss_cleanup_function>(), (void*)static_cast<PtrInt>(i),
                       false);
// Boost (pre- 1.52) bug on Windows: must past pointer to do-nothing tss_cleanup_function instead of nullptr
#endif
  }
  typedef ThreadSpecificInt<Int> Self;

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
template <class Value>
struct ThreadSpecific : boost::noncopyable {
  Value* local_;
  enum { isThreadStack = false };
  bool alreadyThreadSpecific() const { return isSingleThreadProgram() || isThreadStack; }
  typedef boost::thread_specific_ptr<Value> PVal;
  static void cleanFun(Value* p) {
    if (p) {
      p->~Value();
      std::free(p);
    }
  }
  ThreadSpecific() : pVal(cleanFun), local_() {}
  ~ThreadSpecific() {
    if (alreadyThreadSpecific() && local_) {
      local_->~Value();
      std::free(local_);
    } else {
      assert(!local_);
    }
  }
  PVal pVal;
  operator Value&() { return get(); }

  Value& get() {
    maybeCheckSingleThread();
    if (alreadyThreadSpecific()) {
      if (!local_) {
        local_ = alloc();
        new (local_) Value();
      }
      return *local_;
    } else {
      Value* v = pVal.get();
      if (!v) {
        v = alloc();
        new (v) Value();
        pVal.reset(v);
      }
      return *v;
    }
  }

  /// \return whether v needs to be constructed (will only be true once per
  /// thread) - sets v to point at thread specific value
  bool isUnconstructed(Value*& v) {
    if (alreadyThreadSpecific()) {
      if (local_) {
        v = local_;
        return false;
      } else {
        local_ = v = alloc();
        return true;
      }
    } else {
      v = pVal.get();
      if (v)
        return false;
      else {
        pVal.reset(v = alloc());
        return true;
      }
    }
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
  static Value* alloc() { return (Value*)std::malloc(sizeof(Value)); }
};


}}

#endif
