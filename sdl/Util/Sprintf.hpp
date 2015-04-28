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

    MSVC lacks C99/posix/C++11 snprintf. this provides it (as snprintf in global
    namespace).

    also,

    SprintfStr str("%d %f %s, 1, 1.f, "cstring")

    is the same as

    std::string str("1 1.0 cstring")

    if you need only a cstr, try

    SprintfCstr<MAX_EXPECTED_CHARS_PLUS_1> buf(...); // works even if your estimate is too low

    buf.c_str(); //only valid as long as buf object exists

    also provides C99 compliant snprintf, C99snprintf (a synonym), and C99vsnprintf (varargs version)
*/

#ifndef SDL_SPRINTF_JG_2013_05_21_HPP
#define SDL_SPRINTF_JG_2013_05_21_HPP
#pragma once

#ifdef SDL_MUTABLE_STRING_DATA
# define GRAEHL_MUTABLE_STRING_DATA SDL_MUTABLE_STRING_DATA
#endif

#include <sdl/graehl/shared/snprintf.hpp>

namespace sdl { namespace Util {


/**
   usage: Sprintf<>("%d %f %s, 1, 1.f, "cstring").str() gives string("1 1.0
   cstring"). cstr (or char * implicit) is only valid as long as Sprintf object
   exists. implicitly converts to std::string

   default buflen=52 makes sizeof(Sprintf<>) == 64 (w/ 64-bit ptrs)
   - note that we'll still succeed even if 52 is too small (by heap alloc)
*/
using graehl::SprintfCstr;
using graehl::Sprintf;


/**
   if you're going to SprintfCstr<>(...).str(), this would be faster as it avoids
   a copy. this implicitly converts to (is a) std::string
*/
typedef graehl::SprintfStr SprintfStr;

}}

#endif
