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

    strings of 16 and 32 bit chars, and macro for SDL_UNICODE_IS_WCHAR true iff
    wchar is 32 bit
*/

#ifndef SDL_STRING32_JG2012928_HPP
#define SDL_STRING32_JG2012928_HPP
#pragma once


#include <sdl/Types.hpp>
#include <string>
#include <vector>

namespace sdl {
namespace Util {

typedef uint32 UnicodePoint;

// Char32 may need a cast to/from Unicode (even if your platform doesn't seem to need it)

#ifdef WIN32
#define SDL_UNICODE_IS_WCHAR 0
typedef Unicode Char32;
typedef wchar_t Char16;
#else
#define SDL_UNICODE_IS_WCHAR 1
typedef wchar_t Char32;
typedef uint16 Char16;
#endif

typedef std::basic_string<Char32> String32;
typedef std::basic_string<Char16> String16;

typedef std::string Utf8Char;  // a single Unicode in utf8 in a string
typedef std::string Utf8String;


}}

#endif
