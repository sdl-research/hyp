/** \file

    helpers for per-input-size performance logging.
*/


#ifndef SDL_HYPERGRAPH_INPUTSIZE_JG2012730_HPP
#define SDL_HYPERGRAPH_INPUTSIZE_JG2012730_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Util/InputSize.hpp>

namespace sdl {

namespace Hypergraph {
template <class A>
Util::InputSizeAmount sizeAmount(IHypergraph<A> const& hg)
{
  return double(hg.size());
}

template <class A>
Util::InputSizeUnits sizeUnits(IHypergraph<A> const&)
{
  return "state";
}
}

template <class Val>
inline Util::InputSizeUnits sizeUnits(Val const* pval)
{
  return Util::sizeUnits(*pval);
}

template <class Val>
inline Util::InputSizeAmount sizeAmount(Val const* pval)
{
  return Util::sizeAmount(*pval);
}

template <class Val>
inline Util::InputSizeUnits sizeUnits(shared_ptr<Val> const& pval)
{
  return Util::sizeUnits(*pval);
}

template <class Val>
inline Util::InputSizeAmount sizeAmount(shared_ptr<Val> const& pval)
{
  return Util::sizeAmount(*pval);
}

}

#endif
