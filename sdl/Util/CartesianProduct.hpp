/** \file

    cross product of sets.
*/

#ifndef CARTESIANPRODUCT_JG_2014_12_18_HPP
#define CARTESIANPRODUCT_JG_2014_12_18_HPP
#pragma once


#include <vector>
#include <iostream>
#include <iterator>

namespace sdl {
namespace Util {

namespace detail {

template <class VectorVector>
void cartesianProduct(typename VectorVector::const_iterator curr, typename VectorVector::const_iterator end,
                      typename VectorVector::value_type& tmpVec,  // current result
                      VectorVector* result)  // final result
{
  if (curr == end) {
    result->push_back(tmpVec);
    return;
  }
  typedef typename VectorVector::value_type Vector;
  const Vector& currVec = *curr;
  for (typename Vector::const_iterator it = currVec.begin(); it != currVec.end(); it++) {
    tmpVec.push_back(*it);
    cartesianProduct(curr + 1, end, tmpVec, result);
    tmpVec.pop_back();
  }
}
}

/**
   Cartesian product of vector of vectors
*/
template <class VectorVector>
void cartesianProduct(VectorVector const& input, VectorVector* output) {
  typename VectorVector::value_type outputTemp;
  using detail::cartesianProduct;
  cartesianProduct(input.begin(), input.end(), outputTemp, output);
}


}}

#endif
