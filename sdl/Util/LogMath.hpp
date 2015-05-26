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

   add and subtract log probs.
*/

#ifndef SDL_UTIL_LOGMATH_HPP
#define SDL_UTIL_LOGMATH_HPP
#pragma once


#include <sdl/Util/Constants.hpp>
#include <sdl/Util/LogHelper.hpp>

/**
   you want to use log1p() for numbers very close to 1 that can’t be accurately
   represented directly — but if, and only if, your algorithm is already
   designed to compute and store them as offsets from 1 rather than storing them
   directly (i.e., as offsets from 0). If you are storing those inputs directly,
   you gain nothing from writing things as log1p(x – 1), because your algorithm
   has already lost the any extra precision of those inputs it might have had in
   computing and storing them that way. If x is already rounded to 1 rather than
   a conceptual representation of 1 + 1e-57 (where the “1 +” is implied and only
   the 1e-57 is stored in the variable in memory), then subtracting 1 from it
   gets you 0, not 1e-57, and using log1p is no help at all.

   (our usage of these to compute log(exp(a) + exp(b)) and log(exp(a) - exp(b)) qualifies)
*/
#if _MSC_VER
// #  include <amp_math.h>  // TODO: when we have MSVC2012
#include <boost/math/special_functions/log1p.hpp>
using boost::math::log1p;
// using Concurrency::precise_math::log1p; // TODO: when we have MSVC2012
#else
#include <math.h>
// have log1p from <math.h>
#endif

namespace sdl {
namespace Util {

static inline double logExp(double x) {
  using namespace boost::math;
  return log1p(exp(x));
}
static inline float logExp(float x) {
  using namespace boost::math;
  return (float)log1p(exp(x));
}

static inline double logExpMinus(double x) {
  using namespace boost::math;
  return log1p(-exp(x));
}
static inline float logExpMinus(float x) {
  using namespace boost::math;
  return (float)log1p(-exp(x));
}

template <class T>
inline T logPlus(T f1, T f2) {
  T d = f1 - f2;
  if (d > 0) {
    return f2 - logExp(-d);
  } else {
    return f1 - logExp(d);
  }
}

/**
   Functor that multiplies two negated log numbers and returns
   negated log number.

   \return <pre> -log( exp(-a) * exp(-b) ) </pre>
*/
template <class FloatT>
struct NeglogTimesFct {
  typedef FloatT result_type;
  typedef FloatT increment_type;
  FloatT operator()(FloatT const& a, FloatT const& b) const { return a + b; }
  void operator()(FloatT const& b, FloatT& a) const { a += b; }

  enum { kIsCommutative = 1 };
  static inline FloatT zeroPlus(FloatT const& b) { return b; }
  static inline FloatT zero() { return (FloatT)0; }
};

/**
   Functor that divides two negated log numbers and returns
   negated log number.

   \return <pre> -log( exp(-a) / exp(-b) ) </pre>
*/
template <class FloatT>
struct NeglogDivideFct {
  typedef FloatT result_type;
  typedef FloatT increment_type;
  FloatT operator()(FloatT const& a, FloatT const& b) const { return a - b; }
  void operator()(FloatT const& b, FloatT& a) const { a -= b; }

  enum { kIsCommutative = 0 };
  static inline FloatT zeroPlus(FloatT const& b) { return -b; }
  static inline FloatT zero() { return (FloatT)0; }
};

/**
   Functor that adds two negated log numbers and returns
   negated log number.

   \return <pre> -log( exp(-a) + exp(-b) ) </pre>
*/
template <class FloatT>
struct NeglogPlusFct {
  typedef FloatT result_type;
  typedef FloatT increment_type;
  template <class Map>
  void addToMap(Map& map, typename Map::key_type const& key, FloatT value) const {
    std::pair<typename Map::iterator, bool> iNew = map.insert(typename Map::value_type(key, value));
    if (iNew.second) return;
    (*this)(value, iNew.first->second);
  }

  FloatT operator()(FloatT const& a, FloatT const& b) const {
    if (a == std::numeric_limits<FloatT>::infinity()) {
      return b;
    }
    if (b == std::numeric_limits<FloatT>::infinity()) {
      return a;
    }
    if (a <= b) {
      return a - log1p((FloatT)exp(a - b));
    } else {
      return b - log1p((FloatT)exp(b - a));
    }
  }
  void operator()(FloatT const& b, FloatT& a) const {
    if (b != std::numeric_limits<FloatT>::infinity()) {
      if (a <= b)
        a -= log1p((FloatT)exp(a - b));
      else
        a = b - log1p((FloatT)exp(b - a));
    }
  }

  enum { kIsCommutative = 1 };
  static inline FloatT zeroPlus(FloatT const& b) { return b; }
};

/**
   Functor that subtracts two negated log numbers and returns
   negated log number.

   \return <pre> -log( exp(-a) - exp(-b) ) </pre>
*/
template <class FloatT>
struct NeglogSubFct {
  typedef FloatT result_type;
  typedef FloatT increment_type;
  FloatT operator()(FloatT const& a, FloatT const& b) const {
    if (b == std::numeric_limits<FloatT>::infinity()) return a;
    const FloatT d = a - b;
    if (d <= 0)
      return a - Util::logExpMinus(d);
    else if (d < FloatConstants<FloatT>::epsilon)
      return zero();
    else
      SDL_THROW_LOG(Hypergraph, LogNegativeException, "Cannot represent negative result in log space");
    return b;
  }
  void operator()(FloatT const& b, FloatT& a) const { a = operator()(a, b); }

  enum { kIsCommutative = 0 };
  static inline FloatT zeroPlus(FloatT const& b) {
    if (b == std::numeric_limits<FloatT>::infinity()) return b;  // zero-zero=zero
    SDL_THROW_LOG(Hypergraph, LogNegativeException, "Cannot represent negative result in log space");
    return b;  // doesn't execute
  }
  static inline FloatT zero() { return std::numeric_limits<FloatT>::infinity(); }
};


}}

#endif
