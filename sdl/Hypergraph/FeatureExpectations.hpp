








#include <cassert>
#include <cmath>
#include <boost/ptr_container/ptr_vector.hpp>















namespace Hypergraph {

namespace {
template<class Map>
inline void printMapInfo(Map const& map, int n = 10) {

            "Map with " << map.size() << " feature expectations. First "<< n << " values:");
  typedef typename Map::const_iterator Iter;
  for (Iter it = map.begin(); it != map.end() && n; ++it, --n) {

              "  expectation[" << it->first << "]=" << it->second);
  }
}
}

/**




*/
template<class FloatT, class MapT>
struct AccumulateExpectedValuesFct {

  typedef LogWeightTpl<FloatT> LogW;
  typedef MapT Map;

  AccumulateExpectedValuesFct(boost::ptr_vector<LogW> const& insideWeights,
                              boost::ptr_vector<LogW> const& outsideWeights,
                              Map* pResultMap)
      : insideWeights_(insideWeights),



  /**


  */
  void operator()(ArcTpl<FeatureWeightTpl<FloatT, Map, Expectation> > const* arc) {
    FloatT posteriorArcWeight = computeInsideTimesOutside(arc).getValue();
    typedef typename Map::value_type MapValueType;



  }

  /**


  */
  void operator()(ArcTpl<FeatureWeightTpl<FloatT, Map, TakeMin> > const* arc) {
    FloatT posteriorArcWeight =


              "Accumulate feature expectations for " << *arc
              << " with posterior weight " << posteriorArcWeight);
    typedef typename Map::value_type MapValueType;



    if (Util::isDebugBuild())
      printMapInfo(*pResultMap_);
  }

 private:
  template<class Arc>
  LogWeightTpl<FloatT> computeInsideTimesOutside(Arc const& arc) {







    }
    return result;
  }




};


/**






*/
template<class Arc>
typename Arc::Weight::FloatT
computeFeatureExpectations(IHypergraph<Arc> const& hg,
                           typename Arc::Weight::Map* pResultMap) {

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




  // Normalize and remove neglog of feature expectations:
  typedef typename Arc::Weight::Map::value_type ValT;

    val.second = exp(-val.second + pathsSum.getValue());
  }
  if (Util::isDebugBuild())
    printMapInfo(*pResultMap);

  return pathsSum.getValue();
}




#endif
