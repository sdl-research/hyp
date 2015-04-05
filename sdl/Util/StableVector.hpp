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

    a vector you can keep adding to without invalidating earlier iterators.
*/

#ifndef STABLEVECTOR_JG_2014_03_04_HPP
#define STABLEVECTOR_JG_2014_03_04_HPP
#pragma once

#include <sdl/Util/Valgrind.hpp>
#include <graehl/shared/stable_vector.hpp>
#include <sdl/Sym.hpp>
#include <string>

namespace sdl { namespace Util {

using graehl::stable_vector;

// TODO: experiment to find best performing kLog2StableVectorChunkSize. set now
// for (2^9 * 8 = 4096) (sizeof(std::string) is 8, page is typically 4096)
enum { kLog2StableVectorChunkSize = 9 };

typedef stable_vector<std::string, SymInt, kLog2StableVectorChunkSize> StableStrings;


}}

#endif
