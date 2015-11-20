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

#ifndef DFSCOLOR_GRAEHL_2015_08_20_HPP
#define DFSCOLOR_GRAEHL_2015_08_20_HPP
#pragma once

#include <graehl/shared/nibble_array.hpp>

namespace sdl {
namespace Util {

typedef unsigned DfsColor;
enum { kFresh, kOpened, kQueuedOrClosed };
typedef graehl::nibble_array<2, DfsColor> DfsColorArray;


}}

#endif
