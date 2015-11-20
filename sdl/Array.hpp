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

    get the address of array begin and end for array-like (vector, string) containers

    in C++11 you can use vec.data()

    in C++11 strings are guaranteed to be arrays (but in practice they always
    were except for some crazy pre-multithread implementations)

    if you -DSECURE_SCL=0 on windows, then you don't pay any performance penalty
*/

#ifndef ARRAY_JG_2014_01_29_HPP
#define ARRAY_JG_2014_01_29_HPP
#pragma once

#include <cstring>
#include <string>
#include <vector>

#ifndef SDL_WIN32_SECURE_SCL_WORKAROUND
#if _WIN32 && (!defined(_SECURE_SCL) || _SECURE_SCL)
#define SDL_WIN32_SECURE_SCL_WORKAROUND 1
#else
#define SDL_WIN32_SECURE_SCL_WORKAROUND 0
#endif
#endif

namespace sdl {


template <class It1, class It2>
bool equalPodArray(It1 begin1, It2 begin2, std::size_t n) {
  return !std::memcmp(&*begin1, &*begin2, n * sizeof(*begin1));
}

template <class C>
typename C::value_type const* arrayBegin(C const& c) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  return c.empty() ? 0 : &*c.begin();
#else
  return &*c.begin();
#endif
}

template <class C>
typename C::value_type const* arrayEnd(C const& c) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  std::size_t const sz = c.size();
  return sz ? &*c.begin() + sz : 0;
#endif
  return &*c.end();
}

template <class C>
typename C::value_type* arrayBegin(C& c) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  return c.empty() ? 0 : &*c.begin();
#else
  return &*c.begin();
#endif
}

template <class C>
typename C::value_type* zeroArrayBegin(C& c) {
  typedef typename C::value_type Val;
  Val* r = arrayBegin(c);
  std::memset(r, 0, sizeof(Val) * c.size());
  return r;
}

template <class C>
typename C::value_type* arrayEnd(C& c) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  std::size_t const sz = c.size();
  return sz ? &*c.begin() + sz : 0;
#endif
  return &*c.end();
}

inline char const* arrayBegin(std::string const& c) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  return (char const*)&*c.begin();
#else
  return &*c.begin();
#endif
}

inline char* arrayBegin(std::string& c) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  return (char*)&*c.begin();
#else
  return &*c.begin();
#endif
}

inline char const* arrayEnd(std::string const& c) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  std::size_t const sz = c.size();
  return (char const*)&*c.begin() + sz;
#else
  return &*c.end();
#endif
}

inline char* arrayEnd(std::string& c) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  std::size_t const sz = c.size();
  return (char*)&*c.begin() + sz;
#else
  return &*c.end();
#endif
}

template <class T, class A>
T const* arrayBegin(std::vector<T, A> const& c) {
  return c.data();
}

template <class T, class A>
T* arrayBegin(std::vector<T, A>& c) {
  return c.data();
}

#if !defined(__GNUC__)
template <class T, class A>
T const* arrayEnd(std::vector<T, A> const& c) {
  return c.data() + c.size();
}

template <class T, class A>
T* arrayEnd(std::vector<T, A>& c) {
  return c.data() + c.size();
}
#endif


}

#endif
