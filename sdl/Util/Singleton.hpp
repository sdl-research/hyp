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

    thread-safe lazy-initialized process-global object-per-type.

    synopsis: ++singleton<int>() - increments a global counter

    struct MyIntTag {};

    ++singleton<int, MyIntTag>() - a distinct counter associated with the type MyIntTag

    (C++11 can use a std::once_flag which can be default constructed, but with
    boost::once_flag, this thread-safety technique is limited to globals)

    note: if you #define SDL_FREE_SINGLETON 0 then we don't free any of these
    singletons; memory leaks will be reported, but there's really no reason to
    waste time freeing a singleton in general (unless you're depending on
    destructor side effects).

    we rely on the memory model making atomic updates to default (0) initialized
    pointers (either it's 0, or updated completely from the
    call_once(initSingleton)).

    /// C++11 guarantees that construction of static local vars (which are really
    /// globals) is thread-safe. but boost::call_once is pretty efficient too (see
    /// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2660.htm)

    */


#ifndef SINGLETON_ONCE_JG2012102_HPP
#define SINGLETON_ONCE_JG2012102_HPP
#pragma once

#ifndef SDL_STATIC_LOCAL_INIT_ATOMIC
#define SDL_STATIC_LOCAL_INIT_ATOMIC 1
#endif

#if !SDL_STATIC_LOCAL_INIT_ATOMIC
#include <boost/thread/once.hpp>
#include <boost/thread/thread.hpp>
#include <stdexcept>
#endif

#ifndef SDL_FREE_SINGLETON
#define SDL_FREE_SINGLETON 1
#endif

namespace sdl {
namespace Util {

/**
   Eager-initialized singleton. Not safe to use this until static initialization
   is done. If you want more than one singleton of the same type, use
   InstanceTag (any type, contents not used).

   Note that you *could* just define a regular global T. But this is convenient
   for switching between lazy Singleton and EagerSingleton.
*/
template <class T, class InstanceTag = void>
struct EagerSingleton {
  static T singleton;
  static inline T& get() { return singleton; }
};

template <class T, class Tag>
T EagerSingleton<T, Tag>::singleton;

/**
   Lazy-initialized singleton, safe to use in both static initialization and in
   threads. If you want more than one singleton of the same type, use
   InstanceTag (any type, contents not used).
*/
template <class T, class InstanceTag = void>
struct Singleton {
#if SDL_STATIC_LOCAL_INIT_ATOMIC
  static T& get() {
    static T singleton;
    return singleton;
  }
#else
  static inline T& get() {
    for (;;) {
      // pSingleton is volatile so this is partial memory fence on windows
      if (pSingleton) return *(T*)pSingleton;
      boost::call_once(once, &initSingleton);
      // now either constructException (eventually), or pSingleton (memory fenced)
      if (constructException) throw std::runtime_error(constructExceptionWhat);
    }
  }

 private:
  struct FreeSingleton {
#if SDL_FREE_SINGLETON
    ~FreeSingleton() {
      delete pSingleton;  // don't need to check 0
      pSingleton = 0;  // unnecessary but feels safer
    }
#endif
  };

  /**
     to be called only once.
  */
  static void initSingleton() {
    assert(!pSingleton);
    try {
      volatile T* volatile constructed = new volatile T();

      /* MSDN docs say this will ensure pSingleton is not updated until constructor terminates:

         When optimizing, the compiler must maintain ordering among references to
         volatile objects as well as references to other global objects. In
         particular,

         A write to a volatile object (volatile write) has Release semantics; a
         reference to a global or static object that occurs before a write to a
         volatile object in the instruction sequence will occur before that
         volatile write in the compiled binary.

         A read of a volatile object (volatile read) has Acquire semantics; a
         reference to a global or static object that occurs after a read of
         volatile memory in the instruction sequence will occur after that
         volatile read in the compiled binary.
      */
      pSingleton = (T*)constructed;
    } catch (std::exception& e) {
      constructException = true;
      constructExceptionWhat = "singleton constructor: ";
      constructExceptionWhat.append(e.what());
    } catch (...) {
      constructException = true;
      constructExceptionWhat = "singleton constructor: unknown exception";
    }
  }
  static boost::once_flag once;
  static volatile T* pSingleton;  // NOTE: tried to use scoped_ptr but realized that the ctor might be called
  // after another static initializer has used it! bare pointer is safe.
  static FreeSingleton freeSingleton;  // call singleton destructor if needed, on static destruction
  static std::string constructExceptionWhat;
  static bool constructException;
#endif
};


#if !SDL_STATIC_LOCAL_INIT_ATOMIC
template <class T, class Tag>
boost::once_flag Singleton<T, Tag>::once = BOOST_ONCE_INIT;

template <class T, class Tag>
volatile T* Singleton<T, Tag>::pSingleton;

template <class T, class Tag>
std::string Singleton<T, Tag>::constructExceptionWhat;

template <class T, class Tag>
bool Singleton<T, Tag>::constructException;

template <class T, class Tag>
typename Singleton<T, Tag>::FreeSingleton Singleton<T, Tag>::freeSingleton;
#endif

template <class T>
T& singleton() {
  return Singleton<T>::get();
}

template <class T, class InstanceTag>
T& singleton() {
  return Singleton<T, InstanceTag>::get();
}


}}

#endif
