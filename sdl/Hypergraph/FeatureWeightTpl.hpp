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
/**
   \file

   A weight that holds features and can behave like an
   expectation weight or a viterbi weight with features.

   TODO (benchmarking): instead of using a null pointer for an empty map, use an actual empty map

   TODO: test performance without copy-on-write (sometimes it's faster)

   TODO: try vector of pair<id, weight> for use up to size N, then switch to
   something else.  (maybe vector always faster in practice, can sort before doing
   times/plus operations, or on every insertion). also, small_vector to handle
    1 or 2 features (like alignments)

*/

#ifndef HYP__HYPERGRAPH_FEATUREWEIGHT_TPL_HPP
#define HYP__HYPERGRAPH_FEATUREWEIGHT_TPL_HPP
#pragma once

#include <sdl/Hypergraph/WeightUtil.hpp>

#include <functional>
#include <boost/make_shared.hpp>

#include <sdl/Util/Math.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/LogMath.hpp>
#include <sdl/Util/Constants.hpp>
#include <sdl/Util/DefaultPrintRange.hpp>

#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/FeatureIdRange.hpp>
#include <sdl/Exception.hpp>

namespace sdl {
namespace Hypergraph {


/**
   Sum policy for (default) viterbi-like feature weight, in
   which feature values are stored as-is (not taking the neg. log or
   storing expectations).
*/
struct TakeMin {

  /**
     Returns the value of a non-existent feature (namely, 0.0).
  */
  template <class FloatT>
  static FloatT getZeroFeatureValue() {
    return static_cast<FloatT>(0.0);
  }
};


/**
   Sum policy for expectations feature weight, in which feature
   values are stored as-is the neg. log of their expected value.
*/
struct Expectation {

  /**
     Returns the neg. log expectation value of a non-existent
     feature (namely, -log(0.0)).
  */
  template <class FloatT>
  static FloatT getZeroFeatureValue() {
    return FloatLimits<FloatT>::posInfinity;
  }
};


/**
   A weight that holds features. It has a weight component and
   a sparse vector of feature IDs with associated features values.

   \tparam SumPolicy The policy that determines if the feature weight
   should be an expectation weight (see Eisner 2004, expectation
   semiring), or a simple feature weight that behaves like a Viterbi
   weight that carries features. Our ExpectationWeight is a typedef on
   this weight with Expectation as SumPolicy.

   For Expectation, each feature value should be the negative log of
   the non-log value (i.e., the arc weight, e.g., a probability) times
   the feature value.
*/
template <class T, class MapT, class SumPolicy = TakeMin>
class FeatureWeightTpl : public FloatWeightTpl<T> {
 public:
  typedef T FloatT;
  typedef void IsFeatureWeight;  // used by IsFeatureWeight.hpp, FeatureWeightTpl.hpp, AlignmentFeatures.hpp

  typedef void HasPlusBy;  // used by WeightUtil.hpp Hypergraph::plusBy
  typedef void HasTimesBy;

  typedef MapT Map;

  // Map concept type names
  typedef typename Map::key_type key_type;
  typedef typename Map::mapped_type mapped_type;
  typedef typename Map::value_type value_type;

  // more semantic type names. with the T at the end only to distinguish from Hypergraph::FeatureId/Value
  typedef key_type FeatureIdT;
  typedef mapped_type FeatureValueT;

  typedef typename Map::const_iterator const_iterator;
  typedef typename Map::iterator iterator;

 private:
  typedef FloatWeightTpl<FloatT> Base;
  typedef FeatureWeightTpl<FloatT, MapT, SumPolicy> Self;

 public:
  void set(std::string const&);
#if __cplusplus >= 201103L
  FeatureWeightTpl() = default;
#else
  FeatureWeightTpl() {}
#endif

  /** copy ctor that immediately makes a unique writable pointer */
  FeatureWeightTpl(FeatureWeightTpl const& cpfrom, bool)
      : Base(cpfrom), pMap_(sdl::make_shared<Map>(cpfrom.features())) {}

  /// copy, operator=(), C++11 moves are all default

  FeatureWeightTpl(FloatT weight, shared_ptr<Map> const& pMap) : Base(weight), pMap_(pMap) {}

