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

template <class A>
void difference(IHypergraph<A> const& a, IHypergraph<A> const& b, IMutableHypergraph<A> *r,
                OnMissingProperties onMissing = kModifyOrCopyEnsuringProperties
                )
{
  ASSERT_VALID_HG(a);
  ASSERT_VALID_HG(b);
  r->offerVocabulary(a);
  r->offerVocabulary(b);
  if (a.getVocabulary() != b.getVocabulary()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "Difference: hypergraphs must have same vocabulary");
  }

  if (empty(a)) {
    r->setEmpty();
    return;
  }

  if (!b.isFsm())
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "difference(a, b) is impossible for general CFG b - convert b to Fsm first");
  MutableHypergraph<A> c(kStoreOutArcs);
  fs::complement(b, &c);
  sortArcs(&c);
  compose(a, c, r, ComposeOptions(), onMissing);
  ASSERT_VALID_HG(*r);
}


}}

#endif
