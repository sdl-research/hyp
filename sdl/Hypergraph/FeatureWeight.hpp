// Copyright 2014-2015 SDL plc
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

    choose map and float type for feature values riding on hg arcs.
*/

#ifndef HYP__HYPERGRAPH_FEATUREWEIGHT_HPP
#define HYP__HYPERGRAPH_FEATUREWEIGHT_HPP
#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <sdl/Hypergraph/FeatureWeightTpl.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Hypergraph {

typedef FeatureWeightTpl<FeatureValue, Features, TakeMin> FeatureWeight;

/**
   Could also use these, for example:

   typedef FeatureWeightTpl<float,
                         std::map<std::size_t, bool>
                         > FeatureWeight;

   typedef FeatureWeightTpl<float,
                         Features
                         > FeatureWeightStr;

   If you use such additional types, don't forget to define a custom
   parseWeightString function in the .cpp file.
 */

}}

#endif
