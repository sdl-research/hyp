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
#include <vector>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <sdl/SharedPtr.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Equal.hpp>
#include <sdl/Util/Forall.hpp>

#include <sdl/Vocabulary/HelperFunctions.hpp>

#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Compose.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Hypergraph/ArcVisitors.hpp>
#include <sdl/Hypergraph/GetString.hpp>
#include <sdl/Hypergraph/BestPathString.hpp>

#include <sdl/Optimization/OptimizationProcedure.hpp>
#include <sdl/Optimization/Exception.hpp>
#include <sdl/Optimization/LoadFeatureWeights.hpp>
#include <sdl/Optimization/IOriginalFeatureIds.hpp>

#include <sdl/graehl/shared/percent.hpp>
#include <sdl/Util/Delete.hpp>

namespace sdl {
namespace Optimization {

namespace detail {

// no need for anon namespace or inline since this is a .cpp

/**
   from lbfgs.h with search-replace into strings
*/
char const* lbfgsErrorTexts[] = {
    "Unknown error. (LBFGSERR_UNKNOWNERROR)", "Logic error. (LBFGSERR_LOGICERROR)",
    "Insufficient memory. (LBFGSERR_OUTOFMEMORY)",
    "The minimization process has been canceled. (LBFGSERR_CANCELED)",
    "Invalid number of variables specified. (LBFGSERR_INVALID_N)",
    "Invalid number of variables (for SSE) specified. (LBFGSERR_INVALID_N_SSE)",
    "The array x must be aligned to 16 (for SSE). (LBFGSERR_INVALID_X_SSE)",
    "Invalid parameter lbfgs_parameter_t::epsilon specified. (LBFGSERR_INVALID_EPSILON)",
    "Invalid parameter lbfgs_parameter_t::past specified. (LBFGSERR_INVALID_TESTPERIOD)",
    "Invalid parameter lbfgs_parameter_t::delta specified. (LBFGSERR_INVALID_DELTA)",
    "Invalid parameter lbfgs_parameter_t::linesearch specified. (LBFGSERR_INVALID_LINESEARCH)",
    "Invalid parameter lbfgs_parameter_t::max_step specified. (LBFGSERR_INVALID_MINSTEP)",
    "Invalid parameter lbfgs_parameter_t::max_step specified. (LBFGSERR_INVALID_MAXSTEP)",
    "Invalid parameter lbfgs_parameter_t::ftol specified. (LBFGSERR_INVALID_FTOL)",
    "Invalid parameter lbfgs_parameter_t::wolfe specified. (LBFGSERR_INVALID_WOLFE)",
    "Invalid parameter lbfgs_parameter_t::gtol specified. (LBFGSERR_INVALID_GTOL)",
    "Invalid parameter lbfgs_parameter_t::xtol specified. (LBFGSERR_INVALID_XTOL)",
    "Invalid parameter lbfgs_parameter_t::max_linesearch specified. (LBFGSERR_INVALID_MAXLINESEARCH)",
    "Invalid parameter lbfgs_parameter_t::orthantwise_c specified. (LBFGSERR_INVALID_ORTHANTWISE)",
    "Invalid parameter lbfgs_parameter_t::orthantwise_start specified. (LBFGSERR_INVALID_ORTHANTWISE_START)",
    "Invalid parameter lbfgs_parameter_t::orthantwise_end specified. (LBFGSERR_INVALID_ORTHANTWISE_END)",
    "The line-search step went out of the interval of uncertainty. (LBFGSERR_OUTOFINTERVAL)",
    "A logic error occurred; alternatively, the interval of uncertainty became too small. "
    "(LBFGSERR_INCORRECT_TMINMAX)",
    "A rounding error occurred; alternatively, no line-search step satisfies the sufficient decrease and "
    "curvature conditions. (LBFGSERR_ROUNDING_ERROR)",
    "The line-search step became smaller than lbfgs_parameter_t::min_step. (LBFGSERR_MINIMUMSTEP)",
    "The line-search step became larger than lbfgs_parameter_t::max_step. (LBFGSERR_MAXIMUMSTEP)",
    "The line-search routine reaches the maximum number of evaluations. (LBFGSERR_MAXIMUMLINESEARCH)",
    "The algorithm routine reaches the maximum number of iterations. (LBFGSERR_MAXIMUMITERATION)",
    "Relative width of the interval of uncertainty is at most lbfgs_parameter_t::xtol. "
    "(LBFGSERR_WIDTHTOOSMALL)",
    "A logic error (negative line-search step) occurred. (LBFGSERR_INVALIDPARAMETERS)",
    "The current search direction increases the objective function value. (LBFGSERR_INCREASEGRADIENT)",
};

char const* lbfgsStatus(int status) {
  if (status == LBFGS_SUCCESS) return "converged";
  if (status == LBFGS_ALREADY_MINIMIZED) return "converged (already optimized)";

  int errorBegin = LBFGSERR_UNKNOWNERROR;
  int nErrorsKnown = sizeof(lbfgsErrorTexts) / sizeof(lbfgsErrorTexts[0]);
  int i = status - errorBegin;
  return (i < 0 || i >= nErrorsKnown) ? "undocumented status code (check LBFGS docs online?)"
                                      : lbfgsErrorTexts[i];
}

/**
   Checks all gradients analytically -- used for debugging
   gradients.
 */
template <class FloatT>
bool checkGradients(IObjectiveFunction<FloatT>& objFct, FloatT* params, const FeatureId numParams,
                    FloatT tolerance = 1e-3) {
  SDL_INFO(Optimization.OptimizationProcedure, "Starting gradients check");
  if (!numParams) return true;

  const FloatT kGradientCheckEpsilon = 1e-6;
  Util::AutoDeleteArray<FloatT> gradients(numParams);
  FloatT* gradients0 = gradients;
  std::size_t cntWarnings = 0;

  // 1. Record the original function value:
  FloatT fctValBefore = objFct.update(params, gradients0, numParams);

  // Check the gradient of each parameter:
  for (std::size_t i = 0; i < numParams; ++i) {
    SDL_DEBUG(Optimization.OptimizationProcedure, "Checking gradient " << i);

    // 2. Change just one parameter by epsilon:
    FloatT orig = params[i];
    params[i] += kGradientCheckEpsilon;

    // 3. Record how that changed the function value:
    FloatT fctValAfter = objFct.update(params, gradients0, numParams);

    // 4. Check that the scaled function value difference is the same
    // as the provided gradient:
    FloatT wantGradient = (fctValAfter - fctValBefore) / kGradientCheckEpsilon;
    if (!Util::floatEqual(wantGradient, gradients[i], tolerance)) {
      SDL_WARN(Optimization.OptimizationProcedure, std::setprecision(12)
                                                   << "Gradient for feature " << i << " is " << gradients[i]
                                                   << ", but should be " << wantGradient);
      ++cntWarnings;
    }
    params[i] = orig;
  }
  if (cntWarnings) {
    SDL_WARN(Optimization.OptimizationProcedure,
             "Number of incorrect gradients: " << cntWarnings << " / " << numParams << " = "
                                               << graehl::percent<5>(cntWarnings, numParams) << "%");
  } else { SDL_INFO(Optimization.OptimizationProcedure, "Gradients OK"); }
  return !cntWarnings;
}
}

OptimizationProcedure::OptimizationProcedure(shared_ptr<ICreateSearchSpace<Arc> >& searchSpace,
                                             OptimizationProcedureOptions const& opts)
    : opts_(opts), checkIntersectionNonEmpty_(false), checkGradients_(false), pSearchSpace_(searchSpace) {
  if (opts_.testMode) {
    // Will prevent from mapping feature IDs to contiguous space:
    pSearchSpace_->setTestMode();
  }
  pSearchSpace_->prepareTraining();
}

// TODO: Could do automatic evaluation (against the clamped
// hypergraphs in hgPairs[i].first).
void OptimizationProcedure::test() {
  SDL_DEBUG(Optimization.OptimizationProcedure, "Starting test()");
  loadFeatureWeightsFile(opts_.weightsPath, &weightsmap_);
  Hypergraph::InsertSparseWeightsVisitor<Arc> addDotProduct(weightsmap_);

  typedef IFeatureHypergraphPairs<Arc> Pairs;
  typedef Pairs::IHgPtr IHgPtr;
  Pairs& hgPairs = *pSearchSpace_->getFeatureHypergraphPairs();
  SDL_TRACE(Optimize, "Have " << hgPairs.size() << " hg pairs.");
  for (std::size_t i = 0, end = hgPairs.size(); i < end; ++i) {
    SDL_TRACE(Optimize, "Example " << i);
    IHgPtr unclamped(hgPairs[i].second);
    unclamped->visitArcs(addDotProduct);
    if (i == 0)
      SDL_DEBUG_ALWAYS(Optimize.first.unclamped, "Unclamped first hg:\n" << *unclamped);
    else
      SDL_TRACE(Optimize.unclamped, "Unclamped hg:\n" << *unclamped);
    if (opts_.testModeDetailed) {
      Hypergraph::Derivation<Arc>::child_type deriv
          = Hypergraph::bestPath(*unclamped, Hypergraph::BestPathOptions());
      std::cout << deriv->weight() << "\t";
      std::cout << Hypergraph::textFromDeriv(deriv, *unclamped, Hypergraph::DerivationStringOptions())
                << "\n";
    } else if (opts_.testModeOutputHypergraph) {
      if (i > 0) std::cout << "-----\n";
      std::cout << *unclamped;
    } else { std::cout << Hypergraph::bestPathString(*unclamped) << '\n'; }
  }
}

void OptimizationProcedure::optimize() {
  SDL_DEBUG(Optimization.OptimizationProcedure, "Starting optimize() for " << pSearchSpace_->getNumFeatures()
                                                                           << " feature weights");

  HypergraphCrfObjFct<Arc> objFct(pSearchSpace_->getFeatureHypergraphPairs());
  objFct.setRegularizeFct(new L2RegularizeFct<FloatT>(opts_.variance));
  objFct.setNumThreads(opts_.numThreads);

  const FeatureId numParams = pSearchSpace_->getNumFeatures();
  Util::AutoDeleteArray<FloatT> paramsa(numParams, FloatT(0.0));
  FloatT* params = paramsa;

  if (checkGradients_) {
    assert(!opts_.testMode);
    detail::checkGradients(objFct, params, numParams);
  }

  if (opts_.optimizationMethod == kLbfgs) {
    SDL_INFO(sdl.Optimization, "Using L-BFGS optimization (LBFGS_FLOAT=" << LBFGS_FLOAT << ")");

    LbfgsOptimizer optimizer(&objFct, opts_.lbfgsOptions);
    int errorCode = optimizer.optimize(params, numParams);
    if (errorCode) {
      if (errorCode == LBFGSERR_MAXIMUMITERATION) {
        SDL_INFO(OptimizationProcedure, "Specified max number of L-BFGS iterations reached.");
      } else {
        SDL_THROW_LOG(Optimization, OptimizationException,
                      "L-BFGS returned non-zero error code: "
                      << errorCode << " - " << detail::lbfgsStatus(errorCode)
                      << ". Debugging: setCheckIntersectionNonEmpty() can help diagnose input-output "
                         "mismatch; "
                      << "setCheckGradients() can help diagnose gradient computation bugs.");
      }
    }
  } else if (opts_.optimizationMethod == kOnline) {
    SDL_INFO(sdl.Optimization, "Using online optimization");
    OnlineOptimizer<FloatT> optimizer(opts_.onlineOptions);
    optimizer.optimize(objFct, params, numParams);
  } else { SDL_THROW_LOG(Optimization, OptimizationException, "Unknown optimization method"); }

  SDL_DEBUG(OptimizationProcedure, "Writing learned weights to " << opts_.weightsPath);
  Util::Output weightsOutput(opts_.weightsPath);
  Optimization::IOriginalFeatureIds* ids
      = dynamic_cast<Optimization::IOriginalFeatureIds*>(pSearchSpace_.get());
  bool doDelete = false;
  if (ids == NULL) {
    ids = new Optimization::IdentityOriginalFeatureIds();
    doDelete = true;
  }
  for (FeatureId i = 0; i < numParams; ++i) {
    weightsOutput.getStream() << ids->getOriginalFeatureId(i) << "\t" << params[i] << '\n';
  }
  if (doDelete) delete ids;
}


}}
