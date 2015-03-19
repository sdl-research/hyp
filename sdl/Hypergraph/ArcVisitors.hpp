/** \file

    arc modifiers, used as hg.forArcs(modifier)

    \author Markus Dreyer
*/

#ifndef HYP__HYPERGRAPH_ARCVISITORS_HPP
#define HYP__HYPERGRAPH_ARCVISITORS_HPP
#pragma once

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>

#include <sdl/Util/Forall.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Hypergraph/FeatureIdRange.hpp>

namespace sdl {
namespace Hypergraph {

/**
   visitor that visits a pair of visitors
*/
template<class Visitor1, class Visitor2>
struct CompositeVisitor : boost::noncopyable {

  CompositeVisitor(Visitor1* visitor1,
                   Visitor2* visitor2)
      : visitor1_(visitor1)
      , visitor2_(visitor2) {}

  ~CompositeVisitor() {
    delete visitor1_;
    delete visitor2_;
  }

  template<class Arc>
  void operator()(Arc* pArc) const {
    (*visitor1_)(pArc);
    (*visitor2_)(pArc);
  }

  Visitor1* visitor1_;
  Visitor2* visitor2_;
};


/// arc weight gets new constant value (e.g. 0)
template<class Arc>
struct SetWeightValueVisitor : boost::noncopyable {

  typedef typename Arc::Weight::FloatT FloatT;

  SetWeightValueVisitor(FloatT val) : val_(val) {}

  void operator()(Arc* pArc) const {
    pArc->weight().setValue(val_);
  }

  FloatT val_;
};


// Maps feature IDs to contiguous IDs.
//
// You can exclude a range of original feature IDs from this process
// -- they won't appear on the mapped arcs.
template<class Arc>
struct MapFeatureIdsVisitor : boost::noncopyable {
  typedef typename Arc::Weight Weight;
  typedef typename Weight::FeatureIdT FeatureIdT;
  typedef typename Weight::Map Map;
  typedef typename Weight::FloatT FloatT;
  typedef boost::unordered_map<FeatureIdT, FeatureIdT> IdsMap;
  typedef typename Map::value_type value_type;
  typedef typename Map::key_type key_type;
  typedef typename Map::mapped_type mapped_type;

  MapFeatureIdsVisitor(IdsMap* idsMap, IdsMap* idsMap_new2old)
      : idsMap_(idsMap)
      , idsMap_new2old_(idsMap_new2old)
  {}

  void setExcludeRange(FeatureIdRange excludeRange) {
    excludeRange_ = excludeRange;
  }

  void operator()(Arc* pArc) const {

    // Get original features map of this arc (uses orig, non-contiguous IDs):
    Map const& oldArcFeaturesMap = pArc->weight().features();

    // New features map for the arc (uses new, contiguous IDs):
    shared_ptr<Map> pNewMap(new Map());

    forall (value_type const& aPair, oldArcFeaturesMap) {
      if (excludeRange_ && (*excludeRange_).contains(aPair.first)) {
        continue;
      }
      key_type oldId = aPair.first;
      assert(idsMap_);
      assert(idsMap_new2old_);
      typename IdsMap::const_iterator iter = idsMap_->find(oldId);
      key_type newId;
      if (iter == idsMap_->end()) {
        newId = (key_type)idsMap_->size();
        if (newId == (key_type)-1)
          SDL_THROW_LOG(ArcVisitors, ConfigException,
                        "Too many feature IDs: Maximum ID of " << newId << " reached");
        idsMap_->insert(std::make_pair(oldId, newId));
        idsMap_new2old_->insert(std::make_pair(newId, oldId));
      }
      else {
        newId = iter->second;
      }
      pNewMap->insert(value_type(newId, aPair.second));
    }

    pArc->weight().setFeatures(pNewMap);
  }

  IdsMap* idsMap_;         /// Maps old feature IDs to new, contiguous IDs.
  IdsMap* idsMap_new2old_; /// Maps new, contiguous IDs back to old IDs.
  boost::optional<FeatureIdRange> excludeRange_;
};

/**
   given feature-weight array, set arc's weight's cost: sum {feature-weight[id] * feature-value[id]}
*/
template<class Arc>
struct InsertWeightsVisitor {

  typedef typename Arc::Weight Weight;
  typedef typename Weight::Map FeatMap;
  typedef typename Weight::FloatT FloatT;

  InsertWeightsVisitor(FloatT const* weights, FeatureId nweights)
      : weights_(weights)
  {
    SDL_DEBUG_BUILD(nweights_ = nweights);
  }

  ~InsertWeightsVisitor() {}

  void operator()(Arc* arc) const {
    FloatT weightVal = 0.0;
    Weight &w = arc->weight();
    FeatMap const& map = w.features();
    for (typename FeatMap::const_iterator it = map.begin(); it != map.end();
         ++it) {
      SDL_DEBUG_BUILD(assert(it->first < nweights_));
      weightVal += weights_[it->first] * it->second;
    }
    w.setValue(weightVal);
  }

  FloatT const* weights_;
  SDL_DEBUG_BUILD(FeatureId nweights_;)
};

/**
   given feature-weight map, set arc's weight's cost: sum {feature-weight[id] * feature-value[id]}

   (using a sparse feature-weights map)
*/
template<class Arc>
struct InsertSparseWeightsVisitor {

  typedef typename Arc::Weight::Map Map;
  typedef typename Arc::Weight::FloatT FloatT;

  InsertSparseWeightsVisitor(Map const& featureWeights)
      : featWeightsMap_(featureWeights)
  {}

  void operator()(Arc* arc) const {
    FloatT weightVal(static_cast<FloatT>(0.0));
    Map const& map = arc->weight().features();
    for (typename Map::const_iterator it = map.begin(); it != map.end();
         ++it) {
      typename Map::const_iterator weightsIter = featWeightsMap_.find(it->first);
      if (weightsIter != featWeightsMap_.end()) {
        weightVal += weightsIter->second * it->second;
      }
    }
    arc->weight().setValue(weightVal);
  }

  Map const& featWeightsMap_;
};

}}

#endif
