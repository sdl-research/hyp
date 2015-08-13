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

 .
*/

#ifndef ASSERT_GRAEHL_2015_08_09_HPP
#define ASSERT_GRAEHL_2015_08_09_HPP
#pragma once

#include <sdl/Util/LogHelper.hpp>

// Bug: ASSERT_VALID_HG fails on FSMs that do not store a vocab (b/c
// it tests for lexical states). A vocab should not be required in
// general.
#ifdef NDEBUG
#define ASSERT_VALID_HG(hg)
#else
#define ASSERT_VALID_HG(hg)                                                                  \
  do {                                                                                       \
    if (!(hg).checkValid()) {                                                                \
      SDL_THROW_LOG(Hypergraph, InvalidInputException, "ERROR: checkValid failed for " #hg); \
    }                                                                                        \
  } while (0)
#endif

#endif
