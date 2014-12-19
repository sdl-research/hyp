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

    invert: swap input/output terminal state labels.
*/

#ifndef HYP__HG_INVERT_HPP
#define HYP__HG_INVERT_HPP
#pragma once

namespace sdl {
namespace Hypergraph {

struct IMutableHypergraphBase;

/// result has explicit (input,input) for (input)-only labels
void invertForceLabelPair(IMutableHypergraphBase &);

/// do nothing if labels are all (input)-only, else invertForceLabelPair (semantically
/// both are equivalent until you later chang einput label)
void invert(IMutableHypergraphBase &);

template <class Arc>
bool needsInvert(IHypergraph<Arc> const& h) {
  return h.hasOutputLabels();
}


}}

#endif
