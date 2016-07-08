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
#include <sdl/Hypergraph/SimpleSentence.hpp>

namespace sdl {
namespace Hypergraph {

struct AppendLexicalVisitor {
  HypergraphBase const& hg;
  Syms& syms;
  AppendLexicalVisitor(HypergraphBase const& hg, Syms& syms) : hg(hg), syms(syms) {}
  void operator()(ArcBase const* arc) const {
    Sym s = hg.firstLexicalInput(arc);
    if (s) syms.push_back(s);
  }
  void alternative(ArcBase const*) const {}
};

bool getSimpleSentenceWords(HypergraphBase const& hg, Syms& syms) {
  return visitSimpleSentenceArcsLeftToRight(hg, AppendLexicalVisitor(hg, syms));
}

/// get arcs in left->right (start->final) order (whether in arcs or out arcs)
bool getSimpleSentenceArcs(std::vector<ArcBase*>& arcs, HypergraphBase const& hg) {
  arcs.reserve(hg.sizeForHeads());
  bool ok = visitSimpleSentenceArcs(hg, AppendArcBase(arcs));
  if (!ok) return false;
  if (!leftToRightSimpleSentenceVisit(hg)) std::reverse(arcs.begin(), arcs.end());
  return true;
}

/**
   \return length of the simple graph derivation in #arcs, else kNoState.
*/
StateId simpleSentenceLength(StateId& neps, HypergraphBase const& hg, bool allowSausage) {
  CountStatesVisitor v(hg);
  if (!visitSimpleSentenceArcs(hg, v, allowSausage)) return kNoState;
  neps = v.neps;
  return v.n;
}

/// \return 'hg is a graph with only a single path, or state sequence for all paths if allowSausage'
bool isSimpleSentence(HypergraphBase const& hg, bool allowSausage) {
  return visitSimpleSentenceArcs(hg, IgnoreArcVisitor(), allowSausage);
}


}}
