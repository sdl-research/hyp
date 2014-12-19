/** \file

    Some convenience functions relating to Weight classes. we might specialize
    some of these for higher performance, or the optimizer might be doing
    perfectly well already
*/

#ifndef HYP__HYPERGRAPH_WEIGHTUTIL_HPP
#define HYP__HYPERGRAPH_WEIGHTUTIL_HPP
#pragma once


#ifndef SDL_WEIGHT_USE_AT_STATIC_INIT
# define SDL_WEIGHT_USE_AT_STATIC_INIT 1
// if enabled, weights can be used at static init time (in other words, don't
// try to cache zero and one values for CompositeWeight)
#endif

#if SDL_WEIGHT_USE_AT_STATIC_INIT
#include <sdl/Util/CacheStaticLocal.hpp>
#endif

#include <sdl/Hypergraph/WeightsFwdDecls.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <cmath>

namespace sdl {
namespace Hypergraph {

template <class W, class Enable = void>
struct IsZero {
  static inline bool isZero(W const& w) { return w==kZero; }
  static W kZero;
};

template <class W, class Enable = void>
struct IsOne {
  static inline bool isOne(W const& w) { return w==kOne; }
  static W kOne;
};

/**
   define, in your weight type, a

   typedef void HasIsZero

   if you have a public:

   bool isZero() const;
*/
template <class W>
struct IsZero<W, typename W::HasIsZero> {
  static inline bool isZero(W const& w) { return w.isZero(); }
  static W kZero;
};

/**
   ibid but One for Zero (less important - see profiler for time spent in W::one()).
*/
template <class W>
struct IsOne<W, typename W::HasIsOne> {
  static inline bool isOne(W const& w) { return w.isOne(); }
  static W kOne;
};

// note that weights are required to return correct W::one() and W::zero() even during static init (we relax this for CompositeWeight - you're not allowed to have CompositeWeight of CompositeWeight). so this is ok.
template <class W, class E>
W IsZero<W, E>::kZero(W::zero());

// these constants exist also even for HaveIsZero/One

template <class W, class E>
W IsOne<W, E>::kOne(W::one());

/**
   equivalent to, but faster than, w==W::zero(). may be called only after static init (after main starts)
*/
template <class W>
inline bool isZero(W const& w) {
  return IsZero<W>::isZero(w);
}

/**
   equivalent to, but faster than, w==W::one(). may be called only after static init (after main starts)
*/
template <class W>
inline bool isOne(W const& w) {
  return IsOne<W>::isOne(w);
}

template <class W>
inline void setOne(W &w) {
  w = W::one();
}

template <class W>
inline void setZero(W &w) {
  w = W::zero();
}

/**
   plusBy(delta, accum) is faster for any Weight with:

   struct Weight {
   typedef void HasPlusBy;
   void plusBy(Weight const& b);
   }
*/

template <class W, class Enable = void>
struct PlusBy {
  typedef W By;
  typedef W Result;
  static inline void plusBy(By const& b, Result &a) {
    a = Hypergraph::plus(a, b);
  }
  /**
     for use with e.g. Map.hpp updateBy(PlusBy<W>(), map, key, W(delta)).
  */
  inline void operator()(By const& b, Result &a) const {
    a = Hypergraph::plus(a, b);
  }
};

template <class W>
struct PlusBy<W, typename W::HasPlusBy> {
  typedef W By;
  typedef W Result;
  static inline void plusBy(By const& b, Result &a) {
    a.plusBy(b);
  }
  inline void operator()(By const& b, Result &a) const {
    a.plusBy(b);
  }
};

template<class W>
inline void plusBy(W const& b, W &a) {
  PlusBy<W>::plusBy(b, a);
}


/**
   timesBy(delta, accum) is faster for any Weight with:

   struct Weight {
   typedef void HasTimesBy;
   void timesBy(Weight const& b);
   }
*/

template <class W, class Enable = void>
struct TimesBy {
  typedef W By;
  typedef W Result;
  static inline void timesBy(By const& b, Result &a) {
    a = Hypergraph::times(a, b);
  }
};

template <class W>
struct TimesBy<W, typename W::HasTimesBy> {
  typedef W By;
  typedef W Result;
  static inline void timesBy(By const& b, Result &a) {
    a.timesBy(b);
  }
};

template<class W>
inline void timesBy(W const& b, W &a) {
  TimesBy<W>::timesBy(b, a);
}


////

// note: minus, divide, multiplicative inverse, power - will throw (unsupported operation) for general semirings. only some of our weight classes happen to offer these


template <class W>
void minusBy(W const& b, W &a) {
  a = Hypergraph::minus(a, b);
}

/**
   Divides a by b and returns the modified a.
*/
template <class W>
void divideBy(W const& b, W &a) {
  a = Hypergraph::divide(a, b);
}

template <class Float>
void divideBy(FloatWeightTpl<Float> const& b, ViterbiWeightTpl<Float> &a) {
  a.value() -= b.getValue();
}

template <class Float>
void divideBy(FloatWeightTpl<Float> const& b, LogWeightTpl<Float> &a) {
  a.value() -= b.getValue();
}

template <class W, class F>
void powBy(F const& b, W &a) {
  a = Hypergraph::pow(a, b);
}

template <class W>
void multInverseEq(W &a) {
  a = multInverse(a);
}

///

template<class W>
W weightOrZero(std::string const& str) {
  if (str.empty())
    return W::zero();
  W r;
  parseWeightString(str, &r);
  return r;
}

template<class W>
W weightOrOne(std::string const& str) {
  if (str.empty())
    return W::one();
  W r;
  parseWeightString(str, &r);
  return r;
}


template <class W>
inline bool plusByChanges(W const& a, W const& b) {
  return true;
}

template <class W>
inline bool plusByChanged(W const& a, W &b) {
  plusBy(a, b);
  return true;
}

template <class T>
inline bool plusByChanges(ViterbiWeightTpl<T> const& a, ViterbiWeightTpl<T> const &b) {
  return (a.getValue()<b.getValue());
}

template <class T>
inline bool plusByChanged(ViterbiWeightTpl<T> const& a, ViterbiWeightTpl<T> &b) {
  if (a.getValue()<b.getValue()) {
    b = a;
    return true;
  }
  return false;
}

template <class T>
inline bool checkPlusBy(ViterbiWeightTpl<T> const& a, ViterbiWeightTpl<T> const& b, T epsilon = FloatConstants<T>::epsilon)
{
  (void)epsilon;
  return b.getValue()==std::min(a.getValue(), b.getValue());
}

template <class T>
inline bool checkPlusBy(FloatWeightTpl<T> const& a, FloatWeightTpl<T> const& b, T epsilon = FloatConstants<T>::epsilon)
{
  return approxLessOrEqual(a, b, epsilon); // this would need to change if we had negative LogWeight
}

inline
bool plusByChanges(BooleanWeight const& a, BooleanWeight const &b) {
  return a.getValue()<b.getValue();
}

inline
bool plusByChanged(BooleanWeight const& a, BooleanWeight &b) {
  if (a.getValue()<b.getValue()) {
    b = a;
    return true;
  }
  return false;
}

template<class T>
T neglogToProb(T const& v) {
  return std::exp(-v);
}

template<class T>
T probToNeglog(T const& prob) {
  return -std::log(prob);
}

template<class T>
struct ProbToNeglog {
  FloatWeightTpl<T> operator()(T prob) const {
    return FloatWeightTpl<T>(-std::log(prob));
  }
};

template<class T>
struct Identity {
  FloatWeightTpl<T> operator()(T prob) const {
    return prob;
  }
};

#if HAVE_OPENFST
template <class Weight>
Weight Times(Weight const& a, Weight const& b) {
  return Hypergraph::times(a, b);
}

template <class Weight>
Weight Plus(Weight const& a, Weight const& b) {
  return Hypergraph::plus(a, b);
}

template <class Weight>
bool ApproxEqual(Weight const& a, Weight const& b, float epsilon) {
  return approxEqual(a, b, epsilon);
}
#endif

template <class Weight>
void setOneValue(Weight &w) {
  w.setValue(Weight::kOneValue());
}

}}

#endif
