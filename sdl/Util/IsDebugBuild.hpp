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
