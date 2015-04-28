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

    accumulate min or max (in a way that's more efficient for expensive types than std::min, std::max).
*/

#ifndef MINMAX_JG201331_HPP
#define MINMAX_JG201331_HPP
#pragma once

namespace sdl { namespace Util {

template <class Optimum>
void maxEq(Optimum &optimum, Optimum const& candidate) {
  if (optimum < candidate)
    optimum = candidate;
}

template <class Optimum>
void minEq(Optimum &optimum, Optimum const& candidate) {
  if (candidate < optimum)
    optimum = candidate;
}

}}

#endif
