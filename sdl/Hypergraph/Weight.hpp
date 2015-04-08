// Copyright 2014 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    semirings used for Arc for Hypergraph.
*/


#ifndef HYP__HYPERGRAPH_WEIGHT_HPP
#define HYP__HYPERGRAPH_WEIGHT_HPP
#pragma once

/**
   Weight: semiring (zero, one, plus, times, less,==).

   note: some more expensive semirings' zero and one will not be usable until
   main() (static globals). singleton lazy-init pattern not worth the overhead.
*/

#include <sdl/LexicalCast.hpp>

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/Exception.hpp>
#include <sdl/Hypergraph/WeightBase.hpp>

#include <sdl/Util/Constants.hpp>
#include <sdl/Util/Math.hpp>
#include <sdl/Util/LogMath.hpp>

#include <boost/math/special_functions/log1p.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Hash.hpp>

#if HAVE_OPENFST
// OpenFst weight compatability - needed for ToReplaceFst
#define DEFINE_OPENFST_COMPAT_FUNCTIONS(name)            \
  static std::string const& Type() {                     \
    static const std::string r = #name;                  \
    return r;                                            \
  }                                                      \
  static const Self One() { return Self::one(); }        \
  static const Self Zero() { return Self::zero(); }      \
  void Write(std::ostream& o) const { o << *this; }      \
  static inline std::size_t Properties() { return 0x3; } \
  typedef Self ReverseWeight;                            \
  Self& Reverse() { return *this; }                      \
  Self const& Reverse() const { return *this; }
#else
#define DEFINE_OPENFST_COMPAT_FUNCTIONS(name)
#endif

#define SDL_DEFINE_FLOATWT_CMP(c, cmp)                \
  template <class T>                                  \
  bool operator cmp(c<T> const& w1, c<T> const& w2) { \
    return w1.getValue() cmp w2.getValue();           \
  }

#define SDL_DEFINE_FLOATWT_CMPS(c) \
  SDL_DEFINE_FLOATWT_CMP(c, == )   \
  SDL_DEFINE_FLOATWT_CMP(c, >= )   \
  SDL_DEFINE_FLOATWT_CMP(c, <= )   \
  SDL_DEFINE_FLOATWT_CMP(c, != )   \
  SDL_DEFINE_FLOATWT_CMP(c, < )    \
  SDL_DEFINE_FLOATWT_CMP(c, > )

// TODO: #if HAVE_OPENFST and use constants appropriate for particular semiring - this is just to compile
// HgFsmDraw

// TODO: OpenFst Divide. use sdl::Util::logPlus?

namespace sdl {
namespace Hypergraph {

/**
   Simple wrapper weight around a float value. Base class for
   Viterbi weight, Log weight, etc.
 */
template <class T>
class FloatWeightTpl : public WeightBase {
 protected:
  typedef typename Util::OtherFloat<T>::type DoubleT;

 public:
  typedef T FloatT;

  static inline FloatT kOneValue() { return 0; }

  static inline FloatT kZeroValue() { return std::numeric_limits<T>::infinity(); }

  typedef void HasIsZero;
  bool isZero() const { return value_ == std::numeric_limits<T>::infinity(); }
  friend inline void setZero(FloatWeightTpl& x) { x.value_ = std::numeric_limits<T>::infinity(); }

  typedef void HasIsOne;
  bool isOne() const { return !value_; }
  friend inline void setOne(FloatWeightTpl& x) { x.value_ = 0; }

#if __cplusplus >= 201103L || CPP11
  FloatWeightTpl() = default;  // uninitialized
#else
  FloatWeightTpl() {}  // uninitialized
#endif

  FloatWeightTpl(T v) : value_(v) {}
  FloatWeightTpl(DoubleT v) : value_((T)v) {}
  FloatWeightTpl(int v) : value_((T)v) {}
  FloatWeightTpl(std::size_t v) : value_((T)v) {}

  T& value() { return value_; }

  T getValue() const { return value_; }

  void setValue(T v) { value_ = v; }
  void setValue(DoubleT v) { value_ = (T)v; }
  void setValue(int v) { value_ = (T)v; }
  void setValue(std::size_t v) { value_ = (T)v; }

