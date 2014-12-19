#ifndef SDL_OPTIMIZATION_EXTERNALFEATHGPAIRS_HPP
#define SDL_OPTIMIZATION_EXTERNALFEATHGPAIRS_HPP
#pragma once

#include <string>
#include <boost/thread/mutex.hpp>

#include <sdl/SharedPtr.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Optimization/FeatureHypergraphPairs.hpp>

namespace sdl {
namespace Optimization {

template<class ArcT>
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

  void setFeatureWeights(FloatT const* featWeights,
                         FeatureId numParams);

  void setFeatureWeights(TrainingDataIndex begin,
                         TrainingDataIndex end,
                         FloatT const* featWeights,
                         FeatureId numParams);

  std::size_t size() const {
    return size_;
  }

  FeatureId getNumFeatures() {
    return numParams_;
  }

  void setNumFeatures(FeatureId n) {
    numParams_ = n;
  }

 private:
  TrainingDataIndex size_;
  std::string location_;
  FloatT const* weights_;
  FeatureId numParams_;
  boost::mutex mutex_;
};

}}

#endif
