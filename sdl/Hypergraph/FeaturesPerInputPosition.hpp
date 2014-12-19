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

/** \file

    place (alignment, usually) features for a simple sentence (sequence of
    tokens) - used by StringToHypergraph
*/

#ifndef HYP_HYPERGRAPH_FEATURESPERINPUTPOSITION_HPP
#define HYP_HYPERGRAPH_FEATURESPERINPUTPOSITION_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Hypergraph {

typedef std::vector<FeatureId> FeatureIdVec;

/**
   Interface for classes that define what features we have per
   input position.
*/
struct IFeaturesPerInputPosition {
  virtual ~IFeaturesPerInputPosition() {}

  /**
     Returns all features for a given input position.  ref is only valid until next call and isn't thread-safe

     TODO: return an array for more impl flexiblity
  */
  virtual FeatureIdVec const& getFeaturesForInputPosition(FeatureId position) = 0;
};

/**
   Use this if no input token features are wanted.
*/
struct NoFeatures : public IFeaturesPerInputPosition {
  NoFeatures() {}

  /**
     Returns all features for a given input position.
  */
  FeatureIdVec const& getFeaturesForInputPosition(FeatureId position) OVERRIDE {
    return feats_; // empty
  }

 private:
  FeatureIdVec const feats_;
};

/**
   Takes one feature per input position from the supplied
   vector.
*/
struct TakeFeaturesFromVector : public IFeaturesPerInputPosition {
  TakeFeaturesFromVector(FeatureIdVec const& vec)
      : from_(vec), feats_(1)
  {}

  FeatureIdVec const& getFeaturesForInputPosition(FeatureId position) OVERRIDE {
    assert(position < from_.size());
    feats_[0] = from_[position];
    return feats_;
  }

  FeatureIdVec const& from_;
  FeatureIdVec feats_;
};

/**
   Takes multiple features per input position from the supplied
   vector.
*/
struct MultipleFeaturesPerInputPosition : public IFeaturesPerInputPosition {
  MultipleFeaturesPerInputPosition(std::vector<FeatureIdVec > const& feats)
      : feats_(feats) {}

  FeatureIdVec const& getFeaturesForInputPosition(FeatureId position) OVERRIDE {
    return position < feats_.size() ? feats_[position] : empty_;
  }

  std::vector<FeatureIdVec> const& feats_;
  FeatureIdVec const empty_; // for the case that feats_ is too small
};


}}

#endif
