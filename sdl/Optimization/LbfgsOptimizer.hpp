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

   Contains code to optimize an objective function using L-BFGS.

   \author Markus Dreyer
 */

#ifndef SDL_OPTIMIZATION_LBFGSOPTIMIZER_HPP
#define SDL_OPTIMIZATION_LBFGSOPTIMIZER_HPP
#pragma once

#include <sdl/Optimization/ObjectiveFunction.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <cassert>
#include <cstdio>
#include <iomanip>
#include <map>
#include <string>
#include <lbfgs.h>

namespace sdl {
namespace Optimization {

// see lbfgs.h
SDL_ENUM(LbfgsLineSearchType, 4, (/** The default algorithm (MoreThuente method). */
                                  Default,
                                  /** Backtracking method with the Armijo condition. */
                                  Backtracking_Armijo,
                                  /** The backtracking method with the default (regular Wolfe) condition. */
                                  Backtracking,
                                  /** Backtracking method with strong Wolfe condition. */
                                  Backtracking_Strong_Wolfe));

/**
   Required by L-BFGS
 */
static lbfgsfloatval_t evaluate(void* instance, const lbfgsfloatval_t* x, lbfgsfloatval_t* g, const int n,
                                const lbfgsfloatval_t step) {
  SDL_DEBUG(Optimization.LbfgsOptimizer, "evaluate()");
  IObjectiveFunction<lbfgsfloatval_t>* pObjFct = (IObjectiveFunction<lbfgsfloatval_t>*)instance;
  return pObjFct->update(x, g, n);
}

/**
   Required by L-BFGS
 */
static int progress(void* instance, const lbfgsfloatval_t* x, const lbfgsfloatval_t* g,
                    const lbfgsfloatval_t fx, const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
                    const lbfgsfloatval_t step, int n, int num_iter, int ls) {
  SDL_INFO(Optimization, "Iter " << num_iter << ", "
                                 << "fx = " << fx << ", "
                                 << "xnorm = " << xnorm << ", "
                                 << "gnorm = " << gnorm << ", "
                                 << "step = " << step);
  return 0;
}

struct LbfgsOptimizerOptions {

  LbfgsOptimizerOptions()
      : maxIterations(50)
      , m(6)
      , xtol(std::numeric_limits<SdlFloat>::epsilon())
      , epsilon(1e-5)
      , ftol(1e-4)
      , gtol(0.9)
      , orthantwiseC(0.0f)
      , linesearch(kDefault)
      , max_step(1e20) {}

  // Passing liblbfgs options though. (Blame liblbfgs for the option
  // names!) The documentation/help texts for those options are from:
  // http://www.chokkan.org/software/liblbfgs/structlbfgs__parameter__t.html
  template <class Config>
  void configure(Config& config) {
    config("Options for the L-BFGS optimizer (Limited-memory Broyden–Fletcher–Goldfarb–Shanno)");
    config.is("LbfgsOptimizer");
    config("max-iterations", &maxIterations)(
        "Maximum number of iterations. A value of 0 means continue until convergence or error")
        .init(maxIterations);
    config("m", &m)(
        "The number of corrections to approximate the inverse Hessian matrix. The L-BFGS routine stores the "
        "computation results of previous m iterations to approximate the inverse hessian matrix of the "
        "current iteration. This parameter controls the size of the limited memories (corrections). The "
        "default value is 6. Values less than 3 are not recommended. Large values will result in excessive "
        "computing time.)")
        .init(m);
    config("xtol", &xtol)(
        "From liblbfgs: The machine precision for floating-point values. This parameter must be a positive "
        "value set by a client program to estimate the machine precision. The line search routine will "
        "terminate with the status code (LBFGSERR_ROUNDING_ERROR) if the relative width of the interval of "
        "uncertainty is less than this parameter.")
        .init(xtol);
    config("epsilon", &epsilon)(
        "From liblbfgs: Epsilon for convergence test. This parameter determines the accuracy with which the "
        "solution is to be found. A minimization terminates when ||g|| < epsilon * max(1, ||x||), where "
        "||.|| denotes the Euclidean (L2) norm. The default value is 1e-5.")
        .init(epsilon);
    config("ftol", &ftol)(
        "From liblbfgs: A parameter to control the accuracy of the line search routine. The default value is "
        "1e-4. This parameter should be greater than zero and smaller than 0.5.")
        .init(ftol);
    config("gtol", &gtol)(
        "From liblbfgs: A parameter to control the accuracy of the line search routine. The default value is "
        "0.9. If the function and gradient evaluations are inexpensive with respect to the cost of the "
        "iteration (which is sometimes the case when solving very large problems) it may be advantageous to "
        "set this parameter to a small value. A typical small value is 0.1. This parameter shuold be greater "
        "than the ftol parameter (1e-4) and smaller than 1.0.")
        .init(gtol);
    config("orthantwise-c", &orthantwiseC)(
        "Coefficient for the L1 norm of variables (0 means no L1). Note that L2 is handled separately by the "
        "optimization procedure, see option 'variance' there.")
        .init(orthantwiseC);
    config("linesearch", &linesearch)("L-BFGS line search method").init(linesearch);
    config("max-step", &max_step)("L-BFGS max step").init(max_step);
  }

