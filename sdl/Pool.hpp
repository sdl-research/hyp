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

    Pool pool(allocsz); pool.free(pool.malloc())
*/

#ifndef POOL_GRAEHL_2015_10_21_HPP
#define POOL_GRAEHL_2015_10_21_HPP
#pragma once

#include <sdl/Pool/pool.hpp>

namespace sdl {

/// Pool pool(allocsz); pool.free(pool.malloc())
typedef Pool::pool<> ChunkPool;



}

#endif