  explicit FeatureWeightTpl(FloatT weight) : Base(weight) {}

  explicit FeatureWeightTpl(typename Base::DoubleT weight) : Base(weight) {}

  explicit FeatureWeightTpl(int weight) : Base(weight) {}

  static inline Self one() { return Self(static_cast<FloatT>(0.0)); }

  static inline Self zero() { return Self(FloatLimits<FloatT>::posInfinity); }

  typedef void HasIsZero;
  bool isZero() const { return this->value_ == FloatLimits<FloatT>::posInfinity; }

  void setZero() {
    this->value_ = FloatLimits<FloatT>::posInfinity;
    pMap_.reset();
  }
  friend inline void setZero(FeatureWeightTpl& x) { x.setZero(); }

  void setOne() {
    this->value_ = (FloatT)0;
    pMap_.reset();
  }
  friend inline void setOne(FeatureWeightTpl& x) { x.setOne(); }

  typedef void HasIsOne;
  bool isOne() const { return !this->value_ && empty(); }

  /**
     Returns const map, which may be shared with other weight
     objects.
  */
  Map const& features() const {
    if (!pMap_) return staticEmptyMap;
    return *pMap_;
  }

  Map const* maybeFeatures() const { return pMap_.get(); }

  /**
     Returns non-const map, which is never shared with other
     weight objects (cloned here if necessary).
  */
  Map& featuresWrite() {
    ownMap();
    return *pMap_;
  }

  /**
     Inserts features with their values into the features map.
  */
  template <class InputIterator>
  void insertRange(InputIterator first, InputIterator last) {
    featuresWrite().insert(first, last);
  }

  /**
     Inserts a feature with its value into the features map.
  */
  void insert(key_type const& id, mapped_type const& value) { featuresWrite().insert(value_type(id, value)); }

  /**
     Updates an existing feature with new value
  */
  void update(key_type const& id, mapped_type const& value) { featuresWrite()[id] = value; }

  /**
     Inserts a feature with its value into the features map.
  */
  void insert(value_type const& pair) { featuresWrite().insert(pair); }

  bool contains(key_type id) const { return pMap_ && Util::contains(*pMap_, id); }

  /**
     Returns value of feature id. If feature is not stored then
     SumPolicy::getZeroFeatureValue() is returned.
  */
  mapped_type operator[](key_type id) const {
    if (!pMap_) return SumPolicy::template getZeroFeatureValue<mapped_type>();
    Map const& map = *pMap_;
    const_iterator iter = map.find(id);
    return iter == map.end() ? SumPolicy::template getZeroFeatureValue<mapped_type>() : iter->second;
  }

  /**
     precondition: contains(id).

     \return value of existing feature id (may be 0)
  */
  mapped_type getContainedValue(key_type id) const {
    assert(contains(id));
    return *pMap_->find(id);
  }

  /**
     call visitFeature(value_type(id, val)) for all beginId<=id<endId with a val
     defined. (may include explicit 0 values - we do nothing to guarantee
     sparseness of feature vectors)
  */
  template <class Fct>
  void visitFeatureRange(key_type beginId, key_type endId, Fct const& visitFeature) const {
    assert(endId >= beginId);
    if (!pMap_) return;
    MapT const& map = *pMap_;
    typedef const_iterator Iter;
    for (Iter lower = map.lower_bound(beginId), end = map.end(); lower != end; ++lower) {
      // used to iterate until upper_bound but that search is superfluous - we can just check value as we
      // iterate
      if (lower->first >= endId) break;
      visitFeature(*lower);
    }
  }

  template <class Visit>
  void visitFeatureRange(FeatureIdRange const& range, Visit const& visit) const {
    visitFeatureRange(range.begin, range.end, visit);
  }

  struct AddFeature {
    Map& map;
    AddFeature(Self& self) : map(self.featuresWrite()) {}
   void operator()(value_type const& val) const { map.insert(val); }
  };

  const_iterator begin() const { return features().begin(); }

  const_iterator end() const { return features().end(); }

  iterator begin() { return featuresWrite().begin(); }

  iterator end() { return featuresWrite().end(); }