  T value_;
};

SDL_DEFINE_FLOATWT_CMPS(FloatWeightTpl)

template <class T>
std::istream& operator>>(std::istream& i, FloatWeightTpl<T>& x) {
  return i >> x.value();
}
template <class T>
std::ostream& operator<<(std::ostream& o, FloatWeightTpl<T> const& x) {
  return o << x.getValue();
}

/// Use this (guaranteed to be correct) and not sdl::lexical_cast<Weight> (not, e.g. ExpectationWeight)
// TODO: Remove this.
template <class T>
void parseWeightString(std::string const& str, FloatWeightTpl<T>* w) {
  if (!str.empty()) {
    w->setValue(sdl::lexical_cast<T>(str));
  }
}

template <class T>
inline std::size_t hashWeight(const FloatWeightTpl<T>& w) {
  return Util::hashFloat(w.getValue());
}

template <class T>
inline bool approxEqual(const FloatWeightTpl<T>& w1, const FloatWeightTpl<T>& w2,
                        T epsilon = FloatConstants<T>::epsilon) {
  return Util::floatEqual(w1.getValue(), w2.getValue(), epsilon);
}

template <class T>
inline bool approxGreaterOrEqual(const FloatWeightTpl<T>& w1, const FloatWeightTpl<T>& w2,
                                 T epsilon = FloatConstants<T>::epsilon) {
  return Util::approxGreaterOrEqual(w1.getValue(), w2.getValue(), epsilon);
}

template <class T>
inline bool approxLessOrEqual(const FloatWeightTpl<T>& w1, const FloatWeightTpl<T>& w2,
                              T epsilon = FloatConstants<T>::epsilon) {
  return Util::approxLessOrEqual(w1.getValue(), w2.getValue(), epsilon);
}

template <class T>
inline bool definitelyGreater(const FloatWeightTpl<T>& w1, const FloatWeightTpl<T>& w2,
                              T epsilon = FloatConstants<T>::epsilon) {
  return Util::definitelyGreater(w1.getValue(), w2.getValue(), epsilon);
}

template <class T>
inline bool definitelyLess(const FloatWeightTpl<T>& w1, const FloatWeightTpl<T>& w2,
                           T epsilon = FloatConstants<T>::epsilon) {
  return Util::definitelyLess(w1.getValue(), w2.getValue(), epsilon);
}

// all 3 of approx equal, greater, less may be true simultaneously (if same eps, then approxEqual <=>
// approxGreater and approxLess

class BooleanWeight : public FloatWeightTpl<bool> {
 public:
  typedef bool FloatT;

  BooleanWeight() : FloatWeightTpl<bool>() {}

  // explicit only because somebody was wrongly activating this implicitly (they should use safe_bool idiom?)
  explicit BooleanWeight(bool v) : FloatWeightTpl<bool>(v) {}

  typedef BooleanWeight Self;
  DEFINE_OPENFST_COMPAT_FUNCTIONS(Boolean)

  typedef void HasIsOne;
  bool isOne() const { return value_ == true; }
  friend inline void setOne(BooleanWeight& x) { x.value_ = true; }
  static inline BooleanWeight one() { return BooleanWeight(true); }

  typedef void HasIsZero;
  bool isZero() const { return value_ == false; }
  friend inline void setZero(BooleanWeight& x) { x.value_ = false; }
  static inline BooleanWeight zero() { return BooleanWeight(false); }

};

template <class T>
class ViterbiWeightTpl : public FloatWeightTpl<T> {
  typedef ViterbiWeightTpl<T> Self;
  typedef FloatWeightTpl<T> Base;

 public:
  typedef void HasIsZero;
  bool isZero() const { return this->value_ == std::numeric_limits<T>::infinity(); }
  typedef void HasIsOne;
  bool isOne() const { return !this->value_; }

  typedef T FloatT;

  ViterbiWeightTpl() : Base() {}

  ViterbiWeightTpl(T v) : Base(v) {}
  ViterbiWeightTpl(int v) : Base(v) {}
  ViterbiWeightTpl(std::size_t v) : Base(v) {}
  ViterbiWeightTpl(typename Base::DoubleT v) : Base(v) {}

