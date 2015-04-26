// Copyright 2014 SDL plc
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

    reapply feature weights to feature values (and/or affine scale the existing cost/dotproduct).
*/

#ifndef HYP__HG_REWEIGHT_HPP
#define HYP__HG_REWEIGHT_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Hypergraph/FeatureWeightUtil.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Hypergraph/IsFeatureWeight.hpp>
#include <sdl/Util/FnReference.hpp>
#include <sdl/graehl/shared/normalize_range.hpp>
#include <sdl/Util/Random.hpp>
#include <sdl/graehl/shared/is_null.hpp>
#include <sdl/graehl/shared/prefix_option.hpp>

namespace sdl {
namespace Hypergraph {

struct ReweightOptions : TransformOptionsBase {
  //  typedef boost::optional<double> D;
  Util::RandomSeed seed;
  static char const* caption() {
    return "Modify Arc Weights (real-valued costs), optionally (in order 1-5):";
  }
  ReweightOptions() {
    head_normalize = fsm_normalize = false;
    set_null(set);
    random_add = add = scale = set;
    clearFeatures = weightsAdd = false;
  }
  double set;
  double random_add;
  double add;
  double scale;
  typedef std::map<FeatureId, FeatureValue> Weights;
  Weights weights;
  bool weightsAdd;

// for debugging a difficult release bug, but no harm leaving it
#define REWEIGHT_CHECK_NAN(w, what, by)                                     \
  if (graehl::is_nan(w)) {                                                  \
    throw NotANumberException(__FUNCTION__, __FILE__, __LINE__, #what, by); \
  }

  /**
     if the user asked for --set X, set w to X.
  */
  void maybeSet(double& w) const {
    REWEIGHT_CHECK_NAN(w, pre - maybeSet, "");
    if (non_null(set)) w = set;
  }
  // weight type dependent weights dotproduct goes between these two
  void postSet(double& w, Util::Random01& rng) const {
    REWEIGHT_CHECK_NAN(w, before - postSet, set);
    if (non_null(random_add)) {
      double r = rng.random0n(random_add);
      REWEIGHT_CHECK_NAN(w, random0n, r);
      w += r;
      REWEIGHT_CHECK_NAN(w, post - random_add, random_add);
    }
    if (non_null(add)) {
      w += add;
      REWEIGHT_CHECK_NAN(w, post - add, add);
    }
    if (non_null(scale)) {
      w *= scale;
      REWEIGHT_CHECK_NAN(w, post - scale, scale);
    }
  }

  bool head_normalize;
  bool fsm_normalize;
  bool trivial() const {
    return is_null(set) && is_null(random_add) && is_null(add) && is_null(scale) && !clearFeatures
           && !fsm_normalize && !head_normalize && weights.empty();
  }
  template <class Config>
  void configure(Config& c, bool normalizeOpt = true) {
    seed.configure(c);
    c.is("Reweight");
    c("Modify hypergraph arc weights' values (real-valued costs, or -log (base e) probs, in a series of "
      "optional steps (numbered 0-5). nan means disabled");
    c("set", &set).init(NAN)("0. set to constant s");
    c("weights", &weights)(
        "1. (feature weighted FHG only) if nonempty, set cost to dotprod(weights, features) if "
        "weights-add=false, else add to existing cost");
    c("clear-features", &clearFeatures).init(false)("1b. clear features if set");
    c("weights-add", &weightsAdd)
        .init(false)(
            " modifying 1, weighted feature dotproduct adds to existing cost instead of replacing it");
    c("random-add", &random_add).init(NAN)("2. add randomly [0..r) - r may be negative");
    c("plus", &add).init(NAN)("3. add [plus]");
    c("times", &scale).init(NAN)("4. multiply by t - like probability^t");
    if (normalizeOpt) {
      c("head-normalize", &head_normalize)
          .init(false)("5a. normalize probabilities; sum_inarcs(exp(-cost))=1");
      c("fsm-normalize", &fsm_normalize)
          .init(false)(
              "5b. (only choose one) normalize probabilities in semiring; for non-lexical states, "
              "sum_outarcs(exp(-cost))=1");
      c("normalize-value-only", &valueNormalize)
          .init(true)(" for 5 (normalize), don't divide by feature values; just by probability");
    }
  }
  bool clearFeatures;
  bool valueNormalize;
  bool normalize() const { return head_normalize || fsm_normalize; }
  void validate() {
    if (head_normalize && fsm_normalize)
      SDL_THROW_LOG(Hypergraph, InvalidInputException,
                    "Reweight: can't have both head-normalize and fsm-normalize");
  }
  friend inline void validate(ReweightOptions& opt) { opt.validate(); }

  template <class Weight>
  void reweight(Weight& wt, Util::Random01& rng) const {
    double cost = wt.getValue();
    double newcost = cost;
    maybeSet(newcost);
    if (!weights.empty()) {
      if (!IsFeatureWeight<Weight>::value)
        SDL_THROW_LOG(Hypergraph.Reweight, ConfigException,
                      "supplied (unusable) feature weights for non-feature hypergraph");
      double weighted = FeatureDotProduct<Weight, double>::dotProduct(wt, weights);
      if (weightsAdd)
        newcost += weighted;
      else
        newcost = weighted;
    }
    postSet(newcost, rng);

    if (clearFeatures)
      wt = Weight((typename Weight::FloatT)newcost);
    else
      wt.setValue(newcost);
  }
};

template <class A>
struct ReweightNormalize {
  typedef typename A::Weight Weight;
  ReweightOptions opt;
  typedef std::vector<Weight> Sums;
  Sums normsum;
  const bool norm;
  ReweightNormalize(ReweightOptions const& opt, IMutableHypergraph<A>& hg)
      : opt(opt), normsum(opt.normalize() ? hg.size() : 0, Weight::zero()), norm(opt.normalize()), rng(opt.seed) {
    hg.forArcsSafe(Util::visitorReference(*this));
    if (opt.normalize()) hg.forArcsSafe(NormalizePass(*this));
  }

