// Copyright 2014 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef HYP__HYPERGRAPH_CONVERTWEIGHT_HPP
#define HYP__HYPERGRAPH_CONVERTWEIGHT_HPP
#pragma once

namespace sdl {
namespace Hypergraph {

/**
   This can be specialized for more complicated weights, e.g.,
   FeatureWeight.
 */
template<class FromWeight, class ToWeight>
ToWeight convertWeight(FromWeight const& fromWeight) {
  return ToWeight(fromWeight.getValue());
}

/**
   This can be specialized for other weights, see BlockWeight,
   for example.
 */
template<class FromW, class ToW>
struct WeightConverter {
  typedef ToW Weight;
  WeightConverter(FromW const& from, ToW& to) {
    to = ToW(from.getValue());
  }
};

}}

#endif
