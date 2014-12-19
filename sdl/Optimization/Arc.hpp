#ifndef SDL_OPTIMIZATION_ARC_HPP
#define SDL_OPTIMIZATION_ARC_HPP
#pragma once

#include <map>
#include <lbfgs.h>

#include <sdl/Hypergraph/FeatureWeightTpl.hpp>
#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Optimization {

// When optimizing with L-BFGS, we *must* use the lbfgsfloatval_t
// data type (which, by default, is double, but can be changed). For
// simplicity, we also use this type even if the user requests
// online optimization instead of L-BFGS.
typedef lbfgsfloatval_t FloatT;

typedef Hypergraph::FeatureWeightTpl<FloatT,
                                     std::map<Hypergraph::FeatureId, FloatT> > Weight;

typedef Hypergraph::ArcTpl<Weight> Arc;

}}

#endif