  typedef ReweightNormalize Self;

  struct NormalizePass {
    typedef void result_type;
    Self& self;
    NormalizePass(Self& self) : self(self) {}
    template <class X>
    result_type operator()(X* pArc) const {
      A& a = *pArc;
      a.setWeight((divide(a.weight(), self.normFor(a))));
    }
  };

  Weight& normFor(A const& a) {
    assert(norm);
    return normsum[opt.head_normalize ? a.head() : a.fsmSrc()];
  }
  Util::Random01 rng;
  void operator()(A* ap) {
    A& a = *ap;
    Weight& wt = a.weight();
    opt.reweight(wt, rng);
    if (norm) {
      if (opt.valueNormalize)
        Hypergraph::plusBy(Weight((typename Weight::FloatT)wt.getValue()), normFor(a));
      else
        Hypergraph::plusBy(wt, normFor(a));
    }
  }
};


template <class Arc>
void reweight(IMutableHypergraph<Arc>& hg, ReweightOptions const& opt) {
  ReweightNormalize<Arc> rw(opt, hg);
}

struct Reweight : TransformBase<Transform::Inplace>, ReweightOptions {
  Reweight(ReweightOptions const& opt = ReweightOptions()) : ReweightOptions(opt) {}
  template <class Arc>
  bool needs(IMutableHypergraph<Arc>& h) const {
    return !trivial();
  }
  Properties inAddProps() const { return fsm_normalize ? kFsm : 0; }
  template <class Arc>
  void inplace(IMutableHypergraph<Arc>& hg) const {
    Hypergraph::reweight(hg, *this);
  }
};

template <class Arc>
struct TransformFor<ReweightOptions, Arc> {
  typedef Reweight type;
};


}}

#endif
