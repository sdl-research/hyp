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
/**
   \file

   Implements various learning rates for online learning and
   provides a factory function.

   \author Markus Dreyer
 */

#ifndef SDL_OPTIMIZATION_LEARNINGRATE_HPP_
#define SDL_OPTIMIZATION_LEARNINGRATE_HPP_
#pragma once

#include <cmath>
#include <sdl/Util/Enum.hpp>
#include <sdl/Types.hpp>
#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Optimization {


/**
   Learning rate for online training
 */
struct ILearningRate {
  virtual ~ILearningRate() {}

  virtual void setNumUpdates(std::size_t num) = 0;
  virtual SdlFloat operator()(std::size_t iter) = 0;
};


class ConstantLearningRateFct : public ILearningRate {
 public:
  ConstantLearningRateFct(SdlFloat eta_ = floatCast(0.1)) : eta(eta_) {}

  template <class Config>
  void configure(Config& config) {
    config("Options for a constant learning rate");
    config.is("ConstantLearningRateFct");
    config("eta", &eta).defaulted()("Float value to use as constant learning rate");
  }

  // Required, but no-op here
  void setNumUpdates(std::size_t) {}

  SdlFloat operator()(std::size_t) { return eta; }

  SdlFloat eta;
};

class ExponentialLearningRateFct : public ILearningRate {
 public:
  ExponentialLearningRateFct(SdlFloat initialEta_ = floatCast(0.2),
                             SdlFloat alpha_ = floatCast(0.85))  // recommended by Tsuruoka et al. (ACL 2009)
      : initialEta(initialEta_),
        alpha(alpha_),
        numUpdates(100)  // make compiler happy by initializing
  {}

  template <class Config>
  void configure(Config& config) {
    config("Options for an exponential learning rate");
    config.is("ExponentialLearningRateFct");
    config("eta", &initialEta)("Rate on first call").init(initialEta);
    config("alpha", &alpha)("Decay").init(alpha);
  }

  // Required
  void setNumUpdates(std::size_t num) { numUpdates = (SdlFloat)num; }

  SdlFloat operator()(std::size_t iter) { return initialEta * pow(alpha, iter / numUpdates); }

  SdlFloat numUpdates;
  SdlFloat initialEta;
  SdlFloat alpha;
};


/**
   Recommended in Nocedal's book "Optimization".
 */
class NocedalLearningRateFct : public ILearningRate {
 public:
  /**
      \param stepSize  step size, this is what you should play with to speed up convergence

      \param numIters  expected number of iterations (calls)
   */
  NocedalLearningRateFct(SdlFloat stepSize_ = floatCast(0.5))
      : stepSize(stepSize_)
      , alpha((SdlFloat)0.602)
      , length((SdlFloat)100)  // make compiler happy by initializing
  {}

  template <class Config>
  void configure(Config& config) {
    config("Options for the Nocedal learning rate");
    config.is("NocedalLearningRateFct");
    config("step-size", &stepSize)("Step size").init(stepSize);
  }

  // Required fct
  void setNumUpdates(std::size_t numUpdates) {
    // Nocedal recommends length to be 10% of the expected calls:
    length = (SdlFloat)numUpdates / SdlFloat(10);
  }

  SdlFloat operator()(std::size_t iter) {
    // Nick Andrews: "alpha must be between 0.5 and 1; alpha=0.602 is
    // special, and you shouldn't need to change this"
    return stepSize / pow((SdlFloat)((SdlFloat)iter + 1.0 + length), alpha);
  }

  SdlFloat stepSize;
  SdlFloat alpha;
  SdlFloat length;
};


SDL_ENUM(LearningRateType, 4, (Constant, Exponential, Nocedal, Adagrad));

struct LearningRateOptions {

  LearningRateOptions() : adagradRate(), adagradL1Strength() {}

  template <class Config>
  void configure(Config& config) {
    config("Options for the learning rate");
    config.is("LearningRate");
    config("method", &method)("Name of the method").init(kExponential);
    config("constant-rate", &constantRate)("Options if you pick method: 'constant'");
    config("exponential-rate", &exponentialRate)("Options if you pick method: 'exponential'");
    config("nocedal-rate", &nocedalRate)("Options if you pick method: 'nocedal'");
    config("adagrad-rate", &adagradRate)
        .init(1.0f)(
            "Adagrad rate (eta). Initial rate for each feature, which decreases whenever feature is "
            "observed.");
    config("adagrad-l1-strength", &adagradL1Strength)
        .init(0.0f)("Adagrad L1 strength (lambda). Positive number; 0 means no L1.");
  }

  LearningRateType method;
  ConstantLearningRateFct constantRate;
  ExponentialLearningRateFct exponentialRate;
  NocedalLearningRateFct nocedalRate;
  float adagradRate, adagradL1Strength;
};

/**
   Factory function to create a learning rate function object.
 */
shared_ptr<ILearningRate> makeLearningRate(std::size_t numUpdates, LearningRateOptions& opts);


}}

#endif
