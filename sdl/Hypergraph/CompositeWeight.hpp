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

    composite tuple of weights w/ semiring ops pairwise on the tuple elements.

    normally a weight W is required to return correct W::one() and W::zero()
    even during static init (we relax this for CompositeWeight - you're not
    allowed to have CompositeWeight of CompositeWeight). that is: nobody should
    use any CompositeWeight<..>::one() or zero() before static init is done.

    the first weight in a CompositeWeight must have a FloatT getValue()
*/

#ifndef HYP__HYPERGRAPH_COMPOSITEWEIGHT_HPP
#define HYP__HYPERGRAPH_COMPOSITEWEIGHT_HPP
#pragma once

#include <sdl/Hypergraph/WeightUtil.hpp>
#include <ostream>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Used as a placeholder for empty slots in CompositeWeight.
*/
class NoWeight {
 public:
  constexpr NoWeight() {}
  template <class W>
  constexpr NoWeight(W const& otherWeight) {}
  static constexpr NoWeight const& zero() {
    // cppcheck-suppress nullPointer
    return *(NoWeight*)0;
  }
  static constexpr NoWeight const& one() {
    // cppcheck-suppress nullPointer
    return *(NoWeight*)0;
  }
  typedef void HasPlusBy;
  void plusBy(NoWeight const& w) const {}
  typedef void HasTimesBy;
  void timesBy(NoWeight const& w) const {}

  typedef void HasIsZero;
  constexpr bool isZero() const { return true; }
  typedef void HasIsOne;
  constexpr bool isOne() const { return true; }

  /**
     FloatT is required by CompositeWeight<..., NoWeight, ...>
  */
  typedef void FloatT;
};

inline std::ostream& operator<<(std::ostream& out, NoWeight const& w) {
  return out;
}

inline constexpr bool operator==(NoWeight const&, NoWeight const&) {
  return true;
}
inline constexpr bool operator!=(NoWeight const&, NoWeight const&) {
  return false;
}

template <typename W>
inline constexpr bool isNoWeight(W const&) {
  return false;
}

inline constexpr bool isNoWeight(NoWeight const&) {
  return true;
}

inline constexpr NoWeight const& plus(NoWeight const&, NoWeight const&) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline constexpr NoWeight const& minus(NoWeight const&, NoWeight const&) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline constexpr NoWeight const& times(NoWeight const&, NoWeight const&) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline constexpr NoWeight const& divide(NoWeight const&, NoWeight const&) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline constexpr NoWeight const& minusBy(NoWeight const& b, NoWeight const& a) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline constexpr NoWeight const& divideBy(NoWeight const& b, NoWeight const& a) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

template <class F>
inline constexpr NoWeight const& powBy(F const& b, NoWeight const& a) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}


/**
   A composite weight which lets you combine several weights in
   one.


   implicit copy/assign/equal

   (Optional template parameters are implemented in similar
   fashion as tuple, for example.)

   TODO: C++11 typelist / variadic?
*/
template <class W1, class W2 = NoWeight, class W3 = NoWeight>
class CompositeWeight {
 public:
  typedef void HasIsZero;
  bool isZero() const {
    return Hypergraph::isZero(w1_) && Hypergraph::isZero(w2_) && Hypergraph::isZero(w3_);
  }
  typedef void HasIsOne;
  bool isOne() const { return Hypergraph::isOne(w1_) && Hypergraph::isOne(w2_) && Hypergraph::isOne(w3_); }

  typedef CompositeWeight Self;
  static Self kOne;
  static Self kZero;

  void plusBy(CompositeWeight const& a) {
    Hypergraph::plusBy(a.w1_, w1_);
    Hypergraph::plusBy(a.w2_, w2_);
    Hypergraph::plusBy(a.w3_, w3_);
  }
  typedef void HasPlusBy;

  void timesBy(CompositeWeight const& a) {
    Hypergraph::timesBy(a.w1_, w1_);
    Hypergraph::timesBy(a.w2_, w2_);
    Hypergraph::timesBy(a.w3_, w3_);
  }
  typedef void HasTimesBy;

  typedef W1 Weight1;
  typedef W2 Weight2;
  typedef W3 Weight3;

  explicit constexpr CompositeWeight(Weight1 const& w1 = Weight1(), Weight2 const& w2 = Weight2(),
                                     Weight3 const& w3 = Weight3())
      : w1_(w1), w2_(w2), w3_(w3) {}

