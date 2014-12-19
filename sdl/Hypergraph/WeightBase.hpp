/** \file

    Common base class for all weights for clarity of casts. intentionally
    empty/non-virtual dtor

*/

#ifndef HYP__HYPERGRAPH_WEIGHTBASE_HPP
#define HYP__HYPERGRAPH_WEIGHTBASE_HPP
#pragma once

namespace sdl {
namespace Hypergraph {

/**

   Common base class for all weights. intentionally empty/non-virtual dtor

   Allows storing a pointer to weight without templating on the specific weight

   (alternative: use void * instead (no common base class) - but empty base class
   optimization should make them equivalent in practice)
*/

struct WeightBase {};

}}

#endif
