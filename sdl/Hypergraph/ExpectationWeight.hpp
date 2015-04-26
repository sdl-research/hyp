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
/**
   \file

   Feature expectations for the expectation semiring.

   the relationship between FeatureWeight (with nonnegative feature values only)
   and ExpectationWeight is, for a linear derivation (single path):

   FeatureWeight(cost, [1=val1,...])

   is the same as

   ExpectationWeight(cost=-log(prob), [1=-log(val1*prob)=-log(val1)+cost,...])

   times in both cases adds the costs, and the values accumulate in the same way
   modulo the above isomorphism (exception: FeatureWeight with allowed negative
   values would not be isomorphic; we would need to introduce a sign bit to the
   ExpectationWeight's feature values)

   we could have kept the interpretations the same, but this way is more
   computationally efficient (because FeatureWeight plus selects the best path,
   while ExpectationWeight sums probabilities/expectations of things)

   think of ExpectationWeight as representing disjoint events.

   the sum of two ExpectationWeight is then a more probable single event.

   the feature vectors' values are -log(p(event)*E[count of feature|event]) - joint, not conditional

   (the overall probability is also a float value that's -log(prob))

   so adding two ExpectationWeight just separately (-LogPlus) adds the probs and
   expectation vectors

   further, the default value for weight[feature_id] is +infinity, not 0 as it
   is for featureweight

   the times (product) simply concatenates the events together (multiplying the
   two probabilities), and the resulting expectations are counts of features over
   the event (so the product event is less likely, but has more features on it, so
   higher conditional expectation)

   but because we don't store expectations conditional on the event, but rather
   the joint expectation in the overall probability distribution, this means that
   we have (a*b)[i]=-log(prob(a)*prob(b)*E[feature i|a*b]

   where E[feature i|a*b]) := E[feature i|a]+E[feature i|b])

   Now, times in detail:

   the algorithm for C=A*B is:

   costC = costA+costB
   fC[i] = costA+costB+logplus(fA[i]-costA, fB[i]-costB)
   =(probs)probA*probB*(pA[i]/probA+pB[i]/probB) = pA[i]*probB + pB[i]*probA
   = logplus(fA[i]+costB, fB[i]+costA)

   or to add C += B

   fC[i] = logplus(costB + fC[i], costC + fB[i])

   in case fC[i] = fB[i] (same map)

   fC[i] = logplus(costB + fC[i], costC + fC[i]) =(prob) probB*pC[i] + probC*pC[i] = (probB+probC)*pC[i]
   = logplus(costB, costC) + fC[i]

   in case C==B (so further costB == costC), you can take logplus(costC, costC) = costC - log(2)


   graehl thinks it really simplifies thinking about expectations if we use
   conditional (log) expectations in the feature values map. however, it's
   currently correct so don't touch it.

   \author Markus Dreyer

*/

#ifndef HYP__HYPERGRAPH_EXPECTATION_WEIGHT_HPP
#define HYP__HYPERGRAPH_EXPECTATION_WEIGHT_HPP
#pragma once

#include <stdexcept>
#include <string>
#include <iostream>
#include <cassert>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include <sdl/Hypergraph/FeatureWeight.hpp>

#include <sdl/Util/Constants.hpp>
#include <sdl/Util/LogHelper.hpp>

#include <sdl/Util/MultipliedMap.hpp>
#include <sdl/Util/LogMath.hpp>

#include <sdl/Hypergraph/Weight.hpp>

