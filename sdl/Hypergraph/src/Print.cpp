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
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/SymbolPrint.hpp>
#include <iostream>

namespace sdl {
namespace Hypergraph {

void dumpVHG(IHypergraph<ArcTpl<ViterbiWeightTpl<float> > > const& hg) {
  std::cout << hg;
}

void dumpFHG(IHypergraph<ArcTpl<FeatureWeight> > const& hg) {
  std::cout << hg;
}

void printState(std::ostream& out, StateId s, IHypergraphStates const& hg, bool inlineLabel) {
  out << s;
  IVocabulary const* voc = hg.vocab();
  StateId nstates = hg.size();
  if (inlineLabel || s > nstates) {
    Sym sym;
    sym.id_ = s;
    out << '(';
    writeLabel(out, sym, voc);
    out << ')';
  } else {
    LabelPair io(hg.labelPair(s));
    if (io.first || io.second) {
      out << '(';
      if (io.first) {
        writeLabel(out, io.first, voc);
        if (io.second && io.second != io.first) writeLabel(out << ' ', io.second, voc);
      } else
        writeLabel(out << ' ', io.second, voc);
      // "( out-label)" for no in-label vs "(in-label)"
      out << ')';
    }
  }
}

void printArcTails(std::ostream& out, StateIdContainer const& tails, IHypergraphStates const* hg,
                  bool inlineGraphLabels) {
  bool again = false;
  for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
    if (again) out << ' ';
    printState(out, *i, *hg, inlineGraphLabels && again);
    again = true;
  }
}


}}
