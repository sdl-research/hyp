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

    minimalist enable_if without the boost + integral constants: takes a
    (possibly non-existent type) and if doesn't exist, causes substitution
    failure (SFINAE)

    used to selectively enable fn overloads:

    typename VoidIf<typename T::member_type>::type fn(...);

    or

    typename TypeIf<int, typename T::member_type>::type fn(...) { return 1; }

    FULL EXAMPLE:

    template <class T, class Unless=void> //default case (nested type isn't there
    struct Traits {  };

    template <class T> // partial specialization for when T::nested exists
    struct Traits<T, VoidIf<typename T::nested> > { };

*/

#ifndef SDL_VOIDIF_JG20121030_HPP
#define SDL_VOIDIF_JG20121030_HPP
#pragma once


namespace sdl {
namespace Util {

template <class IfTypeExists>
struct VoidIf {
  typedef void type;
};

/**
   exactly like VoidIf but additional first Type argument specifies alternative
   type to void.
*/
template <class Type, class IfTypeExists>
struct TypeIf {
  typedef Type type;
};


}}

#endif
