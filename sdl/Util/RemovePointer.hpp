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

    generalization of boost/type_traits/remove_pointer.hpp to cover shared_ptr and intrusive_ptr also.
*/

#ifndef REMOVEPOINTER_JG20121228_HPP
#define REMOVEPOINTER_JG20121228_HPP
#pragma once

#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>

namespace sdl { namespace Util {

template <class Ptr, class Enable = void>
struct RemovePointer : boost::remove_pointer<Ptr> {};

template <class T>
struct RemovePointer<boost::intrusive_ptr<T>, void> { typedef T type; };

template <class T>
struct RemovePointer<shared_ptr<T>, void> { typedef T type; };

template <class T>
struct RemoveConstPointer {
  typedef typename boost::remove_const<typename RemovePointer<T>::type>::type type;
};

}}

#endif
