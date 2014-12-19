/** \file

    choose map and float type for feature values riding on hg arcs.
*/

#ifndef HYP__HYPERGRAPH_FEATUREWEIGHT_HPP
#define HYP__HYPERGRAPH_FEATUREWEIGHT_HPP
#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <sdl/Hypergraph/FeatureWeightTpl.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Hypergraph {

typedef FeatureWeightTpl<FeatureValue, Features, TakeMin> FeatureWeight;

/**
   Could also use these, for example:

   typedef FeatureWeightTpl<float,
                         std::map<std::size_t, bool>
                         > FeatureWeight;

   typedef FeatureWeightTpl<float,
                         Features
                         > FeatureWeightStr;

   If you use such additional types, don't forget to define a custom
   parseWeightString function in the .cpp file.
 */

}}

#endif
