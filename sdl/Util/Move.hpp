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

    before C++11, use swap to move-assign.

*/

#ifndef MOVE_JG_2014_06_13_HPP
#define MOVE_JG_2014_06_13_HPP
#pragma once

#include <algorithm>
#include <utility>

namespace sdl { namespace Util {

template <class T>
void adlSwap(T &to, T &from) {
  using namespace std;
  swap(to, from); //TODO: C++11
}

template <class T>
void moveAssign(T &to, T &from) {
#if __cplusplus >= 201103L
  to = std::move(from);
#else
  adlSwap(to, from);
#endif
}


}}

#endif
