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
#ifndef SDL_OPTIMIZATION_EXTERNALFEATHGPAIRS_HPP
#define SDL_OPTIMIZATION_EXTERNALFEATHGPAIRS_HPP
#pragma once

#include <sdl/Optimization/FeatureHypergraphPairs.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/SharedPtr.hpp>
#include <mutex>
#include <string>

namespace sdl {
namespace Optimization {

template <class ArcT>
class ExternalFeatHgPairs : public IFeatureHypergraphPairs<ArcT> {
 public:
  typedef ArcT Arc;
  typedef typename IFeatureHypergraphPairs<Arc>::IHg IHg;
  typedef typename IFeatureHypergraphPairs<Arc>::IHgPtr IHgPtr;
  typedef typename IFeatureHypergraphPairs<Arc>::value_type value_type;
  typedef typename IFeatureHypergraphPairs<Arc>::FloatT FloatT;

  ExternalFeatHgPairs(std::string const& location);

  value_type operator[](TrainingDataIndex i);

  void push_back(value_type const&);

  void setFeatureWeights(FloatT const* featWeights, FeatureId numParams);

  void setFeatureWeights(TrainingDataIndex begin, TrainingDataIndex end, FloatT const* featWeights,
                         FeatureId numParams);

  std::size_t size() const { return size_; }

  FeatureId getNumFeatures() { return numParams_; }

  void setNumFeatures(FeatureId n) { numParams_ = n; }

 private:
  TrainingDataIndex size_;
  std::string location_;
  FloatT const* weights_;
  FeatureId numParams_;
  std::mutex mutex_;
};


}}

#endif
