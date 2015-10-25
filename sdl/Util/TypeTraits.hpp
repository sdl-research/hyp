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

    add c++14 enable_if_t etc aliases
*/

#ifndef TYPETRAITS_GRAEHL_2015_10_16_HPP
#define TYPETRAITS_GRAEHL_2015_10_16_HPP
#pragma once

#include <type_traits>

#if __cplusplus < 201400L
namespace std {
template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;
template <class E>
using underlying_type_t = typename underlying_type<E>::type;
}
#endif

namespace sdl {

/// usage: (see has_hash_value) callable_name<T, int(float, int)>
#define SDL_CALLABLE_MEMBER_NAME(name, member) \
template <typename C, typename F, typename = void> \
struct callable_##name : public std::false_type {}; \
template <typename C, typename R, typename... A> \
struct callable_##name<C, R(A...),  \
    typename std::enable_if< \
        std::is_same<R, void>::value || \
        std::is_convertible<decltype( \
            std::declval<C>().member(std::declval<A>()...) \
        ), R>::value \
    >::type \
> : public std::true_type {};
#define SDL_CALLABLE_MEMBER(name) SDL_CALLABLE_MEMBER_NAME(name, name)

SDL_CALLABLE_MEMBER_NAME(function, operator())


}


#endif
