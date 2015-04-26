// Copyright 2014 SDL plc
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
   Functions to multiply maps values by a scalar.

   \author Markus Dreyer
*/

#ifndef SDL_UTIL_MULTIPLIED_MAP_H_
#define SDL_UTIL_MULTIPLIED_MAP_H_
#pragma once

#include <boost/iterator/transform_iterator.hpp>
#include <boost/functional.hpp>

namespace sdl {
namespace Util {

/**
   Function object that takes a pair and multiplies the second
   element by a factor.
*/
template<class T, class TimesFct>
struct Multiply2nd : public std::unary_function<T, T> {
  typedef typename T::second_type FloatT;
  FloatT factor_;
  TimesFct times_fct_;
  Multiply2nd(FloatT f, TimesFct t) : factor_(f), times_fct_(t) {}
  T operator()(const T& f) const {
    return std::make_pair(f.first, times_fct_(f.second, factor_));
  }
};

/**
   A wrapper for std::map that lazily multiplies the map by a
   constant factor.
*/
template<class Map, class TimesFct>
class MultipliedMap {
 public:
  typedef typename Map::value_type value_type;   // type of key/value pair
  typedef typename Map::mapped_type mapped_type; // type of contained object
 private:
  typedef Multiply2nd<value_type, TimesFct> MultiplyValue;
 public:
  typedef boost::transform_iterator<MultiplyValue,
                                    typename Map::const_iterator> const_iterator;
  MultipliedMap(const Map& m,
                TimesFct times_fct,
                mapped_type factor)
      : map_(m), times_fct_(times_fct), factor_(factor) {}
  const_iterator begin() const {
    return boost::make_transform_iterator(map_.begin(), MultiplyValue(factor_, times_fct_));
  }
  const_iterator end() const {
    return boost::make_transform_iterator(map_.end(), MultiplyValue(factor_, times_fct_));
  }
 private:
  const Map& map_;
  TimesFct times_fct_;
  mapped_type factor_;
};

}}

#endif
