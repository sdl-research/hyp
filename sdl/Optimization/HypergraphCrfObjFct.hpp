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
#ifndef SDL_HYPERGRAPH_CRFOBJECTIVEFUNCTION_HPP
#define SDL_HYPERGRAPH_CRFOBJECTIVEFUNCTION_HPP
#pragma once

#include <cassert>
#include <list>
#include <utility>
#include <cmath>
#include <sdl/SharedPtr.hpp>

#include <sdl/Optimization/ObjectiveFunction.hpp>
#include <sdl/Optimization/FeatureHypergraphPairs.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/FeatureExpectations.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Constants.hpp>
#include <sdl/Util/Flag.hpp>

namespace sdl {
namespace Optimization {

/**
   \tparam ArcT The arc type of the hypergraphs used (usually some kind of feature weight arc).
 */
template <class ArcT>
class HypergraphCrfObjFct : public Optimization::DataObjectiveFunction<typename ArcT::Weight::FloatT> {
 public:
  typedef Optimization::DataObjectiveFunction<typename ArcT::Weight::FloatT> Base;
  typedef ArcT Arc;
  typedef typename Arc::Weight Weight;
  typedef typename Weight::Map Map;
  typedef typename Weight::FloatT FloatT;

  typedef IFeatureHypergraphPairs<Arc> Pairs;
  typedef typename Pairs::IHgPtr IHgPtr;

  HypergraphCrfObjFct(shared_ptr<Pairs> const& pHgPairs) : Base(), pHgTrainingPairs_(pHgPairs) {}

  std::size_t getNumExamples() OVERRIDE { return pHgTrainingPairs_->size(); }

  /**
      Inserts feature weights into all training examples
   */
  void setFeatureWeights(FloatT const* params, FeatureId numParams) OVERRIDE {
    pHgTrainingPairs_->setFeatureWeights(params, numParams);
  }

  /**
      Inserts feature weights into training examples [begin,
      end).
   */
  void setFeatureWeights(TrainingDataIndex begin, TrainingDataIndex end, FloatT const* params,
                         FeatureId numParams) OVERRIDE {
    pHgTrainingPairs_->setFeatureWeights(begin, end, params, numParams);
  }

  void accept(IObjectiveFunctionVisitor<FloatT>* visitor) OVERRIDE { visitor->visit(this); }

  FloatT getUpdates(TrainingDataIndex begin, TrainingDataIndex end, IUpdate<FloatT>& updates) OVERRIDE {
    using namespace Hypergraph;

    // The amount that the specified examples contribute to the
    // function value:
    FloatT fctValDelta = 0.0;

    // Loop over all examples
    for (TrainingDataIndex i = begin; i < end; ++i) {

     bool const logFirst = i == 0 && logFirstHgOnce_.first();
      // The hypergraph constrained to observed input and output
      IHgPtr pHgConstrained((*pHgTrainingPairs_)[i].first);
      if (logFirst)
        SDL_DEBUG_ALWAYS(Optimize.first.clamped, "Clamped first hg:\n" << *pHgConstrained);
      else
        SDL_TRACE(Optimize.clamped, "Clamped:\n" << *pHgConstrained);
      Map featExpectationsConstrained;
      FloatT pathSumConstrained = computeFeatureExpectations(*pHgConstrained, &featExpectationsConstrained);

      // The "unconstrained" hypergraph, which is *not* constrained to
      // the observed output (i.e., distribution over possible outputs
      // given the input)
      IHgPtr pHgUnconstrained((*pHgTrainingPairs_)[i].second);
      if (logFirst)
        SDL_DEBUG_ALWAYS(Optimize.first.unclamped, "Unclamped first hg:\n" << *pHgUnconstrained);
      else
        SDL_TRACE(Optimize.unclamped, "Unclamped:\n" << *pHgUnconstrained);
      Map featExpectationsUnconstrained;
      FloatT pathSumUnconstrained
          = computeFeatureExpectations(*pHgUnconstrained, &featExpectationsUnconstrained);

      // The gradients are the constrained minus the unconstrained
      // feature expectations:
      for (typename Map::const_iterator it = featExpectationsUnconstrained.begin();
           it != featExpectationsUnconstrained.end(); ++it) {
        // OK because the constrained feat expectations are guaranteed
        // to be a subset of the unconstrained feat expectations:
        updates.update(it->first, featExpectationsConstrained[it->first] - it->second);
      }

      SDL_TRACE(Optimization, "observed: " << pathSumConstrained << ", unobserved: " << pathSumUnconstrained);
      fctValDelta += (pathSumConstrained - pathSumUnconstrained);
    }

    return fctValDelta;
  }

 private:
  Util::Flag logFirstHgOnce_;
  shared_ptr<Pairs> pHgTrainingPairs_;  /// Training data
};


}}

#endif
