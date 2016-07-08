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

    dot product and similar operations.
*/

#ifndef SDL_DOT_PRODUCT_JG20121114_HPP
#define SDL_DOT_PRODUCT_JG20121114_HPP
#pragma once

#include <sdl/Util/Map.hpp>


namespace sdl {
namespace Util {

template <class Result = double>
struct Times {
  typedef Result result_type;
  template <class A, class B>
  result_type operator()(A const& a, B const& b) const {
    return a * b;
  }
};

template <class Result = double>
struct DotProductCombine {
  typedef Result result_type;
  static inline result_type zero() { return Result(); }
  void plusBy(result_type const& delta, result_type& sum) const { sum += delta; }
  template <class A, class B>
  result_type operator()(A const& a, B const& b) const {
    return a * b;
  }
};


// there's no optimization for sorted Map case, nor do i think there should be. if things are that dense, use
// vectors not maps

/**
   call dotResult(map[i->first], i->second) for range of pairs [i, end)
*/
template <class Map, class PairIter, class DotProductResult>
void intersectValues(Map const& map, PairIter i, PairIter end, DotProductResult& dotResult) {
  for (; i != end; ++i) {
    typename Map::const_iterator j = map.find(i->first);
    if (j != map.end()) dotResult(j->second, i->second);
  }
}

/**
   call dotResult(i->second, map[i->first]) for range of pairs [i, end) - same as
   above but swaps order of dotResult args (which may have different semantics
   or types)
*/
template <class Map, class PairIter, class DotProductResult>
void intersectValues(PairIter i, PairIter end, Map const& map, DotProductResult& dotResult) {
  for (; i != end; ++i) {
    typename Map::const_iterator j = map.find(i->first);
    if (j != map.end()) dotResult(i->second, j->second);
  }
}

/**
   call dotResult(map[key], map2[key]) for key in their intersection.
*/
template <class Map, class Map2, class DotProductResult>
void intersectValues(Map const& map, Map2 const& map2, DotProductResult& dotResult) {
  if (map.size() < map2.size())
    intersectValues(map.begin(), map.end(), map2, dotResult);
  else
    intersectValues(map, map2.begin(), map2.end(), dotResult);
}

template <class Result = double>
struct DotProductResult : DotProductCombine<Result> {
  typedef DotProductCombine<Result> Base;
  Result result;
  DotProductResult() : result(Base::zero()) {}
  template <class Map, class Map2>
  DotProductResult(Map const& map, Map2 const& map2) : result(Base::zero()) {
    intersectValues(map, map2, *this);
  }
  template <class A, class B>
  void operator()(A const& a, B const& b) {
    Base::plusBy(Base::operator()(a, b), result);
  }
};

/**
   return sum {map[key]*map2[key]} for key in their intersection.
*/
template <class Map, class Map2>
double dotProduct(Map const& map, Map const& map2) {
  return DotProductResult<double>(map, map2).result;
}


/**
   call dotResult(i->second, i->second) for range [i, end)
*/
template <class PairIter, class DotProductResult>
void selfIntersectValues(PairIter i, PairIter end, DotProductResult& dotResult) {
  for (; i != end; ++i) dotResult(i->second, i->second);  // TODO: iterator_traits value_type second_type?
}

/// \return dotProduct(map, map) but faster
template <class Map>
double sumSquares(Map const& map) {
  DotProductResult<double> dotter;
  selfIntersectValues(map, dotter);
  return dotter.result;
}


}}

#endif
