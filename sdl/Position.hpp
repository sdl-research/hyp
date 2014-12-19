/** \file

    string position (in unicode codepoints) for xmt API. 32-bit to save space.
*/

#ifndef POSITION_JG_2014_07_18_HPP
#define POSITION_JG_2014_07_18_HPP
#pragma once

namespace sdl {

/// for strings, Position is the 32-bit Unicode char index (not an index into
/// 16-bit-char wstrings, but rather an index into the Unicodes)
typedef unsigned Position;

namespace {
Position const kNullPosition((Position)-1);
}


}

#endif
