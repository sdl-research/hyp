/** \file

    accumulate min or max (in a way that's more efficient for expensive types than std::min, std::max).
*/

#ifndef MINMAX_JG201331_HPP
#define MINMAX_JG201331_HPP
#pragma once

namespace sdl { namespace Util {

template <class Optimum>
void maxEq(Optimum &optimum, Optimum const& candidate) {
  if (optimum < candidate)
    optimum = candidate;
}

template <class Optimum>
void minEq(Optimum &optimum, Optimum const& candidate) {
  if (candidate < optimum)
    optimum = candidate;
}

}}

#endif
