/** \file

    floating point comparison etc. helpers.
*/

#ifndef SDL_UTIL_MATH_HPP_
#define SDL_UTIL_MATH_HPP_
#pragma once

#ifndef SDL_WEIGHT_AVOID_FLOAT_CAST_WARNING
#ifdef _MSC_VER
//because ms build has more warnings right now
# define SDL_WEIGHT_AVOID_FLOAT_CAST_WARNING 1
#else
# define SDL_WEIGHT_AVOID_FLOAT_CAST_WARNING 0
#endif
#endif

#include <sdl/Util/Constants.hpp>

namespace sdl {
namespace Util {

/**
   Subtracts two unsigned numbers, such that the result will not
   be negative; prevents frequent unsigned-numbers bug when performing
   a - b and b > a.
 */
template<class U>
U unsignedSubtract(U a, U b) {
  return b < a ? a - b : 0;
}

template<class U>
U unsignedDiff(U a, U b) {
  return b < a ? a - b : b - a;
}

template<class FloatT, class FloatT2>
inline bool floatEqual(FloatT d1, FloatT2 d2,
                       FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 <= d2 + epsilon && d2 <= d1 + epsilon;
}

template<class FloatT, class FloatT2>
inline bool approxGreaterOrEqual(FloatT d1, FloatT2 d2,
                                 FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 + epsilon > d2;
}

template<class FloatT, class FloatT2>
inline bool approxLessOrEqual(FloatT d1, FloatT2 d2,
                              FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 < d2 + epsilon;
}

template<class FloatT, class FloatT2>
inline bool definitelyGreater(FloatT d1, FloatT2 d2,
                              FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 > d2 + epsilon;
}

template<class FloatT, class FloatT2>
inline bool definitelyLess(FloatT d1, FloatT2 d2,
                           FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 + epsilon < d2;
}

/**
   For use as predicate in std::equal, etc.
 */
template<class FloatT>
struct ApproxEqualFct {
  ApproxEqualFct(FloatT eps) : epsilon(eps) {}
  bool operator()(FloatT f1, FloatT f2) const {
    return floatEqual(f1, f2, epsilon);
  }
  FloatT epsilon;
};

/**
   Tests if the keys in map are equal and the values are
   approximately equal. For use as predicate in std::equal, etc.
 */
template<class MapT>
struct ApproxEqualMapFct {
  typedef typename MapT::value_type ValueT;   // i.e., key/value pair
  typedef typename MapT::mapped_type MappedT; // i.e., value
  ApproxEqualMapFct(MappedT eps) : epsilon(eps) {}
  bool operator()(ValueT const& pair1,
                  ValueT const& pair2) const {
    return pair1.first == pair2.first
        && floatEqual(pair1.second, pair2.second, epsilon);
  }
  MappedT epsilon;
};


template <class Float>
struct BiggerFloat
{
  typedef double type;
};

template <>
struct BiggerFloat<double>
{
  typedef long double type;
};

template <class Float>
struct OtherFloat
{
  typedef double type;
};

template <>
struct OtherFloat<double>
{
  typedef float type;
};

template <typename T> int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

}}

#endif
