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
/**
   \file

   Classes that represent pairs of IHypergraph<FeatureWeight>,
   used as training data in DataObjectiveFunction objects.

   Each pair represents (clamped-to-observed-input-output,
   clamped-to-input-only), where the second is the search space over
   possible outputs given the input observation.

   \author Markus Dreyer
 */

#ifndef SDL_OPTIMIZATION_FEATUREHYPERGRAPHPAIRS_HPP
#define SDL_OPTIMIZATION_FEATUREHYPERGRAPHPAIRS_HPP
#pragma once

#include <utility>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>
#include <sdl/SharedPtr.hpp>

#include <sdl/Util/Sleep.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Assert.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/ArcVisitors.hpp>
#include <sdl/Optimization/IOriginalFeatureIds.hpp>
#include <sdl/Optimization/Types.hpp>

namespace sdl {
namespace Optimization {

namespace bfs = boost::filesystem;

namespace detail {

/**
   Changes arc weights according to new feature weights. Uses
   WeightsInserter.
 */
template <class Arc>
void insertFeatureWeights(Hypergraph::IMutableHypergraph<Arc>* pHg,
                          typename Arc::Weight::FloatT const* pWeights, FeatureId nWeights) {
  if (pWeights) {
    Hypergraph::InsertWeightsVisitor<Arc> inserter(pWeights, nWeights);
    pHg->visitArcs(inserter);
  }
}
}

/**
   Represents training data, which can be available as an
   array, loaded from disk on demand, etc.
 */
template <class ArcT>
class IFeatureHypergraphPairs {
 public:
  typedef Hypergraph::IHypergraph<ArcT> IHg;
  typedef shared_ptr<IHg> IHgPtr;
  typedef ArcT Arc;
  typedef std::pair<IHgPtr, IHgPtr> value_type;
  typedef typename ArcT::Weight Weight;
  typedef typename Weight::FloatT FloatT;

  virtual ~IFeatureHypergraphPairs() {}

  virtual value_type operator[](TrainingDataIndex) = 0;

  virtual void push_back(value_type const&) = 0;

  virtual void finish() {}

  /**
      Changes the arc weights according to new feature weights.
   */
  virtual void setFeatureWeights(FloatT const* featWeights, FeatureId numParams) = 0;

  /**
      Changes the arc weights of certain HG pairs according to
      new feature weights. (Good for online learning.)
   */
  virtual void setFeatureWeights(TrainingDataIndex begin, TrainingDataIndex end, FloatT const* featWeights,
                                 FeatureId numParams) = 0;

  /**
      \return The number of Hypergraph pairs.
   */
  virtual TrainingDataIndex size() const = 0;

  virtual FeatureId getNumFeatures() = 0;
  virtual void setNumFeatures(FeatureId) = 0;
};

/**
   In-memory implementation of IFeatureHypergraphPairs, i.e.,
   all training examples are in memory.)
 */
template <class ArcT>
class InMemoryFeatureHypergraphPairs : public IFeatureHypergraphPairs<ArcT> {
 public:
  typedef ArcT Arc;
  typedef typename IFeatureHypergraphPairs<Arc>::value_type value_type;
  typedef typename IFeatureHypergraphPairs<Arc>::FloatT FloatT;
  typedef std::vector<value_type> Vector;
  typedef shared_ptr<Vector> VectorPtr;

  InMemoryFeatureHypergraphPairs() : pPairs_(new std::vector<value_type>()) {}

  /**
      \param pPairs Pointer to all hypergraph pairs (i.e., complete
      training data incl. features); takes ownership and will delete at
      end.
   */
  InMemoryFeatureHypergraphPairs(VectorPtr const& pPairs) : pPairs_(pPairs), numParams_(0) {}

  value_type operator[](TrainingDataIndex index)OVERRIDE {
    SDL_ASSERT_MSG(pPairs_->size() >= index, "index out of bounds");
    return (*pPairs_)[index];
  }

  /**
      Inserts the feature weights into all stored hypergraphs.
     *
      TODO: This could be made more efficient by having a map from
      feature IDs to arcs that use them.
   */
  void setFeatureWeights(FloatT const* featWeights, FeatureId numParams) OVERRIDE {
    SDL_DEBUG(Optimization.HypergraphCrfObjFct, "Setting feature weights");
    setFeatureWeights(0, size(), featWeights, numParams);
  }

