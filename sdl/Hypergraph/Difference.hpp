// Copyright 2014 SDL plc
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

    set difference.
*/

#ifndef HYP__HYPERGRAPH__DIFFERENCE_HPP
#define HYP__HYPERGRAPH__DIFFERENCE_HPP
#pragma once

#include <sdl/Hypergraph/SortArcs.hpp>
#include <sdl/Hypergraph/Compose.hpp>
#include <sdl/Hypergraph/fs/Complement.hpp>
#include <sdl/Hypergraph/Empty.hpp>

namespace sdl {
namespace Hypergraph {

/// r <- a-b  where b must be fsm (so we can complement then intersect)
template <class A>
void difference(IHypergraph<A> const& a, IHypergraph<A> const& b, IMutableHypergraph<A>* r,
                OnMissingProperties onMissing = kModifyOrCopyEnsuringProperties) {
  ASSERT_VALID_HG(a);
  ASSERT_VALID_HG(b);
  r->offerVocabulary(a);
  r->offerVocabulary(b);
  if (a.getVocabulary() != b.getVocabulary())
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "Difference: hypergraphs must have same vocabulary");

  if (empty(a)) {
    r->setEmpty();
  } else if (!b.isFsm())
    SDL_THROW_LOG(Hypergraph, InvalidInputException,
                  "difference(a, b) is impossible for general CFG b - convert b to Fsm first");
  else {
    MutableHypergraph<A> c(kFsmOutProperties);
    fs::complement(b, &c);
    Hypergraph::compose(a, c, r, ComposeOptions(), onMissing);
    ASSERT_VALID_HG(*r);
  }
}


}}

#endif
