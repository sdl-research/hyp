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
