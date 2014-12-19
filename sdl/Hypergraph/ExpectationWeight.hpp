/**











   ExpectationWeight(cost=-log(prob), [1=-log(val1*prob)=-log(val1)+cost,...])
















   (the overall probability is also a float value that's -log(prob))

   so adding two ExpectationWeight just separately (-LogPlus) adds the probs and











































*/





#include <stdexcept>
#include <string>
#include <iostream>
#include <cassert>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>













namespace Hypergraph {


/// ExpectationWeight is a type of FeatureWeight, where we take the
/// sum of values rather than the min:







template<class FloatT, class MapT, class SumPolicy>
void FeatureWeightTpl<FloatT, MapT, SumPolicy>::plusBy(
    FeatureWeightTpl<FloatT, MapT, Expectation> const& b)
{
  checkSumPolicy<Expectation>();





































/**


*/

void FeatureWeightTpl<FloatT, MapT, SumPolicy>::timesBy(
    FeatureWeightTpl<FloatT, MapT, Expectation> const& b)

  checkSumPolicy<Expectation>();



































template<class FloatT, class MapT>
inline
FeatureWeightTpl<FloatT, MapT, Expectation> plus(
    FeatureWeightTpl<FloatT, MapT, Expectation> const& w1,
    FeatureWeightTpl<FloatT, MapT, Expectation> const& w2) {
  FeatureWeightTpl<FloatT, MapT, Expectation> w3(w1, true);
  w3.plusBy(w2);
  return w3;
}



template<class FloatT, class MapT>
inline
FeatureWeightTpl<FloatT, MapT, Expectation> times(
    FeatureWeightTpl<FloatT, MapT, Expectation> const& w1,
    FeatureWeightTpl<FloatT, MapT, Expectation> const& w2) {
  FeatureWeightTpl<FloatT, MapT, Expectation> w3(w1, true);
  w3.timesBy(w2);
  return w3;
}




#endif
