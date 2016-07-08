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

    add c++14 enable_if_t etc aliases (in sdl namespace in case someone jumped
    the gun in std)
*/

#ifndef TYPETRAITS_GRAEHL_2015_10_16_HPP
#define TYPETRAITS_GRAEHL_2015_10_16_HPP
#pragma once

#include <graehl/shared/cpp11.hpp>
#include <iostream>
#include <type_traits>

namespace sdl {

#if GRAEHL_CPP14_TYPETRAITS

using std::aligned_storage_t;
using std::aligned_union_t;
using std::enable_if_t;
using std::conditional_t;
using std::common_type_t;

#define SDL_TYPETRAITS_T(traitname) using std::traitname##_t;

#else
template <std::size_t Len, std::size_t Align = alignof(std::max_align_t)> /*default-alignment*/
using aligned_storage_t = typename std::aligned_storage<Len, Align>::type;

template <std::size_t Len, class... Types>
using aligned_union_t = typename std::aligned_union<Len, Types...>::type;

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <bool b, class T, class F>
using conditional_t = typename std::conditional<b, T, F>::type;

template <class... T>
using common_type_t = typename std::common_type<T...>::type;

#define SDL_TYPETRAITS_T(traitname) \
  template <class E>                \
  using traitname##_t = typename std::traitname<E>::type;

#endif

// const-volatile modifications:
SDL_TYPETRAITS_T(remove_const)
SDL_TYPETRAITS_T(remove_volatile)
SDL_TYPETRAITS_T(remove_cv)
SDL_TYPETRAITS_T(add_const)
SDL_TYPETRAITS_T(add_volatile)
SDL_TYPETRAITS_T(add_cv)

// reference modifications:
SDL_TYPETRAITS_T(remove_reference)
SDL_TYPETRAITS_T(add_lvalue_reference)
SDL_TYPETRAITS_T(add_rvalue_reference)

// sign modifications:
SDL_TYPETRAITS_T(make_signed)
SDL_TYPETRAITS_T(make_unsigned)

// array modifications:
SDL_TYPETRAITS_T(remove_extent)
SDL_TYPETRAITS_T(remove_all_extents)

// pointer modifications:
SDL_TYPETRAITS_T(remove_pointer)
SDL_TYPETRAITS_T(add_pointer)

SDL_TYPETRAITS_T(underlying_type)
SDL_TYPETRAITS_T(result_of)

SDL_TYPETRAITS_T(decay)


/**
   special situation:
   safely making a wrapper WITHOUT a copy ctor:

template<class T>
struct wrapper
{
    T value;
    template<typename U>
    wrapper(U && u)
      : value(std::forward<U>(u)) {}
};

   per http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
*/
template <class A, class B>
using disable_if_same_or_derived = enable_if_t<!std::is_base_of<A, decay_t<B>>::value>;

/// usage: (see has_hash_value) callable_name<T, int(float, int)>
#define SDL_CALLABLE_MEMBER_NAME(name, member)                                                                                    \
  template <class C, class F, class = void>                                                                                       \
  struct callable_##name : public std::false_type {};                                                                             \
  template <class C, class R, class... A>                                                                                         \
  struct callable_##name<C, R(A...),                                                                                              \
                         typename std::enable_if<std::is_same<R, void>::value                                                     \
                                                 || std::is_convertible<decltype(std::declval<C>().member(std::declval<A>()...)), \
                                                                        R>::value>::type>                                         \
      : public std::true_type {};
#define SDL_CALLABLE_MEMBER(name) SDL_CALLABLE_MEMBER_NAME(name, name)

SDL_CALLABLE_MEMBER_NAME(function, operator())

template <class T>
using not_istream = std::enable_if<!std::is_base_of<std::istream, T>::value, bool>;
template <class T>
using not_istream_t = typename not_istream<T>::type;

template <class T>
using not_istream_or_string
    = std::enable_if<!std::is_base_of<std::istream, T>::value && !std::is_convertible<T, std::string>::value, bool>;
template <class T>
using not_istream_or_string_t = typename not_istream_or_string<T>::type;

template <class T>
using not_ostream = std::enable_if<!std::is_base_of<std::ostream, T>::value, bool>;
template <class T>
using not_ostream_t = typename not_ostream<T>::type;

template <class T>
using is_istream = std::enable_if<std::is_base_of<std::istream, T>::value, bool>;
template <class T>
using is_istream_t = typename is_istream<T>::type;

template <class T>
using is_ostream = std::enable_if<std::is_base_of<std::ostream, T>::value, bool>;
template <class T>
using is_ostream_t = typename is_ostream<T>::type;


}

#endif
