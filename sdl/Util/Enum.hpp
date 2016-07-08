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
/** file

   The SDL_ENUM macro is a mechanism to define an enum, In addition to the enum
   definition this macro also generates a EnumTypeInfo<> class that has emit and
   parse methods to convert an enum instance to/from a string.  The macro also
   generates methods required to use the enum with the configure library.

   (see EnumDetail.hpp for generated interface)
 **/

#ifndef SDL_UTIL_ENUM_HPP_
#define SDL_UTIL_ENUM_HPP_
#pragma once

#include <sdl/Util/EnumDetail.hpp>

#define SDL_STATIC_ENUM_NAMES 0
#if SDL_STATIC_ENUM_NAMES
#define SDL_ENUM_NAME_LINKAGE static
#define SDL_NAME_ENUM(name)
#else
#define SDL_ENUM_NAME_LINKAGE extern
/** now required: you must for each SDL_ENUM(name) put SDL_NAME_ENUM(name) in a single .cpp/lib
    (to cut down on number of redundant std::string enum names global ctor/dtors)
*/
#define SDL_NAME_ENUM(name) SDL_DETAIL_NAME_ENUM(name)
#endif


/// SDL_ENUM(AB, (A, (B, BOOST_PP_NIL))) => Ab a=kA, b=kB;
#define SDL_ENUM_DEF(name, elems)                                                               \
  SDL_DETAIL_ENUM_TYPE_INFO(name, BOOST_PP_LIST_TRANSFORM(SDL_DETAIL_ENUM_PREPEND_k, k, elems), \
                            BOOST_PP_LIST_TRANSFORM(SDL_DETAIL_TOSTRING, _, elems), SDL_ENUM_NAME_LINKAGE)

#if __cplusplus >= 201103L
/// SDL_ENUM(AB, 2, (A, B)) => Ab a=kA, b=kB;
#define SDL_ENUM(name, size, elems) SDL_ENUM_DEF(name, BOOST_PP_TUPLE_TO_LIST(elems))
/// SDL_ENUM(AB, (A, B)) => Ab a=kA, b=kB;
#define SDL_ENUMT(name, elems) SDL_ENUM(name, _, elems)
/// SDL_ENUM(AB, A, B) => Ab a=kA, b=kB;
#define SDL_ENUMS(name, ...) SDL_ENUM(name, _, (__VA_ARGS__))
#else
/// pre-c++11, size arg is required
#define SDL_ENUM(name, size, elems) SDL_ENUM_DEF(name, BOOST_PP_TUPLE_TO_LIST(size, elems))
#endif


#endif