  static constexpr CompositeWeight<Weight1, Weight2, Weight3> zero() {
    return CompositeWeight<Weight1, Weight2, Weight3>(Weight1::zero(), Weight2::zero(), Weight3::zero());
  }

  static constexpr CompositeWeight<Weight1, Weight2, Weight3> one() {
    return CompositeWeight<Weight1, Weight2, Weight3>(Weight1::one(), Weight2::one(), Weight3::one());
  }


  Weight1& weight1() { return w1_; }
  Weight2& weight2() { return w2_; }
  Weight3& weight3() { return w3_; }

  Weight1 const& weight1() const { return w1_; }
  Weight2 const& weight2() const { return w2_; }
  Weight3 const& weight3() const { return w3_; }

  typedef typename Weight1::FloatT FloatT;
  FloatT getValue() const { return w1_.getValue(); }  // for best-path purposes

  Weight1 w1_;
  Weight2 w2_;
  Weight3 w3_;
};


/**
   Specialization for 1 member weight.
*/
template <class W1>
class CompositeWeight<W1, NoWeight, NoWeight> {
 public:
  typedef void HasIsZero;
  bool isZero() const { return Hypergraph::isZero(w1_); }
  typedef void HasIsOne;
  bool isOne() const { return Hypergraph::isOne(w1_); }

  typedef W1 Weight1;
  typedef NoWeight Weight2;
  typedef NoWeight Weight3;
  typedef CompositeWeight Self;
  static Self kOne;
  static Self kZero;

  explicit CompositeWeight(Weight1 const& w1 = Weight1()) : w1_(w1) {}
  CompositeWeight(Weight1 const& w1, Weight2 const&, Weight3 const&) : w1_(w1) {}

