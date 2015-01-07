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

    get thread id of current thread, and declare/check that there's only one thread, ever.
*/

#ifndef THREADID_JG_2014_01_06_HPP
#define THREADID_JG_2014_01_06_HPP
#pragma once

#include <cstdlib>

#ifdef _MSC_VER
//TODO (fix isStackAddress fn in ThreadId.hpp)
# undef SDL_ASSERT_THREAD_SPECIFIC
# define SDL_ASSERT_THREAD_SPECIFIC 0
#endif

#ifndef SDL_ASSERT_THREAD_SPECIFIC
# define SDL_ASSERT_THREAD_SPECIFIC !defined(NDEBUG)
#endif

namespace sdl { namespace Util {

typedef std::size_t ThreadId;

namespace impl {
extern bool gSingleThreadProgram;
extern bool gFixedSingleThreadProgram;
extern ThreadId gLastThreadId;
}

/**
   you may call this once only, and before anyone checks isSingleThreadProgram()

   this means mostly ensuring that there are no global ThreadSpecific... objects
   (THREADLOCAL and ThreadLocalSingleton are fine, though)
*/
void setSingleThreadProgram(bool singleThread = true);

struct DeclareSingleThreadProgram {
  DeclareSingleThreadProgram(bool singleThread = true) {
    setSingleThreadProgram(singleThread);
  }
};

/**
   call this from more than one thread to get an exception if we setSingleThreadProgram(true).
*/
void maybeCheckSingleThread();

/**
   call this from more than one thread to get an exception
*/
void checkSingleThread();

/// this impl detail fn is a good place to set a breakpoint if you want to know
/// about why the process is flagged as single or multi thread (perhaps some
/// static init code causes it to be set already).
void setFixedSingleThread();

/**
   may check as many times as you like.
*/
inline bool isSingleThreadProgram() {
#if SDL_ASSERT_THREAD_SPECIFIC || NDEBUG
  if (impl::gSingleThreadProgram)
    checkSingleThread();
#endif
  setFixedSingleThread();
  return impl::gSingleThreadProgram;
}

ThreadId threadId();


namespace {
#ifdef _MSC_VER
std::size_t const kStackMask = 0; //TODO
#else
// linux
//TODO: mac
std::size_t const kStackMask = sizeof(void *) == 8 ? 0x7f0000000000 : 0xb0000000;
#endif
}

inline bool isStackAddress(void const* ptr) {
#ifdef _MSC_VER
  //TODO: find api fn or 32 and 64 kStackMask
  return false;
#else
  return ((std::size_t)ptr & kStackMask) == kStackMask;
#endif
}


}}

#endif
