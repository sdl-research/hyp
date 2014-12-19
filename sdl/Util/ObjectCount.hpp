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

    for debugging leaks, count number of live objects. use with LeakCheck.hpp

*/

#ifndef COUNTOBJECTS_JG_2014_03_03_HPP
#define COUNTOBJECTS_JG_2014_03_03_HPP
#pragma once

#include <cassert>
#include <sdl/Util/RefCount.hpp>
#include <sdl/Util/LogHelper.hpp>

#ifndef SDL_OBJECT_COUNT
#if SDL_MEMSTATS
#define SDL_OBJECT_COUNT 1
#endif
#endif

#ifndef SDL_OBJECT_COUNT
#define SDL_OBJECT_COUNT 0
#endif

#ifndef SDL_EXPLICIT_OBJECT_COUNT
#define SDL_EXPLICIT_OBJECT_COUNT 0
#endif

#ifndef SDL_OBJECT_COUNT_TRACE
#define SDL_OBJECT_COUNT_TRACE 0
#endif

#if SDL_OBJECT_COUNT
#define SDL_OBJECT_COUNT_LOG_NAME(type, name) \
  do {                                                                                       \
    if (Util::isSingleThreadProgram())                                                       \
      SDL_DEBUG_ALWAYS(Leak.ObjectCount, #name << " has " << Util::ObjectCount<type>::size() \
                                               << " live objects");                          \
  } while (0)
#define SDL_AND_OBJECT_TRACK_BASE(type) , public sdl::Util::ObjectTrack<type>
#define SDL_OBJECT_TRACK_BASE(type) : public sdl::Util::ObjectTrack< type >
#else
#define SDL_OBJECT_COUNT_LOG_NAME(type, name)
#define SDL_AND_OBJECT_TRACK_BASE(type)
#define SDL_OBJECT_TRACK_BASE(type)
#endif

#define SDL_LOG_OBJECT_COUNT(type) SDL_OBJECT_COUNT_LOG_NAME(type, #type)

namespace sdl {
namespace Util {

template <class Val>
struct ObjectCount {
  static AtomicCount count;
  static char const* name;
  static inline void push() { ++count; }
  static inline void pop() {
    --count;
    assert(size() != (std::size_t) - 1);
  }
  static inline std::size_t size() { return count; }
  static inline void setName(char const* name_) { name = name_; }
};

template <class Val>
char const* ObjectCount<Val>::name("unnamed");

#if !SDL_EXPLICIT_OBJECT_COUNT
template <class Val>
AtomicCount ObjectCount<Val>::count(0);
#endif

/// use as superclass of Val to avoid writing push/pop yourself
template <class Val>
struct ObjectTrack {
  ObjectTrack() {
    ObjectCount<Val>::push();
#if SDL_OBJECT_COUNT_TRACE
    SDL_TRACE(Leak.ObjectCount.push, "++count<" << ObjectCount<Val>::name << ">=" << ObjectCount<Val>::size()
                                                << " construct -> (" << this << ")");
#endif
  }

  ObjectTrack(ObjectTrack const&) {
    ObjectCount<Val>::push();
#if SDL_OBJECT_COUNT_TRACE
    SDL_TRACE(Leak.ObjectCount.push, "++count<" << ObjectCount<Val>::name << ">=" << ObjectCount<Val>::size()
                                                << " copy -> (" << this << ")");
#endif
  }

  /// no new objects created
  inline void operator=(ObjectTrack const&) {}

  ~ObjectTrack() {
    ObjectCount<Val>::pop();
#if SDL_OBJECT_COUNT_TRACE
    SDL_TRACE(Leak.ObjectCount.pop, "--count<" << ObjectCount<Val>::name << ">=" << ObjectCount<Val>::size()
                                               << " destroy(" << this << ")");
#endif
  }
};


}}

#endif
