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

    before C++11, use swap to move-assign.

    struct T {
      typedef void memcpy_movable;
      // this typedef means T can be memcpy-moved (e.g. not std::map or std::vector<non-memcpy-move> or other circular/parent-pointer structures
    };

*/

#ifndef MOVE_JG_2014_06_13_HPP
#define MOVE_JG_2014_06_13_HPP
#pragma once

#include <algorithm>
#include <utility>
#include <memory>
#include <vector>
#include <cstring>
#include <boost/type_traits/is_pod.hpp>
#include <boost/utility/enable_if.hpp>
#include <sdl/Util/VoidIf.hpp>
#include <sdl/Util/Construct.hpp>

namespace sdl { namespace Util {

template <class T>
void adlSwap(T &to, T &from) {
  using namespace std;
  swap(to, from); //TODO: C++11
}

template <class T, class Unless=void>
struct MemcpyMovable {
  enum { value = false };
};

template <class T>
struct MemcpyMovable<T, VoidIf<typename T::memcpy_movable> > {
  typedef void type;
  enum { value = true };
};

template <class T>
struct MemcpyMovable<T, typename boost::enable_if<boost::is_pod<T> >::type> {
  typedef void type;
  enum { value = true };
};

template <class T>
struct MemcpyMovable<std::vector<T>, typename VoidIf<typename MemcpyMovable<T>::type>::type> {
  typedef void type;
  enum { value = true };
};

template <class T>
struct MoveConstruct {
  static inline void moveConstruct(T *to, T &from) {
    new(to) T(std::move(from));
  }
};

/**
   pre: to is uninit storage of sizeof(T) bytes.

   post: to <= from, from holds indeterminate valid content
*/
template <class T>
static inline T* moveConstruct(void *to, T &from) {
  MoveConstruct<T>::moveConstruct((T*)to, from);
  return (T*)to;
}


template <class T>
void moveAssign(T &to, T &from) {
  to = std::move(from);
}

template <class T>
T && moved(T &from) {
  return static_cast<T&&>(from);
}

}}

#endif
