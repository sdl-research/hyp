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

    Common base class for all weights for clarity of casts. intentionally
    empty/non-virtual dtor

    TODO: move float member here? few weights don't have one
*/

#ifndef HYP__HYPERGRAPH_WEIGHTBASE_HPP
#define HYP__HYPERGRAPH_WEIGHTBASE_HPP
#pragma once

namespace sdl {
namespace Hypergraph {

/**

   Common base class for all weights. intentionally empty/non-virtual dtor

   Allows storing a pointer to weight without templating on the specific weight

   (alternative: use void * instead (no common base class) - but empty base class
   optimization should make them equivalent in practice)
*/

struct WeightBase {};


}}

#endif
