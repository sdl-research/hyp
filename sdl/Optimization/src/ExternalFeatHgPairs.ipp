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
#include <string>
#include <sstream>
#include <fstream>

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include <sdl/SharedPtr.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Hypergraph/IHypergraphsIteratorTpl.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Optimization/FeatureHypergraphPairs.hpp>

namespace sdl {
namespace Optimization {

namespace bfs = boost::filesystem;

template<class Arc>
ExternalFeatHgPairs<Arc>::ExternalFeatHgPairs(std::string const& location)
    : location_(location)
    , weights_(NULL)
{
  SDL_INFO(Optimization.ExternalFeatHgPairs, "Reading training archive from '"
           << location << "'");

  bfs::path pth1(bfs::path(location_) / bfs::path("size.txt"));
  std::ifstream in1(pth1.string().c_str());
  in1 >> size_;
  SDL_DEBUG(Optimization, "ExternalFeatHgPairs size: " << size_);

  bfs::path pth2(bfs::path(location_) / bfs::path("num-feats.txt"));
  std::ifstream in2(pth2.c_str());
  in2 >> numParams_;
  SDL_DEBUG(Optimization, "ExternalFeatHgPairs num feats: " << numParams_);
}

template<class Arc>
typename ExternalFeatHgPairs<Arc>::value_type
ExternalFeatHgPairs<Arc>::operator[](TrainingDataIndex i) {
  int numThousands = i / 1000;
  bfs::path pth(bfs::path(location_)
                / bfs::path("hg")
                / bfs::path(sdl::lexical_cast<std::string>(numThousands))
                / (sdl::lexical_cast<std::string>(i) + ".hg"));

  // We don't actually make use of the symbols
  // TODO: remove or pass in
  shared_ptr<PerProcessVocabulary> voc(
      new PerProcessVocabulary(Vocabulary::createDefaultVocab()));

  std::ifstream in(pth.string().c_str());
  SDL_DEBUG(Optimization, "Loading hypergraphs from " << pth.string());
  Hypergraph::IHypergraphsIteratorTpl<Arc>* iter =
      Hypergraph::IHypergraphsIteratorTpl<Arc>::create(
          in, Hypergraph::kDashesSeparatedHg, voc);
  iter->setHgProperties(Hypergraph::kStoreInArcs | Hypergraph::kStoreOutArcs);
  IHg* hg1 = iter->value();
  iter->next();
  assert(!iter->done());
  IHg* hg2 = iter->value();
  delete iter;

  typedef Hypergraph::IMutableHypergraph<Arc> MHg;
  detail::insertFeatureWeights(static_cast<MHg*>(hg1), weights_, numParams_);
  detail::insertFeatureWeights(static_cast<MHg*>(hg2), weights_, numParams_);

  return value_type(IHgPtr(hg1), IHgPtr(hg2));
}

template<class Arc>
void ExternalFeatHgPairs<Arc>::push_back(value_type const&) {
  SDL_THROW_LOG(Optimization.ExternalFeatHgPairs, UnimplementedException,
                "Cannot push back to ExternalFeatHgPairs -- it's read-only");
}

template<class Arc>
void ExternalFeatHgPairs<Arc>::setFeatureWeights(FloatT const* featWeights,
                                                 FeatureId numParams) {
  boost::lock_guard<boost::mutex> lock(mutex_);
  weights_ = featWeights;
  numParams_ = numParams;
  SDL_DEBUG(Optimization, "Received " << numParams << " feature weights");
}

template<class Arc>
void ExternalFeatHgPairs<Arc>::setFeatureWeights(TrainingDataIndex begin,
                                                 TrainingDataIndex end,
                                                 FloatT const* featWeights,
                                                 FeatureId numParams) {
  setFeatureWeights(featWeights, numParams);
}

}}
