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

    floating point comparison etc. helpers.
*/

#ifndef SDL_UTIL_MATH_HPP_
#define SDL_UTIL_MATH_HPP_
#pragma once

#include <cstring>
#include <cassert>
#include <sdl/Util/Constants.hpp>
#include <graehl/shared/epsilon.hpp>
#include <sdl/IntTypes.hpp>

namespace sdl {
namespace Util {

/**
   Subtracts two unsigned numbers, such that the result will not
   be negative; prevents frequent unsigned-numbers bug when performing
   a - b and b > a.
 */
template <class U>
U unsignedSubtract(U a, U b) {
  return b < a ? a - b : 0;
}

template <class U>
U unsignedDiff(U a, U b) {
  return b < a ? a - b : b - a;
}

/**
   Approximate equal, For floating-point comparisons
*/
template <class FloatT>
inline bool floatEqual(FloatT d1, FloatT d2, FloatT epsilon = 1e-7) {
  return graehl::within_epsilon_or_ieee_apart(d1, d2, epsilon);
  /// this will scale gracefully down toward 0 or toward +inf: anything really close will
  /// compare equal, even if epsilon is too small e.g. cost=1000 and
  /// epsilon=1e-5 - this would be a relative error of only 1e-8
}

template <class FloatT, class FloatT2>
inline bool approxGreaterOrEqual(FloatT d1, FloatT2 d2, FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 + epsilon > d2;
}

template <class FloatT, class FloatT2>
inline bool approxLessOrEqual(FloatT d1, FloatT2 d2, FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 < d2 + epsilon;
}

template <class FloatT, class FloatT2>
inline bool definitelyGreater(FloatT d1, FloatT2 d2, FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 > d2 + epsilon;
}

template <class FloatT, class FloatT2>
inline bool definitelyLess(FloatT d1, FloatT2 d2, FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 + epsilon < d2;
}

/**
   For use as predicate in std::equal, etc.
 */
template <class FloatT>
struct ApproxEqualFct {
  ApproxEqualFct(FloatT eps) : epsilon(eps) {}
  bool operator()(FloatT f1, FloatT f2) const { return floatEqual(f1, f2, epsilon); }
  FloatT epsilon;
};

/**
   Tests if the keys in map are equal and the values are
   approximately equal. For use as predicate in std::equal, etc.
 */
template <class MapT>
struct ApproxEqualMapFct {
  typedef typename MapT::value_type ValueT;  // i.e., key/value pair
  typedef typename MapT::mapped_type MappedT;  // i.e., value
  ApproxEqualMapFct(MappedT eps) : epsilon(eps) {}
  bool operator()(ValueT const& pair1, ValueT const& pair2) const {
    return pair1.first == pair2.first && floatEqual(pair1.second, pair2.second, epsilon);
  }
  MappedT epsilon;
};


template <class Float>
struct BiggerFloat {
  typedef double type;
};

template <>
struct BiggerFloat<double> {
  typedef long double type;
};

template <class Float>
struct OtherFloat {
  typedef double type;
};

template <>
struct OtherFloat<double> {
  typedef float type;
};

template <class T>
T signbit(T x) {
  return x & (1 << (sizeof(T) * 8 - 1));
}


inline int8 signbit(int8 x) {
  return x & (1 << 7);
}

inline int16 signbit(int16 x) {
  return x & (1 << 15);
}

inline int32 signbit(int32 x) {
  return x & (1 << 31);
}

inline int64 signbit(int64 x) {
  return x & ((int64)1 << 63);
}

inline int32 signbit(float x) {
  assert(sizeof(float) == sizeof(int32));
  int32 y;
  std::memcpy(&y, &x, sizeof(int32));
  // because reinterpret_cast is not technically allowed by std. memcpy should
  // be optimized away; if it weren't we'd want to just grab 1 byte (first or
  // last depending on little/big endian)
  return signbit(y);
}

inline int64 signbit(double x) {
  assert(sizeof(double) == sizeof(int32));
  int64 y;
  std::memcpy(&y, &x, sizeof(int64));
  return signbit(y);
}

template <class T>
inline int sgn(T x) {
  return signbit(x) ? -1 : 1;
}


}}

#endif