  Weight1& weight1() { return w1_; }
  // cppcheck-suppress nullPointer
  NoWeight& weight2() { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight& weight3() { return *(NoWeight*)0; }

  Weight1 const& weight1() const { return w1_; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight2() const { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight3() const { return *(NoWeight*)0; }

  void plusBy(CompositeWeight const& a) { Hypergraph::plusBy(a.w1_, w1_); }
  typedef void HasPlusBy;
  void timesBy(CompositeWeight const& a) { Hypergraph::timesBy(a.w1_, w1_); }
  typedef void HasTimesBy;

  static constexpr CompositeWeight<Weight1, NoWeight, NoWeight> zero() {
    return CompositeWeight<Weight1, NoWeight, NoWeight>(Weight1::zero());
  }

  static constexpr CompositeWeight<Weight1, NoWeight, NoWeight> one() {
    return CompositeWeight<Weight1, NoWeight, NoWeight>(Weight1::one());
  }

  typedef typename Weight1::FloatT FloatT;
  FloatT getValue() const { return w1_.getValue(); }  // for best-path purposes
  Weight1 w1_;
};

/**
   Specialization for 2 member weights.
*/
template <class W1, class W2>
class CompositeWeight<W1, W2, NoWeight> {
 public:
  typedef void HasIsZero;
  bool isZero() const { return Hypergraph::isZero(w1_) && Hypergraph::isZero(w2_); }
  typedef void HasIsOne;
  bool isOne() const { return Hypergraph::isOne(w1_) && Hypergraph::isOne(w2_); }

  typedef CompositeWeight Self;
  static Self kOne;
  static Self kZero;

  void plusBy(CompositeWeight const& a) {
    Hypergraph::plusBy(a.w1_, w1_);
    Hypergraph::plusBy(a.w2_, w2_);
  }
  typedef void HasPlusBy;
  void timesBy(CompositeWeight const& a) {
    Hypergraph::timesBy(a.w1_, w1_);
    Hypergraph::timesBy(a.w2_, w2_);
  }
  typedef void HasTimesBy;

  typedef W1 Weight1;
  typedef W2 Weight2;
  typedef NoWeight Weight3;

  CompositeWeight(Weight1 const& w1, Weight2 const& w2, Weight3 const&) : w1_(w1), w2_(w2) {}
  explicit CompositeWeight(Weight1 const& w1 = Weight1(), Weight2 const& w2 = Weight2()) : w1_(w1), w2_(w2) {}

  static constexpr CompositeWeight<Weight1, Weight2, NoWeight> zero() {
    return CompositeWeight<Weight1, Weight2, NoWeight>(Weight1::zero(), Weight2::zero());
  }

  static constexpr CompositeWeight<Weight1, Weight2, NoWeight> one() {
    return CompositeWeight<Weight1, Weight2, NoWeight>(Weight1::one(), Weight2::one());
  }

  Weight1& weight1() { return w1_; }
  Weight2& weight2() { return w2_; }
  // cppcheck-suppress nullPointer
  NoWeight& weight3() { return *(NoWeight*)0; }

  Weight1 const& weight1() const { return w1_; }
  Weight2 const& weight2() const { return w2_; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight3() const { return *(NoWeight*)0; }

  typedef typename Weight1::FloatT FloatT;
  FloatT getValue() const { return w1_.getValue(); }  // for best-path purposes
  Weight1 w1_;
  Weight2 w2_;
};

template <class W1, class W2, class W3>
bool operator==(CompositeWeight<W1, W2, W3> const& x, CompositeWeight<W1, W2, W3> const& y) {
  return x.weight1() == y.weight1() && x.weight2() == y.weight2() && x.weight3() == y.weight3();
}

template <class W1, class W2, class W3>
bool operator!=(CompositeWeight<W1, W2, W3> const& x, CompositeWeight<W1, W2, W3> const& y) {
  return !(x == y);
}


// TODO: optimize by specializing if profiler suggests

template <class W1, class W2, class W3>
CompositeWeight<W1, W2, W3> plus(CompositeWeight<W1, W2, W3> const& x, CompositeWeight<W1, W2, W3> const& y) {
  CompositeWeight<W1, W2, W3> r(x);
  r.plusBy(y);
  return r;
}

template <class W1, class W2, class W3>
CompositeWeight<W1, W2, W3> times(CompositeWeight<W1, W2, W3> const& x, CompositeWeight<W1, W2, W3> const& y) {
  CompositeWeight<W1, W2, W3> r(x);
  r.timesBy(y);
  return r;
}

template <class W1, class W2, class W3>
CompositeWeight<W1, W2, W3> divide(CompositeWeight<W1, W2, W3> const& x, CompositeWeight<W1, W2, W3> const& y) {
  W1 w1(divide(x.weight1(), y.weight1()));
  W2 w2(divide(x.weight2(), y.weight2()));
  W3 w3(divide(x.weight3(), y.weight3()));
  return CompositeWeight<W1, W2, W3>(w1, w2, w3);
}

template <class W1, class W2, class W3>
CompositeWeight<W1, W2, W3> minus(CompositeWeight<W1, W2, W3> const& x, CompositeWeight<W1, W2, W3> const& y) {
  W1 w1(minus(x.weight1(), y.weight1()));
  W2 w2(minus(x.weight2(), y.weight2()));
  W3 w3(minus(x.weight3(), y.weight3()));
  return CompositeWeight<W1, W2, W3>(w1, w2, w3);
}

template <class W1, class W2, class W3>
inline bool operator<(CompositeWeight<W1, W2, W3> const& x, CompositeWeight<W1, W2, W3> const& y) {
  return x.getValue() < y.getValue();
}

template <class W1, class W2, class W3>
CompositeWeight<W1, W2, W3> CompositeWeight<W1, W2, W3>::kZero(CompositeWeight<W1, W2, W3>::zero());

template <class W1, class W2, class W3>
CompositeWeight<W1, W2, W3> CompositeWeight<W1, W2, W3>::kOne(CompositeWeight<W1, W2, W3>::one());

// Output operators: (Partial) template specializations.

//  (We could also specialize the times() etc. operations that way
// just for efficiency, but the compiler will prob. already optimize
// appropriately.)

template <class W1, class W2, class W3>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, W3> const& w) {
  return out << "(" << w.weight1() << ", " << w.weight2() << ", " << w.weight3() << ")";
}

template <class W1, class W2>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, NoWeight> const& w) {
  return out << "(" << w.weight1() << ", " << w.weight2() << ")";
}

template <class W1>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, NoWeight, NoWeight> const& w) {
  return out << "(" << w.weight1() << ")";
}

inline std::ostream& operator<<(std::ostream& out, CompositeWeight<NoWeight, NoWeight, NoWeight> const& w) {
  return out << "("
             << ")";
}


}}

#endif
