/** \file
   feature weight concept: W has nested typedef void W::IsFeatureWeight
*/

#ifndef HYP__HYPERGRAPH_ISFEATUREWEIGHT_HPP
#define HYP__HYPERGRAPH_ISFEATUREWEIGHT_HPP
#pragma once


#include <boost/type_traits/integral_constant.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Interface for is_feature_weight consistent with boost::type_traits libraries.
   If a class is of type FeatureWeight, the struct will inherit from true_type.
   Please see boost::type_traits documentation for more details.
   http://www.boost.org/doc/libs/1_49_0/libs/type_traits/doc/html/index.html
**/

template <class Weight, class Enable = void>
struct IsFeatureWeight
    : public boost::false_type {};

template <class Weight>
struct IsFeatureWeight<Weight, typename Weight::IsFeatureWeight>
    : public boost::true_type {};

template <class Weight>
bool isFeatureWeight(Weight* p = 0) {
  return IsFeatureWeight<Weight>::value;
}


/**
   Distance weight: if w==Weight(w.getValue()) e.g. Log, Viterbi wt
*/

template <class Weight, class Enable = void>
struct IsDistanceWeight
    : public boost::true_type {};

template <class Weight>
struct IsDistanceWeight<Weight, typename Weight::IsFeatureWeight>
    : public boost::false_type {};

template <class Weight>
bool isDistanceWeight(Weight* p = 0) {
  return IsDistanceWeight<Weight>::value;
}


}}

#endif
