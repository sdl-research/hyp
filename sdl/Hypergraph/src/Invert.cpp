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
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>

namespace sdl {
namespace Hypergraph {

inline bool needsInvert(IMutableHypergraphBase const& h) {
  return !h.hgOutputLabelFollowsInput();
}

inline LabelPair invert(LabelPair x) {
  return LabelPair(x.second, x.first);
}

void invertForceLabelPair(IMutableHypergraphBase &h) {
  for (StateId s = 0, e = h.hgGetNumStates(); s < e; ++s)
    h.setLabelPair(s, invert(h.hgGetLabelPair(s)));
}

void invert(IMutableHypergraphBase &h) {
  if (needsInvert(h))
    invertForceLabelPair(h);
}


}}