  /**
      \param begin First hypergraph pair index

      \param end Last hypergraph pair index plus one
   */
  void setFeatureWeights(TrainingDataIndex begin, TrainingDataIndex end, FloatT const* featWeights,
                         FeatureId numParams) OVERRIDE {
    SDL_DEBUG(Optimization.HypergraphCrfObjFct, "Setting feature weights for HGs (" << begin << ", " << end
                                                                                    << "]");
    for (TrainingDataIndex i = begin; i < end; ++i) {
      value_type const& hgpair = (*pPairs_)[i];
      assert(hgpair.first->isMutable());
      assert(hgpair.second->isMutable());
      typedef Hypergraph::IMutableHypergraph<Arc> MHg;
      detail::insertFeatureWeights(static_cast<MHg*>(hgpair.first.get()), featWeights, numParams);
      detail::insertFeatureWeights(static_cast<MHg*>(hgpair.second.get()), featWeights, numParams);
    }
  }

  void push_back(value_type const& val) OVERRIDE { pPairs_->push_back(val); }

  TrainingDataIndex size() const OVERRIDE { return pPairs_->size(); }

  FeatureId getNumFeatures() OVERRIDE { return numParams_; }
  void setNumFeatures(FeatureId n) OVERRIDE { numParams_ = n; }

 private:
  VectorPtr pPairs_;
  FeatureId numParams_;
};

/**
 * @brief This just writes each HG pair to disk immediately, nothing
 * else.
 */
template <class ArcT>
class WriteFeatureHypergraphPairs : public IFeatureHypergraphPairs<ArcT> {
 public:
  typedef ArcT Arc;
  typedef typename IFeatureHypergraphPairs<Arc>::value_type value_type;
  typedef typename IFeatureHypergraphPairs<Arc>::FloatT FloatT;

  enum { kSleepForMicroSeconds = 500 };

  WriteFeatureHypergraphPairs(std::string const& fname) : dir_(fname), size_(0), project_(true) {
    bfs::create_directories(fname + "/hg");
    Util::microSleep(kSleepForMicroSeconds);  // to be safe on NFS
    SDL_INFO(Optimization.WriteFeatureHypergraphPairs, "Writing training archive to '" << fname << "'");
  }

  void push_back(value_type const& val) OVERRIDE {
    int numThousands = size_ / 1000;
    bfs::path dir(dir_ / bfs::path("hg") / bfs::path(sdl::lexical_cast<std::string>(numThousands)));
    bfs::create_directories(dir);
    std::string name(sdl::lexical_cast<std::string>(size_) + ".hg");
    std::ofstream out((dir / bfs::path(name)).string().c_str());
    if (project_) {  // don't need the input labels (could even remove all labels)
      typedef Hypergraph::IMutableHypergraph<Arc> MHg;
      assert(val.first->isMutable());
      assert(val.second->isMutable());
      static_cast<MHg*>(val.first.get())->projectOutput();
      static_cast<MHg*>(val.second.get())->projectOutput();
    }
    out << *val.first << '\n' << "-----" << '\n' << *val.second << '\n';
    ++size_;
  }

  value_type operator[](TrainingDataIndex index)OVERRIDE { return value_type(); }

  void finish() OVERRIDE {
    bfs::path p(dir_ / bfs::path("size.txt"));
    std::ofstream out(p.string().c_str());
    out << size_;
  }

  void setFeatureWeights(FloatT const* featWeights, FeatureId numParams) OVERRIDE {}

  void setFeatureWeights(TrainingDataIndex begin, TrainingDataIndex end, FloatT const* featWeights,
                         FeatureId numParams) OVERRIDE {}

  std::size_t size() const OVERRIDE { return 0; }
  FeatureId getNumFeatures() OVERRIDE { return 0; }

  void setNumFeatures(FeatureId numParams) OVERRIDE {
    bfs::path p(dir_ / bfs::path("num-feats.txt"));
    std::ofstream out(p.string().c_str());
    out << numParams;
  }

 private:
  boost::filesystem::path dir_;
  std::size_t size_;
  bool project_;
};


}}

#endif
