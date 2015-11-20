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

    helpers for per-input-size performance logging.
*/


#ifndef SDL_HYPERGRAPH_INPUTSIZE_JG2012730_HPP
#define SDL_HYPERGRAPH_INPUTSIZE_JG2012730_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Util/InputSize.hpp>

namespace sdl {

namespace Hypergraph {
template <class A>
InputSizeAmount sizeAmount(IHypergraph<A> const& hg) {
  return double(hg.size());
}

template <class A>
std::string sizeUnits(IHypergraph<A> const&) {
  return "state";
}
}


}

#endif