  Self& operator=(Base const& other) {
    this->setValue(other.getValue());
    return *this;
  }

  static inline ViterbiWeightTpl<T> one() { return ViterbiWeightTpl<T>(0.0f); }

  static inline ViterbiWeightTpl<T> zero() { return ViterbiWeightTpl<T>(FloatLimits<T>::posInfinity); }

  void plusBy(Self const& b) {
    if (b.value_ < this->value_) this->value_ = b.value_;
  }
  typedef void HasPlusBy;

  void timesBy(Self const& b) { this->value_ += b.value_; }
  typedef void HasTimesBy;

  DEFINE_OPENFST_COMPAT_FUNCTIONS(Viterbi)
};


template <class T>
ViterbiWeightTpl<T> plus(ViterbiWeightTpl<T> const& w1, ViterbiWeightTpl<T> const& w2) {
  return w1.getValue() < w2.getValue() ? w1 : w2;
}

template <class T>
ViterbiWeightTpl<T> minus(ViterbiWeightTpl<T> const& w1, ViterbiWeightTpl<T> const& w2) {
  SDL_THROW_LOG(Hypergraph, UnimplementedException, "Viterbi minus is not supported");
  return w1;  // make compiler happy
}

template <class T>
inline ViterbiWeightTpl<T> times(ViterbiWeightTpl<T> const& w1, ViterbiWeightTpl<T> const& w2) {
  if (w1 == ViterbiWeightTpl<T>::zero() || w2 == ViterbiWeightTpl<T>::zero())
    return ViterbiWeightTpl<T>::zero();
  return ViterbiWeightTpl<T>(w1.getValue() + w2.getValue());
}

template <class T>
inline bool less(ViterbiWeightTpl<T> const& w1, ViterbiWeightTpl<T> const& w2) {
  return w1.getValue() < w2.getValue();
}

template <class T>
inline ViterbiWeightTpl<T> divide(ViterbiWeightTpl<T> const& w1, ViterbiWeightTpl<T> const& w2) {
  if (w1 == ViterbiWeightTpl<T>::zero() || w2 == ViterbiWeightTpl<T>::zero())
    // Technically can't divide by 0. but practically ok.
    return ViterbiWeightTpl<T>::zero();
  return ViterbiWeightTpl<T>(w1.getValue() - w2.getValue());
}

template <class T>
inline ViterbiWeightTpl<T> invert(ViterbiWeightTpl<T> const& w) {
  return ViterbiWeightTpl<T>(-w.getValue());
}

template <class T>
inline ViterbiWeightTpl<T> pow(ViterbiWeightTpl<T> const& w, T k) {
  return ViterbiWeightTpl<T>(k * w.getValue());
}

template <class T>
class LogWeightTpl : public FloatWeightTpl<T> {

  typedef FloatWeightTpl<T> Base;
  typedef LogWeightTpl Self;

 public:
  typedef void HasIsZero;
  bool isZero() const { return this->value_ == std::numeric_limits<T>::infinity(); }

  typedef void HasIsOne;
  bool isOne() const { return !this->value_; }

  typedef T FloatT;

  LogWeightTpl() : Base() {}

  LogWeightTpl(T v) : Base(v) {}
  LogWeightTpl(typename Base::DoubleT v) : Base(v) {}
  LogWeightTpl(int v) : Base(v) {}
  LogWeightTpl(std::size_t v) : Base(v) {}

  static inline LogWeightTpl<T> one() { return LogWeightTpl<T>(0.0f); }

  static inline LogWeightTpl<T> zero() { return LogWeightTpl<T>(FloatLimits<T>::posInfinity); }

  Self& operator=(Base const& other) {
    this->setValue(other.getValue());
    return *this;
  }


  void timesBy(LogWeightTpl const& b) { this->value_ += b.value_; }
  typedef void HasTimesBy;

