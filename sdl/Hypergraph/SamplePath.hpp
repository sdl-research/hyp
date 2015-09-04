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
/** \file

    randomly sample a high-probability path.
*/


#ifndef HYP__HYPERGRAPH_SAMPLE_PATH_HPP
#define HYP__HYPERGRAPH_SAMPLE_PATH_HPP
#pragma once

#include <ctime>
#include <cstdlib>
#include <queue>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/InsideAlgorithm.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/LogHelper.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Samples one of the incoming arcs that point into the
   specified state.
*/

template <typename Arc>
class Sampler {
 public:
  Sampler(unsigned long seed = time(0)) { srand(seed); }

  virtual ~Sampler() {}

  virtual void initialize(IHypergraph<Arc> const& hg) {}

  virtual Arc* sampleArc(IHypergraph<Arc> const& hg, StateId s) const = 0;
};

template <typename Arc>
class UniformInArcSampler : public Sampler<Arc> {
 public:
  typedef typename Arc::Weight Weight;
  UniformInArcSampler(unsigned long seed = time(0)) : Sampler<Arc>(seed) {}

  virtual Arc* sampleArc(IHypergraph<Arc> const& hg, StateId s) const {
    std::size_t numArcs = hg.numInArcs(s);
    if (numArcs == 0) {
      return NULL;
    }
    double r = rand() / (RAND_MAX + 1.0);
    ArcId arcid = static_cast<ArcId>(r * numArcs);
    return hg.inArc(s, arcid);
  }
};

template <typename Arc>
class ProbabilityArcSampler : public Sampler<Arc> {
 public:
  typedef typename Arc::Weight Weight;
  ProbabilityArcSampler(unsigned long seed = time(0)) : Sampler<Arc>(seed) {}

  virtual void initialize(IHypergraph<Arc> const& hg) { insideAlgorithm(hg, &insideDistances_); }
  virtual Arc* sampleArc(IHypergraph<Arc> const& hg, StateId s) const {
    std::size_t numArcs = hg.numInArcs(s);
    if (numArcs == 0) {
      return NULL;
    }
    if (numArcs == 1) {
      return hg.inArc(s, 0);
    }

    Weight totalWeight = Weight::zero();
    std::vector<Weight> weights;
    std::vector<Arc*> arcs;
    forall (ArcId arcid, hg.inArcIds(s)) {
      Arc* arc = hg.inArc(s, arcid);
      Weight w = arc->weight();
      forall (StateId tailId, arc->tails()) {
        if (!hg.hasLexicalLabel(tailId)) {
          timesBy(insideDistances_[tailId], w);
        }
      }
      plusBy(w, totalWeight);
      arcs.push_back(arc);
      weights.push_back(w);
    }

    double random = rand() % RAND_MAX / static_cast<double>(RAND_MAX) * exp(-totalWeight.getValue());
    int i = 0;
    Weight weightSum = weights[0];
    while (exp(-weightSum.getValue()) < random) {
      weightSum = Hypergraph::plus(weightSum, weights[++i]);
    }
    return arcs[i];
  }

 private:
  boost::ptr_vector<Weight> insideDistances_;
};


// TODO: Add other sample functors: (1) Local: Sample in proportion to
// the weights of the incoming arcs, (2) Global: Sample in proportion
// to the forward (i.e., inside) weights

/**
   Samples a path from hg, starting from the final state.
*/
template <class Arc>
void samplePath(IHypergraph<Arc> const& hg, Sampler<Arc>& sampler, IMutableHypergraph<Arc>* result) {
  if (!(hg.properties() & kStoreInArcs)) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "HG must store incoming arcs");
  }
  result->setVocabulary(hg.getVocabulary());
  sampler.initialize(hg);
  std::queue<StateId> queue;
  std::set<StateId> onQueue;
  StateId finalStateId = hg.final();
  queue.push(finalStateId);
  onQueue.insert(finalStateId);
  result->addStateId(finalStateId, hg.inputLabel(finalStateId), hg.outputLabel(finalStateId));

  while (!queue.empty()) {
    StateId hgStateId = queue.front();
    queue.pop();
    if (hg.start() == hgStateId) {
      result->setStart(hgStateId);
    }

    if (Arc* arc = sampler.sampleArc(hg, hgStateId)) {
      forall (StateId s, arc->tails()) {
        if (onQueue.insert(s).second) {
          result->addStateId(s, hg.labelPair(s));
          queue.push(s);
        }
      }
      result->addArc(new Arc(*arc));
    }
  }
  result->setFinal(finalStateId);
  SDL_DEBUG(Hypergraph.samplePath, "Result: " << *result);
}


}}

#endif
