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

    much simpler than boost/move/move.hpp.
*/

#ifndef CONSTRUCT_JG_2015_05_05_HPP
#define CONSTRUCT_JG_2015_05_05_HPP
#pragma once

#include <cstdlib>

/// for pre --std=c++11 NONCOPYABLE will leave you in public: access, and
/// SDL_*DEFAULT won't actually define the members (you'd have to do that
/// yourself if you already e.g. defined something else blocking it)

#define SDL_DEFAULT_CTOR(TYPE) TYPE() = default;
#define SDL_NONCOPYABLE(TYPE)   \
  TYPE(TYPE const& o) = delete; \
  TYPE& operator=(TYPE const& o) = delete;
#define SDL_MOVE_DEFAULT(TYPE) \
  TYPE(TYPE&& o) = default;    \
  TYPE& operator=(TYPE&& o) = default;
#define SDL_COPY_DEFAULT(TYPE)   \
  TYPE(TYPE const& o) = default; \
  TYPE& operator=(TYPE const& o) = default;

/// opt for default move ctor/assign if C++11, disable entirely copy (you may still wish to define swap
/// yourself but the default C++11 should be smart enough to use 3 moves to swap (after checking for
/// src==dst))
#define SDL_MOVE_NONCOPYABLE(TYPE) SDL_MOVE_DEFAULT(TYPE) SDL_NONCOPYABLE(TYPE)

/// does nothing unless c++11
#define SDL_MOVE_COPY_DEFAULT(TYPE) SDL_MOVE_DEFAULT(TYPE) SDL_COPY_DEFAULT(TYPE)

/// you shouldn't delete the move ctor but you can remove it by providing copy without mentioning move:
#define SDL_COPY_DEFAULT_NO_MOVE(TYPE) SDL_COPY_DEFAULT(TYPE)

#endif