namespace sdl {
namespace Hypergraph {


/// ExpectationWeight is a type of FeatureWeight, where we take the
/// sum of values rather than the min:
typedef FeatureWeightTpl<FeatureValue, Features, Expectation> ExpectationWeight;


/**
   Adds another weight to this weight (using semiring
   plus). This is the version for Expectation policy.
*/
template <class FloatT, class MapT, class SumPolicy>
void FeatureWeightTpl<FloatT, MapT, SumPolicy>::plusBy(FeatureWeightTpl<FloatT, MapT, Expectation> const& b) {
  checkSumPolicy<Expectation>();
  if (b.isZero()) {
    // adding zero to this means do nothing. the expectations must all be zero too.
  } else if (isZero()) {
    *this = b;
  } else if (&b == this) {
    // initially, I thought the pmap equal by pointer case would handle this, but
    // i was wrong. ownMap() only copies if pointer wasn't unique. but the
    // pointer will be unique if the objects are identical.
    const FloatT neglog2 = -std::log((FloatT)2);
    this->value_ += neglog2;
    if (pMap_) {
      ownMap();
      for (iterator i = pMap_->begin(), e = pMap_->end(); i != e; ++i)
        i->second += neglog2;  // in prob space, we're adding p1+p2
    }
  } else {
    Util::NeglogPlusFct<FloatT> neglogplus;
    neglogplus(b.value_, this->value_);
    assert(!isZero());  // adding two nonzero things can never give a zero result
    if (pMap_ == b.pMap_) {  // maps equal by pointer. doesn't imply probabilities are the same too
      if (pMap_) {
        ownMap();
        const FloatT neglog2 = -std::log((FloatT)2);
        for (iterator i = pMap_->begin(), e = pMap_->end(); i != e; ++i)
          i->second += neglog2;  // in prob space, we're adding p1+p2
        // by multiplying by 2 (since the maps are the same, the joint probs are all the same)
      }
    } else {
      // add (joint) expectations:
      if (!b.empty()) {
        ownMap();
        MapT& thismap = *pMap_;
        MapT *bmap = b.pMap_.get();
        assert(&thismap != bmap);
        if (bmap)
          for (typename MapT::const_iterator i = bmap->begin(), e = bmap->end(); i != e; ++i)
            neglogplus.addToMap(thismap, i->first, i->second);
      }
    }
  }
}

/**
   Multiplies this weight by another weight (using semiring
   times). This is the version for Expectation policy.
*/
template <class FloatT, class MapT, class SumPolicy>
void FeatureWeightTpl<FloatT, MapT, SumPolicy>::timesBy(FeatureWeightTpl<FloatT, MapT, Expectation> const& b) {
  checkSumPolicy<Expectation>();
  if (isZero()) {
  } else if (b.isZero()) {
    setZero();
  } else if (b.isOne()) {
  } else if (isOne()) {
    *this = b;
  } else {
    FloatT const aval = this->value_;
    FloatT const bval = b.value_;
    this->value_ = aval + bval;  // pa' = pa*pb
    Util::NeglogPlusFct<FloatT> neglogplus;
    if (pMap_ == b.pMap_) {  // special (rare) case to avoid modifying map while reading from it
      ownMap();  // note: this must occur after the test above
      // fC[i] <= logplus(costB + fC[i], costC + fB[i]) = logplus(costB, costC) + fC[i]
      FloatT const aplusbval = neglogplus(aval, bval);  // we're setting (in prob space) v'=v*(pa+pb)
      SDL_DEBUG_ALWAYS(Hypergraph.ExpectationWeight, "same map for a*b; result[i] = a[i] + neglogplus("
                                                     << aval << ", " << bval << ") = " << aplusbval);
      for (typename MapT::iterator i = pMap_->begin(), e = pMap_->end(); i != e; ++i)
        i->second += aplusbval;  // *=(pa+pb)
    } else {
      assert(this != &b);
      ownMap();
      // fC[i] = logplus(costB + fC[i], costC + fB[i])
      // multiply A expectations by probB
      for (typename MapT::iterator i = pMap_->begin(), e = pMap_->end(); i != e; ++i)
        i->second += bval;
      // then add (B expectations multiplied by probA).
      if (!b.empty()) {
        MapT& thismap = *pMap_;
        MapT const* bmap = b.pMap_.get();
        assert(&thismap != bmap);
        if (bmap)
          for (typename MapT::const_iterator i = bmap->begin(), e = bmap->end(); i != e; ++i)
            neglogplus.addToMap(thismap, i->first, i->second + aval);
      }
    }
    assert(!isZero());  // we shouldn't ever underflow by adding two non-zero probs in logspace
  }
}


template <class FloatT, class MapT>
inline FeatureWeightTpl<FloatT, MapT, Expectation> plus(FeatureWeightTpl<FloatT, MapT, Expectation> const& w1,
                                                        FeatureWeightTpl<FloatT, MapT, Expectation> const& w2) {
  FeatureWeightTpl<FloatT, MapT, Expectation> w3(w1, true);
  w3.plusBy(w2);
  return w3;
}


/// result has p=w1.p*w2.p and feats=w1.p*w2.feats+w2.p*w1.feats
template <class FloatT, class MapT>
inline FeatureWeightTpl<FloatT, MapT, Expectation> times(FeatureWeightTpl<FloatT, MapT, Expectation> const& w1,
                                                         FeatureWeightTpl<FloatT, MapT, Expectation> const& w2) {
  FeatureWeightTpl<FloatT, MapT, Expectation> w3(w1, true);
  w3.timesBy(w2);
  return w3;
}


}}

#endif
