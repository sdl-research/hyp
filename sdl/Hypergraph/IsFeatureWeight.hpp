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
   feature weight concept: W has nested typedef void W::IsFeatureWeight
*/

#ifndef HYP__HYPERGRAPH_ISFEATUREWEIGHT_HPP
#define HYP__HYPERGRAPH_ISFEATUREWEIGHT_HPP
#pragma once

#include <type_traits>

namespace sdl {
namespace Hypergraph {

/**
   Interface for is_feature_weight consistent with std::type_traits libraries.
**/

template <class Weight, class Enable = void>
struct IsFeatureWeight : public std::false_type {};

template <class Weight>
struct IsFeatureWeight<Weight, typename Weight::IsFeatureWeight> : public std::true_type {};

template <class Weight>
bool isFeatureWeight(Weight* p = 0) {
  return IsFeatureWeight<Weight>::value;
}


/**
   Distance weight: if w==Weight(w.getValue()) e.g. Log, Viterbi wt
*/

template <class Weight, class Enable = void>
struct IsDistanceWeight : public std::true_type {};

template <class Weight>
struct IsDistanceWeight<Weight, typename Weight::IsFeatureWeight> : public std::false_type {};

template <class Weight>
bool isDistanceWeight(Weight* p = 0) {
  return IsDistanceWeight<Weight>::value;
}


}}

#endif
