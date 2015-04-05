// Copyright 2014 SDL plc
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
#ifndef HYP__HYPERGRAPH_FEATUREWEIGHTUTIL_HPP
#define HYP__HYPERGRAPH_FEATUREWEIGHTUTIL_HPP
#pragma once

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/DotProduct.hpp>
#include <sdl/Util/Contains.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/IsFeatureWeight.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Compute new cost from weights and features. (But some weights don't
   have features, so check IsFeatureWeight.)
*/
template<class Weight, class Result = double, class Enable = void>
struct FeatureDotProduct {
  typedef Result result_type;
  template <class WeightsMap>
  static inline Result dotProduct(Weight const& w, WeightsMap const& weights) {
    return 0;
  }
};

template<class Weight, class Result>
struct FeatureDotProduct<Weight, Result, typename Weight::IsFeatureWeight> {
  typedef Result result_type;
  template <class WeightsMap>
  static inline Result dotProduct(Weight const& w, WeightsMap const& weights) {
    return Util::DotProductResult<Result>(w.features(), weights).result;
  }
};

template <class Weight, class WeightsMap>
double dotProduct(Weight const& w, WeightsMap const& weights) {
  return FeatureDotProduct<Weight, double>::dotProduct(w, weights);
}


template<class Weight, class Enable = void>
struct FeaturesVisitor {
  template <class Visitor>
  static inline void visit(Weight const&, Visitor&) {
    // no op in no-features weight type (e.g., ViterbiWeight)
  }
};
template<class Weight>
struct FeaturesVisitor<Weight, typename Weight::IsFeatureWeight> {
  template <class Visitor>
  static inline void visit(Weight const& weight, Visitor& visitor) {
    typedef typename Weight::value_type value_type;
    forall (value_type aPair, weight) {
      visitor(aPair.first, aPair.second);
    }
  }
};

/**
   Calls function on each feature id and value in the feature
   weight (or no-op if not a feature weight). Good to print features,
   sum them, etc.
*/
template <class Weight, class Visitor>
void visitFeatures(Weight const& weight, Visitor& visitor) {
  return FeaturesVisitor<Weight>::visit(weight, visitor);
}


template<class Weight, class Enable = void>
struct GetNumFeatures {
  std::size_t operator()(Weight const&) const {
    return 0; // because not a feature weight
  }
};

// specialization for feature weight
template<class Weight>
struct GetNumFeatures<Weight, typename Weight::IsFeatureWeight> {
  std::size_t operator()(Weight const& weight) const {
    return weight.size();
  }
};

template <class Weight>
std::size_t getNumFeatures(Weight const& weight) {
  GetNumFeatures<Weight> getNum;
  return getNum(weight);
}


template<class Weight, class Enable = void>
struct RemoveFeatures {
  void operator()(Weight&) const {
    // not a feature weight: no-op
  }
};

// specialization for feature weight
template<class Weight>
struct RemoveFeatures<Weight, typename Weight::IsFeatureWeight> {
  void operator()(Weight& weight) const {
    weight.removeFeatures();
  }
};

template <class Weight>
void removeFeatures(Weight& weight) {
  RemoveFeatures<Weight> removeFeats;
  return removeFeats(weight);
}

////////////////////////////////////////////////////////////////////////////////

template <class FeaturesWrite>
void insertNewFeature(FeaturesWrite &map, typename FeaturesWrite::value_type const& idValuePair) {
  assert(!Util::contains(map, idValuePair.first));
  map.insert(idValuePair);
}

template <class FeaturesWrite>
void insertNewFeature(FeaturesWrite &map, typename FeaturesWrite::key_type id, typename FeaturesWrite::mapped_type val) {
  assert(!Util::contains(map, id));
  map.insert(typename FeaturesWrite::value_type(id, val));
}

/**
   Functor; inserts features into the weight if the weight type
   supports features.
*/
template<class Weight, class Enable = void>
struct FeatureInsertFct {
  enum { enabled = false };
  typedef unsigned key_type;
  typedef float mapped_type;
  template<class FeatureIndex, class FeatureValue>
  void operator()(Weight* w, FeatureIndex id, FeatureValue val) const {
    static bool first = true; // windows: ok to be thread-unsafe here
    if (first)
      SDL_INFO(Hypergraph.StringToHypergraph,
               "Hypergraph weight type does not support features.");
    first = false;
  }
};

template<class Weight>
struct FeatureInsertFct<Weight, typename Weight::IsFeatureWeight> {
  enum { enabled = true };
  typedef typename Weight::key_type key_type;
  typedef typename Weight::mapped_type mapped_type;
  void operator()(Weight* w, key_type id, mapped_type val) const {
    insertNewFeature(w->featuresWrite(), id, val);
  }
};


/**
   if weight supports features, add a new feature id=val (in debug mode, assert failure if it already existed).
*/
template <class Weight>
void insertFeature(Weight *w,
                   typename FeatureInsertFct<Weight>::key_type id,
                   typename FeatureInsertFct<Weight>::mapped_type val)
{
  FeatureInsertFct<Weight>()(w, id, val);
}

/**
   non-feature hgs never possess features in any range.
*/
template<class Weight, class Enable = void>
struct FeatureRangeVisitFct {
  template <class Visit, class FeatureIndex>
  static inline void visitRange(Weight const& weight, Visit const& visit, FeatureIndex begin, FeatureIndex end) {
  }
  template <class Visit, class FeatureRange>
  static inline void visitRange(Weight const& weight, Visit const& visit, FeatureRange const& range) {
  }
};

template<class Weight>
struct FeatureRangeVisitFct<Weight, typename Weight::IsFeatureWeight> {
  typedef typename Weight::key_type key_type;
  typedef typename Weight::mapped_type mapped_type;
  template <class Visit>
  static inline void visitRange(Weight const& weight, Visit const& visit, key_type begin, key_type end) {
    weight.visitFeatureRange(begin, end, visit);
  }
  template <class Visit, class FeatureRange>
  static inline void visitRange(Weight const& weight, Visit const& visit, FeatureRange const& range) {
    weight.visitFeatureRange(range.begin, range.end, visit);
  }
};


template <class Weight, class Visit, class FeatureIndex>
void visitFeatureRange(Weight const& weight, Visit const& visit, FeatureIndex begin, FeatureIndex end) {
  FeatureRangeVisitFct<Weight>::visitRange(weight, visit, begin, end);
}


template <class Weight, class Visit, class FeatureRange>
void visitFeatureRange(Weight const& weight, Visit const& visit, FeatureRange const& range)
{
  FeatureRangeVisitFct<Weight>::visitRange(weight, visit, range);
}


}}

#endif
