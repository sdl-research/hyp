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
