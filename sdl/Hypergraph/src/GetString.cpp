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
#include <sdl/Hypergraph/GetString.hpp>

namespace sdl {
namespace Hypergraph {

Syms& symsFromStatesAppend(Syms& result, StateString const& ss, HypergraphBase const& hg,
                                  WhichSymbolOptions const& opt) {
  for (StateString::const_iterator i = ss.begin(), e = ss.end(); i != e; ++i) {
    Sym sym = hg.label(*i, opt.labelType);
    if (sym && opt.shows(sym)) result.push_back(sym);
  }
  return result;
}

namespace {
struct AppendSymsFromStates {
  Syms& result;
  HypergraphBase const& hg;
  WhichSymbolOptions const& opt;
  AppendSymsFromStates(Syms& result, HypergraphBase const& hg, WhichSymbolOptions const& opt)
      : result(result), hg(hg), opt(opt) {}
  void push_back(StateId s) {
    Sym sym(hg.label(s, opt.labelType));
    if (sym && opt.shows(sym)) result.push_back(sym);
  }
};
}

Syms& symsFromDerivAppend(Syms& syms, DerivationPtr const& pDerivation, HypergraphBase const& hg,
                                 WhichSymbolOptions const& opt) {
  if (pDerivation) {
    AppendSymsFromStates append(syms, hg, opt);
    pDerivation->appendStates(append, hg.final(), opt.leafOnly);
  }
  return syms;
}

}}
