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

    'short string optimization' for vectors

    template <class T, unsigned kMaxInlineSize=3, class Size=unsigned> struct small_vector;

    like vector<T> but stores up to kMaxInlineSize inline (without using
    another heap allocation). iterators invalidate the same as std::vector
    except that erasing elements may also invalidate (as size falls back to
    <=kMaxInlineSize)

    requires T can be initialized by memcpy, e.g. plain old data.
*/

#ifndef SDL_SMALLVECTOR_JG2012123_HPP
#define SDL_SMALLVECTOR_JG2012123_HPP
#pragma once

// necessary for BDBBinaryArchive serialization compatability with std::vector:
#define GRAEHL_SMALL_VECTOR_SERIALIZE_VERSION 0

#include <sdl/Util/Valgrind.hpp>
#include <sdl/graehl/shared/small_vector.hpp>

namespace sdl {

enum { kSmallVectorInlinePointers = 2 };

namespace Util {

using graehl::small_vector;


}}

#endif
