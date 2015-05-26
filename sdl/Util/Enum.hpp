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
   generates methods required to use the enum with the configure library.  e.g.
   SDL_ENUM(MyEnum, 2, (Test1, Test2))

   (see EnumDetail.hpp for generaetd interface)
 **/

#ifndef SDL_UTIL_ENUM_HPP_
#define SDL_UTIL_ENUM_HPP_
#pragma once

#include <sdl/Util/EnumDetail.hpp>

/**
   e.g. c++11 doesn't need size: SDL_ENUM(name, BOOST_PP_TUPLE_TO_LIST(elems)
**/
#define SDL_ENUM_DEF(name, elems)                                                               \
  SDL_DETAIL_ENUM_TYPE_INFO(name, BOOST_PP_LIST_TRANSFORM(SDL_DETAIL_ENUM_PREPEND_k, k, elems), \
                            BOOST_PP_LIST_TRANSFORM(SDL_DETAIL_TOSTRING, _, elems))

#define SDL_ENUM(name, size, elems) SDL_ENUM_DEF(name, BOOST_PP_TUPLE_TO_LIST(size, elems))


#endif
