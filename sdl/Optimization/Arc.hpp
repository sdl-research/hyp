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
#ifndef SDL_OPTIMIZATION_ARC_HPP
#define SDL_OPTIMIZATION_ARC_HPP
#pragma once

#include <map>
#include <lbfgs.h>

#include <sdl/Hypergraph/FeatureWeightTpl.hpp>
#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Optimization {

// When optimizing with L-BFGS, we *must* use the lbfgsfloatval_t
// data type (which, by default, is double, but can be changed). For
// simplicity, we also use this type even if the user requests
// online optimization instead of L-BFGS.
typedef lbfgsfloatval_t FloatT;

typedef Hypergraph::FeatureWeightTpl<FloatT, std::map<Hypergraph::FeatureId, FloatT>> Weight;

typedef Hypergraph::ArcTpl<Weight> Arc;


}}

#endif
