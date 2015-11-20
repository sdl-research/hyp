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

    isGraphArcImpl,  templated so MutableHypergraph can bypass all the
    virtual calls to inputLabel.
*/

#ifndef IHYPERGRAPHSTATES_IPP___GRAEHL_2015_08_15_IPP
#define IHYPERGRAPHSTATES_IPP___GRAEHL_2015_08_15_IPP
#pragma once

#include <sdl/Hypergraph/ArcBase.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Hypergraph {
namespace detail {

/**
   sets oneLexical to false (else leaves it alone) if a violation is
   detected. caller is presumed to init oneLexical to true first (likewise fsm)
*/
template <class HG>
bool isGraphArcImpl(HG const& hg, ArcBase const& a, bool& fsm, bool& oneLexical) {
  StateIdContainer const& tails = a.tails_;
  TailId N = tails.size();

  if (hg.inputLabelImpl(a.head_).isTerminal()) return (fsm = oneLexical = false);

  if (N == 2) {
    bool const term1 = hg.inputLabelImpl(tails[1]).isTerminal();
    if (hg.inputLabelImpl(tails[0]).isTerminal()) {
      if (term1) oneLexical = false;
      return (fsm = false);
    } else if (!term1) {
      oneLexical = false;  // best assume not since we return early
      return (fsm = false);
    } else
      return true;
  } else {
    fsm = false;
    if (!N) return false;
    bool lex = false;
    bool const firstLabeled = hg.inputLabelImpl(tails[0]).isTerminal();
    TailId i = !firstLabeled;
    for (;; ++i) {
      if (i == N) return true;
      StateId const tail = tails[i];
      Sym const sym = hg.inputLabelImpl(tail);
      if (sym.isLexical())
        break;
      else if (!sym.isSpecialTerminal()) {
        oneLexical = false;  // best assume not since we return early
        return false;
      } else if (hg.elseHasLexicalOutputLabelImpl(tail))
        break;
    }
    for (;;) {
      if (++i == N) return true;
      StateId const tail = tails[i];
      Sym const sym = hg.inputLabelImpl(tail);
      if (sym.isLexical())
        break;
      else if (!sym.isSpecialTerminal()) {
        oneLexical = false;  // best assume not since we return early
        return false;
      } else if (hg.elseHasLexicalOutputLabelImpl(tail))
        break;
    }
    oneLexical = false;
    for (;;) {
      if (++i == N) return true;
      if (!hg.inputLabelImpl(tails[i]).isTerminal()) return false;
    }
  }
  assert(0);  // can't reach
  return true;
}


}}}

#endif