  bool operator==(Self const& other) const {
    if (this->value_ != other.value_) return false;
    if (empty()) return other.empty();
    if (!other.pMap_) return false;
    return *pMap_ == *other.pMap_;
  }

  bool operator!=(Self const& other) const { return !(*this == other); }

  /**
     Calls empty() on the features map.
  */
  bool empty() const { return !pMap_ || pMap_->empty(); }

  /**
     Calls size() on the features map.
  */
  std::size_t size() const { return pMap_ ? pMap_->size() : 0; }

  /**
     Removes all features (weight value is unaffected)
  */
  void removeFeatures() { pMap_.reset(); }

  DEFINE_OPENFST_COMPAT_FUNCTIONS(Feature)

  void setFeatures(shared_ptr<Map> const& pMap) { pMap_ = pMap; }
#if __cplusplus >= 201103L
  void setFeatures(shared_ptr<Map>&& pMap) { pMap_ = std::move(pMap); }
#endif

  shared_ptr<Map> const& featuresPtr() const { return pMap_; }

  /**
     Multiplies this weight by another weight (using semiring
     times). This is the version for TakeMin policy.
  */
  void timesBy(FeatureWeightTpl<FloatT, MapT, TakeMin> const& b) {
    // TODO: check that INF+x=x+INF=INF (or else short circuit on zero())
    checkSumPolicy<TakeMin>();  // this weight must also use TakeMin
    this->value_ += b.value_;
    timesFeaturesBy(b);
  }

  void divideBy(FeatureWeightTpl<FloatT, MapT, TakeMin> const& b) {
    this->value_ -= b.value_;
    divideFeaturesBy(b);
  }


  /**
     Adds another weight to this weight (using semiring
     plus). This is the version for TakeMin policy.
  */
  void plusBy(FeatureWeightTpl<FloatT, MapT, TakeMin> const& b) {
    checkSumPolicy<TakeMin>();  // this weight must also use TakeMin
    if (!b.isZero() && this->value_ > b.value_) {
      *this = b;
    }
  }

  // Versions for Expectation policy. Defined in
  // ExpectationWeight.hpp:
  void timesBy(FeatureWeightTpl<FloatT, MapT, Expectation> const& b);
  void plusBy(FeatureWeightTpl<FloatT, MapT, Expectation> const& b);
  /**
     blend two feature weights together with something other than equal
     contribution (of existing feature weights).
  */
  void timesByScaled(FeatureWeightTpl<FloatT, MapT, TakeMin> const& b, FloatT scale) {
    this->value_ += scale * b.value_;
    timesFeaturesBy(b);
  }
  void timesByScaled(FeatureWeightTpl<FloatT, MapT, Expectation> const& b, FloatT) { timesBy(b); }

  // Feature map may be shared with other FeatureWeightTpl
  // objects. Using copy-on-write semantics.
  shared_ptr<Map> pMap_;

 private:
  /**
     Multiplies this weight by another weight (using semiring
     times). This is the version for TakeMin policy.
  */
  void timesFeaturesBy(FeatureWeightTpl<FloatT, MapT, TakeMin> const& b) {
    if (b.empty()) {
    } else if (empty()) {
      pMap_ = b.pMap_;
    } else if (pMap_ == b.pMap_) {
      ownMap();
      for (iterator i = pMap_->begin(), end = pMap_->end(); i != end; ++i) i->second *= 2;
    } else {
      Map& map = featuresWrite();
      for (const_iterator i = b.pMap_->begin(), end = b.pMap_->end(); i != end; ++i)
        map[i->first] += i->second;
    }
  }

  void divideFeaturesBy(FeatureWeightTpl<FloatT, MapT, TakeMin> const& b) {
    MapT const* map2 = b.pMap_.get();
    if (map2) {
      MapT& out = featuresWrite();
      for (typename MapT::const_iterator i = map2->begin(), e = map2->end(); i != e; ++i)
        out[i->first] -= i->second;
    }
  }

  /**
     Copy-on-write technique: Feature map may be shared with
     other objects, unless this is called.
  */
  void ownMap() {
    if (!pMap_)
      pMap_ = sdl::make_shared<Map>();
    else if (!pMap_.unique())
      pMap_ = sdl::make_shared<Map>(*pMap_);
  }

