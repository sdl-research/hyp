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
#ifndef SDL_OPTIMIZATION_ONLINEOPTIMIZER_HPP
#define SDL_OPTIMIZATION_ONLINEOPTIMIZER_HPP
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <sdl/SharedPtr.hpp>
#include <boost/scoped_ptr.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Math.hpp>
#include <sdl/Optimization/ObjectiveFunction.hpp>
#include <sdl/Optimization/LearningRate.hpp>

namespace sdl {
namespace Optimization {

struct OnlineOptimizerOptions {

  template <class Config>
  void configure(Config& config) {
    config("Options for the online optimizer");
    config.is("OnlineOptimizer");
    config("num-epochs", &numEpochs)("Number of epochs (i.e., runs over the training data)").init(10);
    config("learning-rate", &learningRateOptions)("Options for the learning rate");
  }

  std::size_t numEpochs;
  LearningRateOptions learningRateOptions;
};

template<class FloatT>
class ParameterUpdate : public IUpdate<FloatT> {
 public:
  ParameterUpdate(FloatT *params, FeatureId numParams)
      : params_(params)
      , rate_(1.0)
#if SDL_IS_DEBUG_BUILD
      , numParams_(numParams)
#endif
  {}

  void update(FeatureId index, FloatT value) OVERRIDE {
    SDL_DEBUG_BUILD(assert(index < numParams_));
    params_[index] -= rate_ * value;
  }

  virtual void setRate(FloatT rate) {
    rate_ = rate;
  }

  virtual void incTimeStep() {}

 protected:
  FloatT *params_;
  FloatT rate_;
  SDL_DEBUG_BUILD(FeatureId const numParams_);
};

template<class FloatT>
struct AdagradParameterUpdate : public ParameterUpdate<FloatT> {
  AdagradParameterUpdate(FloatT *params, FeatureId numParams, FloatT eta)
      : ParameterUpdate<FloatT>(params, numParams)
      , eta_(eta)
      , prevGrads_(numParams, (FloatT)0)
  {
    SDL_INFO(OnlineOptimizer, "Adagrad eta: " << eta_);
  }

  /// No-op since Adagrad sets its own feature-specific learning rates
  void setRate(FloatT rate) OVERRIDE {}

  void update(FeatureId index, FloatT value) OVERRIDE {
    if (!value) return;
    assert(index < this->numParams_);
    prevGrads_[index] += value * value;
    FloatT rate = eta_ / std::sqrt(prevGrads_[index]);
    this->params_[index] -= rate * value;
  }

  FloatT eta_;
  Util::AutoDeleteArray<FloatT> prevGrads_;
};

/*
  Adagrad with L1 regularization is implemented after Chris Dyer's
  notes, http://www.ark.cs.cmu.edu/cdyer/adagrad.pdf.
 */
template<class FloatT>
struct AdagradL1ParameterUpdate : public ParameterUpdate<FloatT> {
  AdagradL1ParameterUpdate(FloatT *params, FeatureId numParams
                           , FloatT eta
                           , FloatT l1Strength)
      : ParameterUpdate<FloatT>(params, numParams)
      , eta_(eta), l1Strength_(l1Strength)
      , timeStep_(1)
      , prevGrads_(numParams, (FloatT)0)
      , prevGradsSquared_(numParams, (FloatT)0)
  {
    SDL_INFO(OnlineOptimizer, "Adagrad L1 strength: " << l1Strength_ << ", eta: " << eta_);
  }

  /// No-op since Adagrad sets its own feature-specific learning rates
  void setRate(FloatT rate) OVERRIDE {}

  void incTimeStep() OVERRIDE {
    ++timeStep_;
  }

  void update(FeatureId index, FloatT value) OVERRIDE {
    if (!value) return;
    prevGrads_[index] += value;
    prevGradsSquared_[index] += value * value;
    FloatT const absAvgGrad = std::abs(prevGrads_[index]) / timeStep_;
    if (absAvgGrad > l1Strength_) {
      FloatT const rate = Util::sgn(prevGrads_[index]) * (timeStep_ * eta_
                                                     / std::sqrt(prevGradsSquared_[index]));
      this->params_[index] = rate * (l1Strength_ - absAvgGrad);
    } else
      this->params_[index] = 0.0f;
  }

