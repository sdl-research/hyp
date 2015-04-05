// Copyright 2014-2015 SDL plc
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

/* \file

   equality check

   no rho, sigma, phi allowed in first position in compose

   therefore, implement fs-only Compose that supports them? note: openfst doesn't allow them in first position either.
*/

#ifndef HYP__HYPERGRAPH__EQUAL_HPP
#define HYP__HYPERGRAPH__EQUAL_HPP
#pragma once

#include <sdl/Hypergraph/Determinize.hpp>
#include <sdl/Hypergraph/Difference.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>

namespace sdl {
namespace Hypergraph {
namespace fs {

template <class A>
bool equal(IHypergraph<A> const& a, IHypergraph<A> const& b) {
  ASSERT_VALID_HG(a);ASSERT_VALID_HG(b);
  if (&a==&b) return true;
  if (a.prunedEmpty())
    return empty(b);
  if (b.prunedEmpty())
    return empty(a);
  if (!a.isFsm() || !b.isFsm()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "equal(a, b) is undecidable for general CFG a and b - convert to Fsm first");
  }
  if (a.getVocabulary() != b.getVocabulary()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "Difference: hypergraphs must have same vocabulary");
  }
  //  return &a==&b; //FIXME

  typedef IHypergraph<A> H;
  typedef shared_ptr<H const> HP;

  // will need both indexes for compose-first-position
  //  HP pa=ensureProperties(a, kStoreInArcs|kStoreOutArcs);
  //  HP pb=ensureProperties(b, kStoreInArcs|kStoreOutArcs);

  Properties p=kStoreOutArcs; // |kStoreInArcs

#define HG_EQUAL_CHECK_DIFF(a, b, msgv, msgempty) do {                     \
    MutableHypergraph<A> d(p);                                          \
    difference(a, b, &d);                                                 \
    if (!empty(d)) {                                                    \
      return false;                                                     \
    }                                                                   \
  } while(0)


  HG_EQUAL_CHECK_DIFF(a, b,5, "test_fsm_equal empty a-b (may be equal!)");
  HG_EQUAL_CHECK_DIFF(b, a,4, "test_fsm_equal a-b == b-a == EMPTY, so a==b.\n");
#undef HG_EQUAL_CHECK_DIFF
  return true;
}

}}}

#endif
