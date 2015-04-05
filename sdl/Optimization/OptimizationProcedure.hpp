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
/**
   \file

   Procedure for optimizing hypergraph weights.

   The procedure is: Read data, generate features, construct
   parameterized objective function, optimize function parameters.

   \author Markus Dreyer
 */

#ifndef SDL_OPTIMIZATION_OPTIMIZATIONPROCEDURE_HPP
#define SDL_OPTIMIZATION_OPTIMIZATIONPROCEDURE_HPP
#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

#include <sdl/SharedPtr.hpp>

#include <sdl/Util/Input.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/StringToTokens.hpp>

#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/StringToHypergraph.hpp>
#include <sdl/Hypergraph/ArcVisitors.hpp>

#include <sdl/Optimization/HypergraphCrfObjFct.hpp>
#include <sdl/Optimization/LbfgsOptimizer.hpp>
#include <sdl/Optimization/OnlineOptimizer.hpp>
#include <sdl/Optimization/ICreateSearchSpace.hpp>
#include <sdl/Optimization/Arc.hpp>

namespace sdl {
namespace Optimization {

SDL_ENUM(OptimizationMethod, 2, (Lbfgs, Online));

struct OptimizationProcedureOptions {

  OptimizationProcedureOptions() {
    Config::inits(this);
  }

  template <class Config>
  void configure(Config& config) {
    config("Optimize the weights of an SDL module");
    config.is("OptimizationProcedure");

    config("optimization-method", &optimizationMethod)("Either 'lbfgs' or 'online'. For each method you can specify options, e.g., lbfgs").init(kLbfgs);
    config("lbfgs", &lbfgsOptions)("Options for 'lbfgs' (use if optimization-method is 'lbfgs')");
    config("online", &onlineOptions)("Options for 'online' (use if optimization-method is 'online')");

    config("variance", &variance)("Variance for L2 regularization").init(floatCast(2.0));
    config("weights-path", &weightsPath)("Output path for learned weights ('-' is stdout)").init("-");
    config("test-mode", &testMode)("Test mode? In test mode, we don't optimize, but read the weights and get the best path").init(false);
    config("test-mode-output-hypergraph", &testModeOutputHypergraph)("Prints hypergraph output (test mode)").init(false);
    config("test-mode-detailed-output", &testModeDetailed)("Will produce detailed info for 1-best (alignment, features, scores) ").init(false);
    config("num-threads", &numThreads)("Number of threads for computing feature expectations on the aligned data").init(1);
  }

  OptimizationMethod optimizationMethod;
  LbfgsOptimizerOptions lbfgsOptions;
  OnlineOptimizerOptions onlineOptions;

  std::string weightsPath;
  double variance;
  bool testMode;
  bool testModeDetailed;
  bool testModeOutputHypergraph;
  std::size_t numThreads;
};

/*
Procedure for optimizing hypergraph weights.
 */
class OptimizationProcedure
{
 public:
  typedef Optimization::FloatT FloatT;
  typedef Optimization::Arc Arc;

  OptimizationProcedure(shared_ptr<ICreateSearchSpace<Arc> >& searchSpace
                        , OptimizationProcedureOptions const&);

  /**
      Starts the optimization. Call this after all training
      examples have been added.
   */
  void optimize();

  /**
      If true: Will check on every training pair if the two
      created hypergraphs are compatible (i.e., they
      intersect). Expensive test, use only for debugging.
   */
  void setCheckIntersectionNonEmpty(bool b = true) {
    checkIntersectionNonEmpty_ = b;
  }

  /**
      If true: Will check if the gradients and the function
      value (as computed based on the task-specific CreateSearchSpace
      functor) are compatible. Expensive test, use only for debugging.
   */
  void setCheckGradients(bool b = true) {
    checkGradients_ = b;
  }

  /**
      Gets the best path from all unclamped hypergraphs, after
      weighting the arcs according to the passed weights map.
   */
  void test();

 private:
  OptimizationProcedureOptions opts_;

  shared_ptr<ICreateSearchSpace<Arc> > pSearchSpace_;
  bool checkIntersectionNonEmpty_;
  bool checkGradients_;
  Weight::Map weightsmap_;
};


}}

#endif
