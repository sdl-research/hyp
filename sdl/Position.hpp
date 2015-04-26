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
Position const kMaxPosition((Position)-1);
}


}

#endif