  FloatT eta_, l1Strength_;
  Util::AutoDeleteArray<FloatT> prevGrads_, prevGradsSquared_;
  unsigned timeStep_;
};

/**
   Implements a generic online training algorithm that can be
   used for perceptron, stochastic gradient descent, and others.

   The objective function is responsible for computing the updates
   (e.g., CRF updates, perceptron updates, etc).

   \see ObjectiveFunction

   Adagrad is implemented after Green et al (2013). See also Chris
   Dyer's notes, http://www.ark.cs.cmu.edu/cdyer/adagrad.pdf.

   TODO Implement mini-batch updates.
   TODO Support infinite training data.
 */
template<class FloatT>
class OnlineOptimizer {
 public:

  OnlineOptimizer(OnlineOptimizerOptions const& opts)
      : opts_(opts) {}

  FloatT optimize(DataObjectiveFunction<FloatT>& objFct,
                  std::vector<FloatT>& params) {
    return optimize(objFct, arrayBegin(params), params.size());
  }

  /**
      Optimizes the objective function and writes the learned
      weights into params.
     *

      \params The initial params, which must have correct size.
   */
  FloatT optimize(DataObjectiveFunction<FloatT>& objFct,
                  FloatT *params, FeatureId numParams)
  {
    const std::size_t numExamples = objFct.getNumExamples();
    SDL_INFO(OnlineOptimizer, "Starting online optimization on " << numExamples
             << " training examples with " << opts_.numEpochs << " epochs");

    // Create a vector of indices that we can shuffle, for processing
    // the training examples in random order:
    std::vector<std::size_t> randomOrder;
    randomOrder.reserve(numExamples);
    for (std::size_t i = 0; i < numExamples; ++i) {
      randomOrder.push_back(i);
    }

    const std::size_t numUpdates = opts_.numEpochs * objFct.getNumExamples();
    shared_ptr<ILearningRate> pLearningRate =
        makeLearningRate(numUpdates,
                         opts_.learningRateOptions);

    bool useAdagrad = opts_.learningRateOptions.method == kAdagrad;
    bool useAdagradL1 = opts_.learningRateOptions.adagradL1Strength > 0.0f;

    boost::scoped_ptr<ParameterUpdate<FloatT> > update;
    if (useAdagrad) {
      if (useAdagradL1)
        update.reset(new AdagradL1ParameterUpdate<FloatT>(
            params, numParams, opts_.learningRateOptions.adagradRate,
            opts_.learningRateOptions.adagradL1Strength));
      else
        update.reset(new AdagradParameterUpdate<FloatT>(
            params, numParams, opts_.learningRateOptions.adagradRate));
    }
    else
      update.reset(new ParameterUpdate<FloatT>(params, numParams));

    // Iterate over all training examples opts_.numEpochs times:
    std::size_t cntSteps = 0;
    for (std::size_t epoch = 0; epoch < opts_.numEpochs; ++epoch) {
      std::random_shuffle(randomOrder.begin(), randomOrder.end());
      objFct.initFunctionValue();
      for (std::size_t i = 0; i < numExamples; ++i, ++cntSteps) {
        objFct.setFeatureWeights(i, i + 1, params, numParams);
        if (!useAdagrad) // AdaGrad sets its own learning rate
          update->setRate(static_cast<FloatT>((*pLearningRate)(cntSteps)));
        FloatT fctDiff = objFct.getUpdates(i, i + 1, *update);
        objFct.increaseFunctionValue(fctDiff);
        update->incTimeStep();
      }
      SDL_INFO(OnlineOptimizer, "Epoch " << epoch << ", function value: "
               << objFct.getFunctionValue());
    } // epochs

    SDL_INFO(OnlineOptimizer, "Finished online optimization, function value: "
             << objFct.getFunctionValue());
    return objFct.getFunctionValue();
  }

 private:
  OnlineOptimizerOptions opts_;

};


}}

#endif