  DEFINE_OPENFST_COMPAT_FUNCTIONS(Log)
};

template <class T>
LogWeightTpl<T> plus(LogWeightTpl<T> const& w1, LogWeightTpl<T> const& w2) {
  if (w1 == LogWeightTpl<T>::zero()) return w2;
  if (w2 == LogWeightTpl<T>::zero()) return w1;

  const T f1 = w1.getValue();
  const T f2 = w2.getValue();
  const T d = f2 - f1;  // d>0 means prob1>prob2
  if (d > 0)
    return LogWeightTpl<T>(f1 - Util::logExp(-d));
  else
    return LogWeightTpl<T>(f2 - Util::logExp(d));
}

template <class T>
LogWeightTpl<T> minus(LogWeightTpl<T> const& w1, LogWeightTpl<T> const& w2) {
  typedef LogWeightTpl<T> W;
  if (w2 == W::zero()) return w1;

  const T f1 = w1.getValue();
  const T f2 = w2.getValue();
  const T d = f1 - f2;  // d>0 means prob1>prob2
  if (d < 0)  // w1>w2 because cost1 < cost2
    return W(f1 - Util::logExpMinus(d));
  else if (d >= FloatConstants<T>::epsilon)
    SDL_THROW_LOG(Hypergraph.Weight, LogNegativeException,
                  "a-b=" << w1.getValue() - w2.getValue()
                         << " greater than epsilon=" << FloatConstants<T>::epsilon);
  return W::zero();
}

template <class T>
inline LogWeightTpl<T> times(LogWeightTpl<T> const& w1, LogWeightTpl<T> const& w2) {
  if (w1 == LogWeightTpl<T>::zero() || w2 == LogWeightTpl<T>::zero()) return LogWeightTpl<T>::zero();
  return LogWeightTpl<T>(w1.getValue() + w2.getValue());
}

template <class T>
inline bool less(LogWeightTpl<T> const& w1, LogWeightTpl<T> const& w2) {
  return w1.getValue() < w2.getValue();
}

template <class T>
inline LogWeightTpl<T> divide(LogWeightTpl<T> const& w1, LogWeightTpl<T> const& w2) {
  if (w1 == LogWeightTpl<T>::zero() || w2 == LogWeightTpl<T>::zero()) {
    // Technically can't divide by zero. but practically ok.
    return LogWeightTpl<T>::zero();
  }
  return LogWeightTpl<T>(w1.getValue() - w2.getValue());
}

template <class T>
inline LogWeightTpl<T> invert(LogWeightTpl<T> const& w) {
  return LogWeightTpl<T>(-w.getValue());
}

template <class T>
inline LogWeightTpl<T> pow(LogWeightTpl<T> const& w, T k) {
  return LogWeightTpl<T>(k * w.getValue());
}

typedef LogWeightTpl<float> LogWeight;
typedef ViterbiWeightTpl<float> ViterbiWeight;

inline BooleanWeight plus(BooleanWeight const& w1, BooleanWeight const& w2) {
  return BooleanWeight(w1.getValue() || w2.getValue());
}


inline BooleanWeight times(BooleanWeight const& w1, BooleanWeight const& w2) {
  return BooleanWeight(w1.getValue() && w2.getValue());
}

inline bool less(BooleanWeight const& w1, BooleanWeight const& w2) {
  return w1.getValue() < w2.getValue();
}

inline BooleanWeight divide(BooleanWeight const& w1, BooleanWeight const& w2) {
  SDL_THROW_LOG(Hypergraph, UnimplementedException, "Boolean divide is not supported");
  return w1;  // make compiler happy
}

inline BooleanWeight invert(BooleanWeight const& w1, BooleanWeight const& w2) {
  SDL_THROW_LOG(Hypergraph, UnimplementedException, "Boolean invert is not supported");
  return w1;  // make compiler happy
}

inline BooleanWeight pow(BooleanWeight const& w, bool p) {
  return p ? w : BooleanWeight(true);
}

template <class Weight>
char const* weightName(Weight*) {
  return "Weight";
}

template <class Weight>
char const* weightName() {
  return weightName((Weight*)0);
}

template <class T>
inline char const* weightName(ViterbiWeightTpl<T>*) {
  return "Viterbi";
}

template <class T>
inline char const* weightName(LogWeightTpl<T>*) {
  return "Log";
}

inline char const* weightName(BooleanWeight*) {
  return "Boolean";
}


}}

#endif
