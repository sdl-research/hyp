/** \file

    helpers for IHypergraph.
*/


#ifndef HYP__HYPERGRAPHIMPL_HPP
#define HYP__HYPERGRAPHIMPL_HPP
#pragma once

#include <sdl/Hypergraph/Arc.hpp>

namespace sdl {
namespace Hypergraph {
namespace impl {

struct deleter {
  template <class X> void operator()(X *x) const {
    delete x;
  }
};

struct set_weight1 {
  template <class A> void operator()(A *a) const {
    a->setWeight(A::Weight::one());
  }
};


}}}

#endif
