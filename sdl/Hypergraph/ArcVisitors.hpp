// Copyright 2014-2015 SDL plc
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

    arc modifiers, used as hg.visitArcs(modifier)

    \author Markus Dreyer
*/

#ifndef HYP__HYPERGRAPH_ARCVISITORS_HPP
#define HYP__HYPERGRAPH_ARCVISITORS_HPP
#pragma once

#include <sdl/Hypergraph/FeatureIdRange.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Exception.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

namespace sdl {
namespace Hypergraph {

/**
   given feature-weight array, set arc's weight's cost: sum {feature-weight[id] * feature-value[id]}

   see e.g. Optimization/FeatureHypergraphPairs
*/
template <class Arc>
struct InsertWeightsVisitor {

  typedef typename Arc::Weight Weight;
  typedef typename Weight::Map FeatMap;
  typedef typename Weight::FloatT FloatT;

  InsertWeightsVisitor(FloatT const* weights, FeatureId nweights) : weights_(weights) {
    SDL_DEBUG_BUILD(nweights_ = nweights);
  }

  ~InsertWeightsVisitor() {}

  void operator()(ArcBase* a) const {
    Arc* arc = (Arc*)a;
    FloatT weightVal = 0.0;
    Weight& w = arc->weight();
    FeatMap const& map = w.features();
    for (typename FeatMap::const_iterator it = map.begin(); it != map.end(); ++it) {
      SDL_DEBUG_BUILD(assert(it->first < nweights_));
      weightVal += weights_[it->first] * it->second;
    }
    w.value_ = weightVal;
  }

  FloatT const* weights_;
  SDL_DEBUG_BUILD(FeatureId nweights_;)
};

/**
   given feature-weight map, set arc's weight's cost: sum {feature-weight[id] * feature-value[id]}

   (using a sparse feature-weights map)

   see e.g. Optimization/OptimizationProcedure
*/
template <class Arc>
struct InsertSparseWeightsVisitor {

  typedef typename Arc::Weight::Map Map;
  typedef typename Arc::Weight::FloatT FloatT;

  InsertSparseWeightsVisitor(Map const& featureWeights) : featWeightsMap_(featureWeights) {}

  void operator()(ArcBase* a) const {
    Arc* arc = (Arc*)a;
    FloatT weightVal(0.0);
    Map const& map = arc->weight_.features();
    for (typename Map::const_iterator i = map.begin(), end = map.end(); i != end; ++i) {
      typename Map::const_iterator weightsIter = featWeightsMap_.find(i->first);
      if (weightsIter != featWeightsMap_.end()) weightVal += weightsIter->second * i->second;
    }
    arc->weight_.value_ = weightVal;
  }

  Map const& featWeightsMap_;
};


}}

#endif
