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
/** \file

    Add (weighted) feature to existing Weight. Unfortunate nomenclature: a
    feature weight (scaling factor, the thing that's tuned) is only slightly
    different than the 'Weight' in a FeatureWeight, which has feature values.

    TODO: we should have a string-keyed feature weights/names resource

*/

#ifndef HYP__MIXFEATURE_JG2012323_HPP
#define HYP__MIXFEATURE_JG2012323_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Hypergraph/ExpectationWeight.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <boost/optional.hpp>
#include <sdl/graehl/shared/is_null.hpp>
#include <sdl/graehl/shared/int_types.hpp>

// TODO: feature-name->id dictionary and feature weights abstraction, set id via name

// TODO: provide a standardized way of configuring a MixFeature from an external map<name:scale> for user
// experience consistency

namespace sdl {
namespace Hypergraph {

/**
   Functor for adding new (weighted) feature to weights. Moderately expensive to
   copy, so please pass by reference in performance-critical applications.

   //TODO: default FeatureValueT should be XmtFloatType or similar
*/
template <class FeatureIdT = FeatureId, class FeatureValueT = double>
struct MixFeature {
  std::string name;
  typedef typename graehl::signed_for_int<FeatureIdT>::signed_t IdConfigType;  // so -1 shows as -1
  IdConfigType
      id;  ///< if NOID(), the component feature won't be stored (just the weighted dotproduct contribution)
  /** \return don't add individual feature but do adjust weighted dotproduct. */

  static IdConfigType ConfigNOID() {
    return (IdConfigType)-1;
  }  // this should be used in options so we see a '-1'

  static FeatureIdT NOID() { return (FeatureIdT)-1; }  // this will be used in computation

  /// (mixing with a weight of 0 isn't a noop for FeatureWeight unless id is also NOID())
  FeatureValueT scale;

  void disable() { set_null(scale); }

  bool hasFeatureId() const { return id != ConfigNOID(); }

  void clear(FeatureValueT scale_ = 1.f) {
    id = ConfigNOID();
    name = "";
    scale = scale_;
  }

  MixFeature(FeatureValueT scale_ = 1.f) { clear(scale_); }

  template <class Id2, class Scale2>
  void operator=(MixFeature<Id2, Scale2> const& o) {
    name = o.name;
    id = o.id;
    scale = (FeatureValueT)o.scale;
  }

  template <class Config>
  void configure(Config& c) {
    c.is("Optional feature weight (and id).");
    c("Enabled only if weight is set (to 0 or otherwise). Adds feature if id != -1");
    c("name", &name)
        .verbose()("feature name");  //  (TODO: use to set id automatically from feature-names dictionary)
    c("id", &id).self_init()(
        "feature id (for FeatureWeight hypergraphs) -1 means don't add feature (but will still use feature "
        "if a weight is given)");
    c("weight", &scale)
        .self_init()(
            "weight for this feature's contribution to cost. 'nan' (not a number) means disabled (even if id "
            "is configured).");
  }

  void enable(FeatureValueT featureScale) { scale = featureScale; }

  bool enabled() const { return non_null(scale); }

  /** for FeatureWeight (and later, ExpectationWeight) */
  template <class FloatT>
  void addWeighted(FeatureValueT featureValueDelta, FloatWeightTpl<FloatT>& w) const {
    w.setValue((FloatT)(w.getValue() + scale * featureValueDelta));
  }

  // TODO: support an "overwrite feature id=featureValueDelta" vs. an "add delta"
  /** add feature id=featureValueDelta to w, adding featureValueDelta*scale to FloatWeight cost. */
  template <class FloatT, class MapT, class SumPolicy>
  void operator()(FeatureValueT featureValueDelta, FeatureWeightTpl<FloatT, MapT, SumPolicy>& w) const {
    if (enabled()) {
      addWeighted(featureValueDelta, w);
      if (hasFeatureId()) incrementFeatureValue(w, (FeatureIdT)id, (FloatT)featureValueDelta);
    }
  }

  template <class Weight>
  void operator()(FeatureValueT featureValueDelta, Weight& w) const {
    if (enabled()) timesBy(Weight(scale * featureValueDelta), w);
  }

  template <class Weight>
  void operator()(FeatureValueT featureValueDelta, TokenWeightTpl<Weight>& w) const {
    if (enabled()) w.allTimesBy(Weight(scale * featureValueDelta));
    // SDL_THROW_LOG(MixFeature, UnimplementedException, "can't MixFeature into TokenWeight (yet)");
  }

  /** Weird legacy behavior that OCRC test expects (TODO@ziyuan: update test to use FeatureWeight and we will
   * make this do the same thing as FeatureWeight, or eliminate ExpectationWeight entirely). */
  void operator()(FeatureValueT featureValueDelta, ExpectationWeight& w) const {
    w.insert((FeatureIdT)id, (ExpectationWeight::FeatureValueT)featureValueDelta);
  }

  template <class Weight>
  Weight weightedDelta(FeatureValueT featureValueDelta) const {
    Weight r((Weight::one()));
    (*this)(featureValueDelta, r);
    return r;
  }
};

template <class Id, class Val, class FeatArray, class Weight>
inline void mixFeatureArray(MixFeature<Id, Val> const* mixArray, FeatArray const& featArray,
                            unsigned arrayLen, Weight& mixTo) {
  for (unsigned i = 0; i < arrayLen; ++i) mixArray[i](featArray[i], mixTo);
}


}}

#endif
