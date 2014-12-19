#ifndef HYP__HYPERGRAPH_CONVERTWEIGHT_HPP
#define HYP__HYPERGRAPH_CONVERTWEIGHT_HPP
#pragma once

namespace sdl {
namespace Hypergraph {

/**
   This can be specialized for more complicated weights, e.g.,
   FeatureWeight.
 */
template<class FromWeight, class ToWeight>
ToWeight convertWeight(FromWeight const& fromWeight) {
  return ToWeight(fromWeight.getValue());
}

/**
   This can be specialized for other weights, see BlockWeight,
   for example.
 */
template<class FromW, class ToW>
struct WeightConverter {
  typedef ToW Weight;
  WeightConverter(FromW const& from, ToW& to) {
    to = ToW(from.getValue());
  }
};

}}

#endif
