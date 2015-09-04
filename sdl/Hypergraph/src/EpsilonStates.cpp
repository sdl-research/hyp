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

  mark epsilon-labeled states (where both input and output labels, unless
  NoSymbol, are epsilon).
*/

#include <sdl/Hypergraph/HypergraphBase.hpp>
#include <sdl/Hypergraph/EpsilonStates.hpp>

namespace sdl {
namespace Hypergraph {

EpsilonStates::EpsilonStates(HypergraphBase const& hg) : StateSet(hg.size()) {
  HypergraphBase::MaybeLabels inout(hg.maybeLabels());
  if (inout.first) {
    typedef HypergraphBase::Labels Labels;
    Labels const& in = *inout.first;
    Labels const& out = *inout.second;
    StateId i = 0, nin = (StateId)in.size(), nout = (StateId)out.size(), nmin = std::min(nin, nout);
    for (; i < nmin; ++i)
      if (in[i] == EPSILON::ID && out[i] == EPSILON::ID) StateSet::set(i);
    for (; i < nin; ++i)
      if (in[i] == EPSILON::ID) StateSet::set(i);
    for (; i < nout; ++i)
      // allowed to have epsilon output and NoSymbol input?
      if (out[i] == EPSILON::ID) StateSet::set(i);
  } else
    for (StateId i = 0, n = StateSet::size(); i != n; ++i)
      if (isEpsilonLabelPair(hg.labelPair(i))) StateSet::set(i);
}


}}
