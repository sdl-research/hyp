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
#include <cmath>

#ifndef SDL_EXACT_LOG1P
#define SDL_EXACT_LOG1P 1
#endif
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

namespace sdl {
namespace Util {

///  \return log1plus(x) = log(1 + x).
static inline double log1plus(double x) {
#if SDL_EXACT_LOG1P
  using namespace std;
  return log1p(x);
#else
  return log(1. + x);
#endif
}
static inline float log1plus(float x) {
#if SDL_EXACT_LOG1P
  using namespace std;
  return log1pf(x);
#else
  return (float)log1plus((double)x);
#endif
}

static inline double logExp(double x) {
  return log1plus(std::exp(x));
}
static inline float logExp(float x) {
  return log1plus(std::exp(x));
}

static inline double logExpMinus(double x) {
  return log1plus(-std::exp(x));
}
static inline float logExpMinus(float x) {
  return log1p(-std::exp(x));
}


/**
   \return -log(exp(-a) + exp(-b))

   A=log(a) + B=log(b) = C=log(a + b)
   C = log(expA + expB) =
   log(expA * (1 + expB/expA)) =
   log(expA) + log(1 + expB/expA) =
   A + log(1 + exp(B-A))

   now negate A, B, C:
   A=-log(a) + B=-log(b) = C=-log(a+b)

   C = -(-A + log(1 + exp(-B--A)) =
   A-log(1 + exp(A-B))

*/
template <class Float>
Float neglogPlus(Float a, Float b) {
  if (a == std::numeric_limits<Float>::infinity()) return b;
  if (b == std::numeric_limits<Float>::infinity()) return a;
  return a <= b ? a - log1plus((Float)std::exp(a - b)) : b - log1plus((Float)std::exp(b - a));
  //    log1plus(x) = log(1 + x).
}

template <class Float>
void neglogPlusBy(Float b, Float& a) {
  if (b != std::numeric_limits<Float>::infinity()) {
    if (a <= b)
      a -= log1plus((Float)std::exp(a - b));
    else
      a = b - log1plus((Float)std::exp(b - a));
  }
}

/// \return -neglogPlus(-a, -b)
template <class Float>
Float logPlus(Float a, Float b) {
  Float b_a = b - a;
  return b_a < 0 ? a + log1plus(std::exp(b_a)) : b + log1plus(std::exp(-b_a));
}

template <class Float>
void logPlusBy(Float b, Float& a) {
  Float b_a = b - a;
  if (b_a < 0)
    a += log1plus((Float)std::exp(b_a));
  else
    a = b + log1plus((Float)std::exp(-b_a));
}

/**
   Functor that multiplies two negated log numbers and returns
   negated log number.

   \return -log( exp(-a) * exp(-b) )
*/
template <class Float>
struct NeglogTimesFct {
  typedef Float result_type;
  typedef Float increment_type;
  Float operator()(Float const& a, Float const& b) const { return a + b; }
  void operator()(Float const& b, Float& a) const { a += b; }

  enum { kIsCommutative = 1 };
  static inline Float zeroPlus(Float const& b) { return b; }
  static inline Float zero() { return (Float)0; }
};

/**
   Functor that divides two negated log numbers and returns
   negated log number.

   \return -log( exp(-a) / exp(-b) )
*/
template <class Float>
struct NeglogDivideFct {
  typedef Float result_type;
  typedef Float increment_type;
  Float operator()(Float const& a, Float const& b) const { return a - b; }
  void operator()(Float const& b, Float& a) const { a -= b; }

  enum { kIsCommutative = 0 };
  static inline Float zeroPlus(Float const& b) { return -b; }
  static inline Float zero() { return (Float)0; }
};


template <class Map>
void mapAddNeglogPlus(Map& map, typename Map::key_type const& key, typename Map::mapped_type value) {
  std::pair<typename Map::iterator, bool> iNew = map.insert(typename Map::value_type(key, value));
  if (iNew.second) return;
  neglogPlusBy(value, iNew.first->second);
}

/**
   For Map.hpp updateBy
*/
template <class Float>
struct NeglogPlusFct {
  typedef Float result_type;
  typedef Float increment_type;

  void operator()(Float b, Float& a) const { neglogPlusBy(b, a); }

  enum { kIsCommutative = 1 };
  static inline Float zeroPlus(Float const& b) { return b; }
};


/**
   \return -log(exp(-a) - exp(-b))

   (A > B)

   A=log(a) - B=log(b) = C=log(a-b)
   C = log(expA-expB) =
   log(expA * (1-expB/expA)) =
   log(expA) + log(1-expB/expA) =
   A + log(1-exp(B-A))

   now negate A, B, C:
   A=-log(a) + B=-log(b) = C=-log(a+b)

   C = -(-A + log(1-exp(-B--A)) =
   A-log(1-exp(A-B))

*/
template <class Float>
inline Float neglogMinus(Float a, Float b) {
  if (b == std::numeric_limits<Float>::infinity()) return a;
  const Float d = a - b;
  if (d <= 0)
    return a - log1plus(-std::exp(d));
  else if (d < FloatConstants<Float>::epsilon)
    return std::numeric_limits<Float>::infinity();
  else
    SDL_THROW_LOG(Hypergraph, LogNegativeException, "Cannot represent negative result in log space");
  return b;
}

/// \return neglogMinus(0, b)
template <class Float>
inline Float neglogSubFrom1(Float b) {
  if (b >= 0)
    return -log1plus(-std::exp(-b));
  else {
    SDL_THROW_LOG(Hypergraph, LogNegativeException,
                  "Cannot represent negative result in log space: 1-exp()");
  }
  return b;
}

/**
   For Map.hpp updateBy
*/
template <class Float>
struct NeglogSubFct {
  typedef Float result_type;
  typedef Float increment_type;
  void operator()(Float b, Float& a) const { a = neglogMinus(a, b); }
  enum { kIsCommutative = 0 };
  static inline Float zeroPlus(Float const& b) {
    if (b == std::numeric_limits<Float>::infinity()) return b;  // zero-zero=zero
    SDL_THROW_LOG(Hypergraph, LogNegativeException, "Cannot represent negative result in log space");
    return b;  // doesn't execute
  }
};


}}

#endif
