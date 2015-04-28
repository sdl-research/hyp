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
