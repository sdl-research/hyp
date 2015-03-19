/** \file

    replace the really obnoxious #ifndef NDEBUG convention

    (NDEBUG means NoDebug).
*/
#ifndef SDL_UTIL_ISDEBUGBUILD_HPP
#define SDL_UTIL_ISDEBUGBUILD_HPP
#pragma once


namespace sdl {
namespace Util {

/**
   Quick test if code is compiled as debug code or not.

   Calls to this functions will be optimized out in non-debug mode, so
   it's safe to call this in code that is frequently executed.
*/
inline bool isDebugBuild() {
#ifndef NDEBUG
  return true;
#else
  return false;
#endif
}

}}


/**
   but if you add a member to an object only in debug build, checking
   isDebugBuild won't suffice. use SDL_DEBUG_BUILD(expr) as long as your
   expression has no commas - otherwise use '#if SDL_IS_DEBUG_BUILD'.
*/
#ifdef NDEBUG
# define SDL_DEBUG_BUILD(x)
# define SDL_IS_DEBUG_BUILD 0
#else
# define SDL_DEBUG_BUILD(x) x
# define SDL_IS_DEBUG_BUILD 1
#endif

#endif
