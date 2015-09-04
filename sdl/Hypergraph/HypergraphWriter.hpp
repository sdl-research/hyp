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

    print a hypergraph.
*/

#ifndef HYP__HYPERGRAPH_HYPERGRAPHWRITER_HPP
#define HYP__HYPERGRAPH_HYPERGRAPHWRITER_HPP
#pragma once

#include <sdl/Hypergraph/HypergraphBase.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Util/Print.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>

namespace sdl {
namespace Hypergraph {

template <class Arc>
std::ostream& writeHypergraph(std::ostream& out, IHypergraph<Arc> const& hg, bool fullEmptyCheck = false) {
  if (fullEmptyCheck ? empty(hg) : hg.prunedEmpty()) return out;
  out << hg;
  return out;
}

struct WithProperties {
  HypergraphBase const& hg;
  WithProperties(HypergraphBase const& hg) : hg(hg) {}
  friend inline std::ostream& operator<<(std::ostream& out, WithProperties const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { out << PrintProperties(hg.properties()) << '\n' << hg; }
};

inline WithProperties withProperties(HypergraphBase const& hg) {
  return WithProperties(hg);
}


}}

#endif
