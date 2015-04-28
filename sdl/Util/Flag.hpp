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

    boolean that default constructs always to false. otherwise it's too easy to
    add a bool but forget to 0-init in all constructors.

    flag.first() returns true the first time only
*/

#ifndef FLAG_JG_2014_12_18_HPP
#define FLAG_JG_2014_12_18_HPP
#pragma once


#include <sdl/graehl/shared/flag.hpp>

namespace sdl {
namespace Util {

typedef graehl::flag Flag;  // default-init to false, otherwise like bool
typedef Flag DefaultFalse;
typedef graehl::default_true DefaultTrue;  // default-init to true, otherwise like bool
typedef graehl::counter<std::size_t> Counter;
typedef graehl::counter<graehl::atomic_count> CounterAtomic;


}}

#endif
