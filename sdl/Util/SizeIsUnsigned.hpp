// Copyright 2014 SDL plc
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

    compile configuration: if you want to provide overloads for size_t and
    unsigned, but also want to support 32-bit.
*/


#ifndef LWUTIL_SIZE_IS_UNSIGNED_HPP
#define LWUTIL_SIZE_IS_UNSIGNED_HPP
#pragma once

#if defined(SDL_32)
# define SDL_SIZE_IS_UINT32 1
#endif

#ifndef SDL_SIZE_IS_UINT32
#if defined(_WIN32) && !defined(_WIN64)
# define SDL_SIZE_IS_UINT32 1
#else
# define SDL_SIZE_IS_UINT32 0
#endif
#endif

namespace sdl {
namespace Util {

#if SDL_SIZE_IS_UINT32
struct UnsignedNotSize
{
  unsigned i;
  UnsignedNotSize(unsigned i) : i(i) {}
  operator unsigned&() { return i; }
  operator unsigned() const { return i; }
};
#else
typedef unsigned UnsignedNotSize;
#endif

}}

#endif
