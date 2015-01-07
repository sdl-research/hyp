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

    for now, just calls PruneNonBest or nothing.
*/

#ifndef PRUNETONBEST_JG_2014_08_11_HPP
#define PRUNETONBEST_JG_2014_08_11_HPP
#pragma once

#include <sdl/Hypergraph/PruneNonBest.hpp>

namespace sdl { namespace Hypergraph {

struct PruneToNbestOptions {
  void validate() {
    if (pruneToNbest>1) {
      SDL_ERROR(Hypergraph.pruneToNbest, "prune-to-nbest " << pruneToNbest << " > 1 is not supported yet; using 1");
      pruneToNbest = 1;
    }
  }
  PruneToNbestOptions(unsigned defaultNbest = 0)
      : pruneToNbest(defaultNbest)
  {}
  template <class Config>
  void configure(Config &config)
  {
    config("prune-to-nbest", &pruneToNbest).self_init()
        ("if >0, prune output so as to preserve just the [prune-to-nbest]-best paths (or you can use the separate PruneToBest module instead). TODO: only 1-best is supported so far.");
  }
  unsigned pruneToNbest;
  friend inline std::ostream& operator<<(std::ostream &out, PruneToNbestOptions const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream &out) const {
    out << "prune-to-nbest=" << pruneToNbest;
  }
};

template <class Arc>
void justNbest(IMutableHypergraph<Arc> &hg, PruneToNbestOptions const& opt) {
  if (opt.pruneToNbest)
    justBest(hg); //TODO: nbest
}


}}

#endif