  std::size_t maxIterations;
  std::size_t m;
  lbfgsfloatval_t xtol;
  lbfgsfloatval_t epsilon;
  lbfgsfloatval_t ftol;
  lbfgsfloatval_t gtol;
  lbfgsfloatval_t max_step;
  lbfgsfloatval_t orthantwiseC;
  LbfgsLineSearchType linesearch;
};

/**
   Runs L-BFGS to optimize a function.
 */
class LbfgsOptimizer {

 public:
  /**
      \param pObjFct The objective function to optimize

      \param lbfgsOptions Options to pass to L-BFGS library
   */
  LbfgsOptimizer(IObjectiveFunction<lbfgsfloatval_t>* pObjFct,
                 LbfgsOptimizerOptions const& opts = LbfgsOptimizerOptions())
      : opts_(opts), pObjFct_(pObjFct) {
    lbfgs_parameter_init(&lbfgsOptions_);
    lbfgsOptions_.max_iterations = (int)opts.maxIterations;
    lbfgsOptions_.m = (int)opts.m;
    lbfgsOptions_.xtol = opts.xtol;
    lbfgsOptions_.epsilon = opts.epsilon;
    lbfgsOptions_.ftol = opts.ftol;
    lbfgsOptions_.gtol = opts.gtol;
    lbfgsOptions_.orthantwise_c = opts.orthantwiseC;
    lbfgsOptions_.linesearch = opts.linesearch;
    lbfgsOptions_.max_step = opts.max_step;
  }

  int optimize(std::vector<lbfgsfloatval_t>& params) { return optimize(arrayBegin(params), params.size()); }

  /**
      \param x Initial parameters

      \param numParams number of parameters (i.e., length of x array)
   */
  int optimize(lbfgsfloatval_t* x, FeatureId numParams) {
    int ret = 0;
    lbfgsfloatval_t fx;

    if (lbfgsOptions_.orthantwise_c) {
      // TODO: test
      lbfgsOptions_.orthantwise_start = 0;
      lbfgsOptions_.orthantwise_end = numParams - 1;
      lbfgsOptions_.linesearch = kBacktracking;  // required for L1
      SDL_INFO(Optimization, "L-BFGS L1 regularization enabled with C="
                                 << lbfgsOptions_.orthantwise_c
                                 << " (linesearch was automatically set to 'backtracking')");
    }

    assert((int)numParams >= 0);
    // Run L-BFGS optimization; this will invoke the callback
    // functions evaluate() and progress() when necessary:
    ret = lbfgs((int)numParams, x, &fx, evaluate, progress, (void*)pObjFct_, &lbfgsOptions_);
    SDL_INFO(Optimization, "L-BFGS optimization terminated with status code " << ret << ", function value " << fx);
    return ret;
  }

 private:
  LbfgsOptimizerOptions opts_;
  lbfgs_parameter_t lbfgsOptions_;
  IObjectiveFunction<lbfgsfloatval_t>* pObjFct_;
};


}}

#endif
