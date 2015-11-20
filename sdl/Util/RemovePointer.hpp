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

    generalization of remove_pointer.hpp (which is for basic pointer types) to
    cover misc smart ptrs too
*/

#ifndef REMOVEPOINTER_JG20121228_HPP
#define REMOVEPOINTER_JG20121228_HPP
#pragma once

#include <type_traits>
#include <sdl/SharedPtr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>

namespace sdl {
namespace Util {

template <class Ptr, class Enable = void>
struct RemovePointer : std::remove_pointer<Ptr> {};

template <class T>
struct RemovePointer<boost::intrusive_ptr<T>, void> {
  typedef T type;
};

template <class T>
struct RemovePointer<boost::shared_ptr<T>, void> {
  typedef T type;
};

template <class T>
struct RemovePointer<std::shared_ptr<T>, void> {
  typedef T type;
};

template <class T>
struct RemoveConstPointer {
  typedef typename std::remove_const<typename RemovePointer<T>::type>::type type;
};


}}

#endif