  /**
     Uses BOOST_STATIC_ASSERT to give compile error if the
     SumPolicy of this class is not the same as the passed sum policy
     (OtherSumPolicy).
  */
  template <class OtherSumPolicy>
  void checkSumPolicy() {
    typedef boost::is_same<SumPolicy, OtherSumPolicy> SamePolicy;
    BOOST_STATIC_ASSERT(SamePolicy::value);
  }

  static Map const staticEmptyMap;


};  // end class


template <class FloatT, class MapT, class SumPolicy>
MapT const FeatureWeightTpl<FloatT, MapT, SumPolicy>::staticEmptyMap;

template <class FloatT, class MapT, class SumPolicy>
inline bool approxEqual(FeatureWeightTpl<FloatT, MapT, SumPolicy> const& w1,
                        FeatureWeightTpl<FloatT, MapT, SumPolicy> const& w2,
                        FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return Util::floatEqual(w1.getValue(), w2.getValue(), epsilon) && w1.size() == w2.size()
         && std::equal(w1.begin(), w1.end(), w2.begin(), Util::ApproxEqualMapFct<MapT>(epsilon));
}

// for Expectation instead of TakeMin, see ExpectationWeight.hpp
template <class FloatT, class MapT>
FeatureWeightTpl<FloatT, MapT, TakeMin> plus(FeatureWeightTpl<FloatT, MapT, TakeMin> const& w1,
                                             FeatureWeightTpl<FloatT, MapT, TakeMin> const& w2) {
  return w1.getValue() < w2.getValue() ? w1 : w2;
}

// for Expectation instead of TakeMin, see ExpectationWeight.hpp
template <class FloatT, class MapT>
FeatureWeightTpl<FloatT, MapT, TakeMin> times(FeatureWeightTpl<FloatT, MapT, TakeMin> const& w1,
                                              FeatureWeightTpl<FloatT, MapT, TakeMin> const& w2) {
  FeatureWeightTpl<FloatT, MapT, TakeMin> r(w1, true);
  r.timesBy(w2);
  return r;
}

template <class FloatT, class MapT>
inline FeatureWeightTpl<FloatT, MapT, TakeMin> divide(FeatureWeightTpl<FloatT, MapT, TakeMin> const& w1,
                                                      FeatureWeightTpl<FloatT, MapT, TakeMin> const& w2) {
  typedef FeatureWeightTpl<FloatT, MapT, TakeMin> FeatW;
  typedef typename MapT::mapped_type FeatValueT;
  FeatW result(w1);
  result.divideBy(w2);
  return result;
}

template <class FloatT, class MapT, class SumPolicy>
std::ostream& operator<<(std::ostream& out, FeatureWeightTpl<FloatT, MapT, SumPolicy> const& weight) {
  out << weight.getValue();
  if (!weight.empty()) Util::printRange(out, weight, Util::RangeSep(",", "[", "]"));
  return out;
}

/**
   Adds delta (default: 1) to value of feature id. Either insert(id, delta), or
   set (*this)[id] += delta. this is useful for computing count features in
   random order (e.g. sum over aligned source words) without first grouping
   results in a local map

*/
template <class FeatureWt>
inline void incrementFeatureValue(FeatureWt& wt, typename FeatureWt::FeatureIdT id,
                                  typename FeatureWt::FeatureValueT delta
                                  = typename FeatureWt::FeatureValueT(1.0)) {
  wt.featuresWrite()[id] += delta;
}

template <class Float, class Map>
FeatureWeightTpl<Float, Map, Expectation> divide(FeatureWeightTpl<Float, Map, Expectation> const& w1,
                                                 FeatureWeightTpl<Float, Map, Expectation> const&) {
  SDL_THROW_LOG(Hypergraph.FeatureWeightTpl, UnimplementedException, "TODO: divide ExpectationWeight?");
  return w1;
}

template <class FloatT, class MapT, class SumPolicy>
inline char const* weightName(FeatureWeightTpl<FloatT, MapT, SumPolicy>*) {
  return "Feature";
}


}}

#include <sdl/Hypergraph/src/FeatureWeight.ipp>

#endif
