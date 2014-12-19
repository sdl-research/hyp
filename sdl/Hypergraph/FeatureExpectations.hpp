/** \file

    compute expectation of feature values over probability distribution (weight values are logprobs).
*/

#ifndef HYP__HYPERGRAPH_FEATUREEXPECTATIONS_HPP
#define HYP__HYPERGRAPH_FEATUREEXPECTATIONS_HPP
#pragma once

#include <cassert>
#include <cmath>
#include <boost/ptr_container/ptr_vector.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/FeatureWeightTpl.hpp>
#include <sdl/Hypergraph/WeightsFwdDecls.hpp>
#include <sdl/Hypergraph/InsideAlgorithm.hpp>
#include <sdl/Hypergraph/OutsideAlgorithm.hpp>
#include <sdl/Hypergraph/CastHypergraph.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/LogMath.hpp>
#include <sdl/Util/Map.hpp>
#include <sdl/Util/IsDebugBuild.hpp>

namespace sdl {
namespace Hypergraph {

namespace {
template<class Map>
inline void printMapInfo(Map const& map, int n = 10) {
  SDL_DEBUG(Hypergraph.FeatureExpectations,
            "Map with " << map.size() << " feature expectations. First "<< n << " values:");
  typedef typename Map::const_iterator Iter;
  for (Iter it = map.begin(); it != map.end() && n; ++it, --n) {
    SDL_DEBUG(Hypergraph.FeatureExpectations,
              "  expectation[" << it->first << "]=" << it->second);
  }
}
}

/**
   Gets called for each arc in the hypergraph and adds up the
   expected feature values (as neglog numbers) that it finds on
   them. Works correctly for both types of feature arcs (viterbi and
   expectation).
*/
template<class FloatT, class MapT>
struct AccumulateExpectedValuesFct {

  typedef LogWeightTpl<FloatT> LogW;
  typedef MapT Map;

  AccumulateExpectedValuesFct(boost::ptr_vector<LogW> const& insideWeights,
                              boost::ptr_vector<LogW> const& outsideWeights,
                              Map* pResultMap)
      : insideWeights_(insideWeights),
        outsideWeights_(outsideWeights),
        pResultMap_(pResultMap) {}

  /**
     Accumulates feature expectations found on a feature
     weight arc. (Gets called with *expectation* feature weights.)
  */
  void operator()(ArcTpl<FeatureWeightTpl<FloatT, Map, Expectation> > const* arc) {
    FloatT posteriorArcWeight = computeInsideTimesOutside(arc).getValue();
    typedef typename Map::value_type MapValueType;
    Util::NeglogPlusFct<FloatT> logPlusBy;
    forall (MapValueType const& idValue, arc->weight())
        updateBy(logPlusBy, *pResultMap_, idValue.first, posteriorArcWeight + idValue.second); // times prob
  }

  /**
     Accumulates feature expectations found on a feature
     weight arc. (Gets called with *viterbi* feature weights.)
  */
  void operator()(ArcTpl<FeatureWeightTpl<FloatT, Map, TakeMin> > const* arc) {
    FloatT posteriorArcWeight =
        computeInsideTimesOutside(arc).getValue() + arc->weight().getValue(); // unlike Expectation, which already includes this factor in the feature values
    SDL_DEBUG(Hypergraph.FeatureExpectations,
              "Accumulate feature expectations for " << *arc
              << " with posterior weight " << posteriorArcWeight);
    typedef typename Map::value_type MapValueType;
    Util::NeglogPlusFct<FloatT> logPlusBy;
    forall (MapValueType const& idValue, arc->weight())
        updateBy(logPlusBy, *pResultMap_, idValue.first, posteriorArcWeight - log(idValue.second)); // times prob (which was in linear space, not -log, because of TakeMin aka FeatureWeight)
    if (Util::isDebugBuild())
      printMapInfo(*pResultMap_);
  }

 private:
  template<class Arc>
  LogWeightTpl<FloatT> computeInsideTimesOutside(Arc const& arc) {
    assert(outsideWeights_.size() > arc->head());
    LogW result(outsideWeights_[arc->head()]);
    StateIdContainer const& tails = arc->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
      StateId const tailId = *i;
      assert(insideWeights_.size() > tailId);
      Hypergraph::timesBy(insideWeights_[tailId], result);
    }
    return result;
  }

  Map* pResultMap_;
  boost::ptr_vector<LogW> insideWeights_;
  boost::ptr_vector<LogW> outsideWeights_;
};


/**
   Computes the expected value of each feature in the
   hypergraph.

   \param pResultMap A pointer to map that will hold the result.

   \return Sum of all path weights (used for normalization)
*/
template<class Arc>
typename Arc::Weight::FloatT
computeFeatureExpectations(IHypergraph<Arc> const& hg,
                           typename Arc::Weight::Map* pResultMap) {
  SDL_TRACE(Hypergraph, "computeFeatureExpectations");
  typedef typename Arc::Weight Weight;
  typedef typename Weight::Map Map;
  typedef typename Weight::FloatT FloatT;

  // Cast orig arcs to log arcs:
  typedef LogWeightTpl<FloatT> LogW;
  typedef ArcTpl<LogW> LogArc;
  CastHypergraph<Arc, LogArc> logWeightHg(hg);

  // Run inside/outside algorithm in log semiring:
  boost::ptr_vector<LogW> insideWeights;
  boost::ptr_vector<LogW> outsideWeights;
  insideAlgorithm(logWeightHg, &insideWeights);
  outsideAlgorithm(logWeightHg, insideWeights, &outsideWeights);

  AccumulateExpectedValuesFct<FloatT, Map> fct(insideWeights, outsideWeights, pResultMap);
  hg.visitArcs(fct);

  // Negative log of sum over all paths (used for global
  // normalization):
  assert(hg.final() != kNoState);
  assert(insideWeights.size() > hg.final());
  LogW const& pathsSum = insideWeights[hg.final()];

  // Normalize and remove neglog of feature expectations:
  typedef typename Arc::Weight::Map::value_type ValT;
  forall (ValT& val, *pResultMap) {
    val.second = exp(-val.second + pathsSum.getValue());
  }
  if (Util::isDebugBuild())
    printMapInfo(*pResultMap);

  return pathsSum.getValue();
}


}}

#endif
