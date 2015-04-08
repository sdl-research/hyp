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
  NoWeight() {}
  template <class W>
  NoWeight(W const& otherWeight) {}
  static NoWeight const& zero() {
    // cppcheck-suppress nullPointer
    return *(NoWeight*)0;
  }
  static NoWeight const& one() {
    // cppcheck-suppress nullPointer
    return *(NoWeight*)0;
  }
  typedef void HasPlusBy;
  void plusBy(NoWeight const& w) const {}
  typedef void HasTimesBy;
  void timesBy(NoWeight const& w) const {}

  typedef void HasIsZero;
  bool isZero() const { return true; }
  typedef void HasIsOne;
  bool isOne() const { return true; }

  /**
     FloatT is required by CompositeWeight<..., NoWeight, ...>
  */
  typedef void FloatT;
};

inline std::ostream& operator<<(std::ostream& out, NoWeight const& w) {
  return out;
}

inline bool operator==(NoWeight const&, NoWeight const&) {
  return true;
}
inline bool operator!=(NoWeight const&, NoWeight const&) {
  return false;
}

template <typename W>
inline bool isNoWeight(W const&) {
  return false;
}

inline bool isNoWeight(NoWeight const&) {
  return true;
}

inline NoWeight const& plus(NoWeight const&, NoWeight const&) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline NoWeight const& minus(NoWeight const&, NoWeight const&) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline NoWeight const& times(NoWeight const&, NoWeight const&) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline NoWeight const& divide(NoWeight const&, NoWeight const&) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline NoWeight const& minusBy(NoWeight const& b, NoWeight const& a) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

inline NoWeight const& divideBy(NoWeight const& b, NoWeight const& a) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}

template <class F>
inline NoWeight const& powBy(F const& b, NoWeight const& a) {
  // cppcheck-suppress nullPointer
  return *(NoWeight*)0;
}


/**
   A composite weight which lets you combine several weights in
   one.


   implicit copy/assign/equal

   (Optional template parameters are implemented in similar
   fashion as boost::tuple, for example.)
*/
template <class W1 = NoWeight, class W2 = NoWeight, class W3 = NoWeight, class W4 = NoWeight, class W5 = NoWeight>
class CompositeWeight {
 public:
  typedef void HasIsZero;
  bool isZero() const {
    return Hypergraph::isZero(w1_) && Hypergraph::isZero(w2_) && Hypergraph::isZero(w3_)
           && Hypergraph::isZero(w4_) && Hypergraph::isZero(w5_);
  }
  typedef void HasIsOne;
  bool isOne() const {
    return Hypergraph::isOne(w1_) && Hypergraph::isOne(w2_) && Hypergraph::isOne(w3_)
           && Hypergraph::isOne(w4_) && Hypergraph::isOne(w5_);
  }

  typedef CompositeWeight Self;
  static Self kOne;
  static Self kZero;

  void plusBy(CompositeWeight const& a) {
    Hypergraph::plusBy(a.w1_, w1_);
    Hypergraph::plusBy(a.w2_, w2_);
    Hypergraph::plusBy(a.w3_, w3_);
    Hypergraph::plusBy(a.w4_, w4_);
    Hypergraph::plusBy(a.w5_, w5_);
  }
  typedef void HasPlusBy;

  void timesBy(CompositeWeight const& a) {
    Hypergraph::timesBy(a.w1_, w1_);
    Hypergraph::timesBy(a.w2_, w2_);
    Hypergraph::timesBy(a.w3_, w3_);
    Hypergraph::timesBy(a.w4_, w4_);
    Hypergraph::timesBy(a.w5_, w5_);
  }
  typedef void HasTimesBy;


  typedef W1 Weight1;
  typedef W2 Weight2;
  typedef W3 Weight3;
  typedef W4 Weight4;
  typedef W5 Weight5;

  explicit CompositeWeight(Weight1 const& w1 = Weight1(), Weight2 const& w2 = Weight2(),
                           Weight3 const& w3 = Weight3(), Weight4 const& w4 = Weight4(),
                           Weight5 const& w5 = Weight5())
      : w1_(w1), w2_(w2), w3_(w3), w4_(w4), w5_(w5) {}

  static CompositeWeight<Weight1, Weight2, Weight3, Weight4, Weight5> zero() {
    return CompositeWeight<Weight1, Weight2, Weight3, Weight4, Weight5>(
        Weight1::zero(), Weight2::zero(), Weight3::zero(), Weight4::zero(), Weight5::zero());
  }

  static CompositeWeight<Weight1, Weight2, Weight3, Weight4, Weight5> one() {
    return CompositeWeight<Weight1, Weight2, Weight3, Weight4, Weight5>(
        Weight1::one(), Weight2::one(), Weight3::one(), Weight4::one(), Weight5::one());
  }


  Weight1& weight1() { return w1_; }
  Weight2& weight2() { return w2_; }
  Weight3& weight3() { return w3_; }
  Weight4& weight4() { return w4_; }
  Weight5& weight5() { return w5_; }

  Weight1 const& weight1() const { return w1_; }
  Weight2 const& weight2() const { return w2_; }
  Weight3 const& weight3() const { return w3_; }
  Weight4 const& weight4() const { return w4_; }
  Weight5 const& weight5() const { return w5_; }

  typedef typename Weight1::FloatT FloatT;
  FloatT getValue() const { return w1_.getValue(); }  // for best-path purposes

 private:
  Weight1 w1_;
  Weight2 w2_;
  Weight3 w3_;
  Weight4 w4_;
  Weight5 w5_;
};


/**
   Specialization for 1 member weight.
*/
template <class W1>
class CompositeWeight<W1, NoWeight, NoWeight, NoWeight, NoWeight> {
 public:
  typedef void HasIsZero;
  bool isZero() const { return Hypergraph::isZero(w1_); }
  typedef void HasIsOne;
  bool isOne() const { return Hypergraph::isOne(w1_); }

  typedef W1 Weight1;
  typedef NoWeight Weight2;
  typedef NoWeight Weight3;
  typedef NoWeight Weight4;
  typedef NoWeight Weight5;
  typedef CompositeWeight Self;
  static Self kOne;
  static Self kZero;

  explicit CompositeWeight(Weight1 const& w1 = Weight1()) : w1_(w1) {}
  CompositeWeight(Weight1 const& w1, Weight2 const&, Weight3 const&, Weight4 const&, Weight5 const&)
      : w1_(w1) {}

  Weight1& weight1() { return w1_; }
  // cppcheck-suppress nullPointer
  NoWeight& weight2() { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight& weight3() { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight& weight4() { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight& weight5() { return *(NoWeight*)0; }

  Weight1 const& weight1() const { return w1_; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight2() const { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight3() const { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight4() const { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight5() const { return *(NoWeight*)0; }

  void plusBy(CompositeWeight const& a) { Hypergraph::plusBy(a.w1_, w1_); }
  typedef void HasPlusBy;
  void timesBy(CompositeWeight const& a) { Hypergraph::timesBy(a.w1_, w1_); }
  typedef void HasTimesBy;

  static CompositeWeight<Weight1, NoWeight, NoWeight, NoWeight, NoWeight> zero() {
    return CompositeWeight<Weight1, NoWeight, NoWeight, NoWeight, NoWeight>(Weight1::zero());
  }

  static CompositeWeight<Weight1, NoWeight, NoWeight, NoWeight, NoWeight> one() {
    return CompositeWeight<Weight1, NoWeight, NoWeight, NoWeight, NoWeight>(Weight1::one());
  }

  typedef typename Weight1::FloatT FloatT;
  FloatT getValue() const { return w1_.getValue(); }  // for best-path purposes
 private:
  Weight1 w1_;
};

/**
   Specialization for 2 member weights.
*/
template <class W1, class W2>
class CompositeWeight<W1, W2, NoWeight, NoWeight, NoWeight> {
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
  typedef NoWeight Weight4;
  typedef NoWeight Weight5;

  CompositeWeight(Weight1 const& w1, Weight2 const& w2, Weight3 const&, Weight4 const&, Weight5 const&)
      : w1_(w1), w2_(w2) {}
  explicit CompositeWeight(Weight1 const& w1 = Weight1(), Weight2 const& w2 = Weight2()) : w1_(w1), w2_(w2) {}

  static CompositeWeight<Weight1, Weight2, NoWeight, NoWeight, NoWeight> zero() {
    return CompositeWeight<Weight1, Weight2, NoWeight, NoWeight, NoWeight>(Weight1::zero(), Weight2::zero());
  }

  static CompositeWeight<Weight1, Weight2, NoWeight, NoWeight, NoWeight> one() {
    return CompositeWeight<Weight1, Weight2, NoWeight, NoWeight, NoWeight>(Weight1::one(), Weight2::one());
  }

  Weight1& weight1() { return w1_; }
  Weight2& weight2() { return w2_; }
  // cppcheck-suppress nullPointer
  NoWeight& weight3() { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight& weight4() { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight& weight5() { return *(NoWeight*)0; }

  Weight1 const& weight1() const { return w1_; }
  Weight2 const& weight2() const { return w2_; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight3() const { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight4() const { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight5() const { return *(NoWeight*)0; }

  typedef typename Weight1::FloatT FloatT;
  FloatT getValue() const { return w1_.getValue(); }  // for best-path purposes
 private:
  Weight1 w1_;
  Weight2 w2_;
};

/**
   Specialization for 3 member weights.
*/
template <class W1, class W2, class W3>
class CompositeWeight<W1, W2, W3, NoWeight, NoWeight> {
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
  typedef NoWeight Weight4;
  typedef NoWeight Weight5;

  CompositeWeight(Weight1 const& w1, Weight2 const& w2, Weight3 const& w3, Weight4 const&, Weight5 const&)
      : w1_(w1), w2_(w2), w3_(w3) {}

  explicit CompositeWeight(Weight1 const& w1 = Weight1(), Weight2 const& w2 = Weight2(),
                           Weight3 const& w3 = Weight3())
      : w1_(w1), w2_(w2), w3_(w3) {}

  Weight1& weight1() { return w1_; }
  Weight2& weight2() { return w2_; }
  Weight3& weight3() { return w3_; }
  // cppcheck-suppress nullPointer
  NoWeight& weight4() { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight& weight5() { return *(NoWeight*)0; }

  Weight1 const& weight1() const { return w1_; }
  Weight2 const& weight2() const { return w2_; }
  Weight3 const& weight3() const { return w3_; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight4() const { return *(NoWeight*)0; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight5() const { return *(NoWeight*)0; }

  static CompositeWeight<Weight1, Weight2, Weight3, NoWeight, NoWeight> zero() {
    return CompositeWeight<Weight1, Weight2, Weight3, NoWeight, NoWeight>(Weight1::zero(), Weight2::zero(),
                                                                          Weight3::zero());
  }

  static CompositeWeight<Weight1, Weight2, Weight3, NoWeight, NoWeight> one() {
    return CompositeWeight<Weight1, Weight2, Weight3, NoWeight, NoWeight>(Weight1::one(), Weight2::one(),
                                                                          Weight3::one());
  }

  typedef typename Weight1::FloatT FloatT;
  FloatT getValue() const { return w1_.getValue(); }  // for best-path purposes
 private:
  Weight1 w1_;
  Weight2 w2_;
  Weight3 w3_;
};

/**
   Specialization for 4 member weights.
*/
template <class W1, class W2, class W3, class W4>
class CompositeWeight<W1, W2, W3, W4, NoWeight> {
 public:
  typedef void HasIsZero;
  bool isZero() const {
    return Hypergraph::isZero(w1_) && Hypergraph::isZero(w2_) && Hypergraph::isZero(w3_)
           && Hypergraph::isZero(w4_);
  }
  typedef void HasIsOne;
  bool isOne() const {
    return Hypergraph::isOne(w1_) && Hypergraph::isOne(w2_) && Hypergraph::isOne(w3_) && Hypergraph::isOne(w4_);
  }

  typedef CompositeWeight Self;
  static Self kOne;
  static Self kZero;

  void plusBy(CompositeWeight const& a) {
    Hypergraph::plusBy(a.w1_, w1_);
    Hypergraph::plusBy(a.w2_, w2_);
    Hypergraph::plusBy(a.w3_, w3_);
    Hypergraph::plusBy(a.w4_, w4_);
  }
  typedef void HasPlusBy;

  void timesBy(CompositeWeight const& a) {
    Hypergraph::timesBy(a.w1_, w1_);
    Hypergraph::timesBy(a.w2_, w2_);
    Hypergraph::timesBy(a.w3_, w3_);
    Hypergraph::timesBy(a.w4_, w4_);
  }
  typedef void HasTimesBy;

  typedef W1 Weight1;
  typedef W2 Weight2;
  typedef W3 Weight3;
  typedef W4 Weight4;
  typedef NoWeight Weight5;

  CompositeWeight(Weight1 const& w1, Weight2 const& w2, Weight3 const& w3, Weight4 const& w4, Weight5 const&)
      : w1_(w1), w2_(w2), w3_(w3), w4_(w4) {}

  explicit CompositeWeight(Weight1 const& w1 = Weight1(), Weight2 const& w2 = Weight2(),
                           Weight3 const& w3 = Weight3(), Weight4 const& w4 = Weight4())
      : w1_(w1), w2_(w2), w3_(w3), w4_(w4) {}

  Weight1& weight1() { return w1_; }
  Weight2& weight2() { return w2_; }
  Weight3& weight3() { return w3_; }
  Weight4& weight4() { return w4_; }
  // cppcheck-suppress nullPointer
  NoWeight& weight5() { return *(NoWeight*)0; }

  Weight1 const& weight1() const { return w1_; }
  Weight2 const& weight2() const { return w2_; }
  Weight3 const& weight3() const { return w3_; }
  Weight4 const& weight4() const { return w4_; }
  // cppcheck-suppress nullPointer
  NoWeight const& weight5() const { return *(NoWeight*)0; }

  static CompositeWeight<Weight1, Weight2, Weight3, Weight4, NoWeight> zero() {
    return CompositeWeight<Weight1, Weight2, Weight3, Weight4, NoWeight>(Weight1::zero(), Weight2::zero(),
                                                                         Weight3::zero(), Weight4::zero());
  }

  static CompositeWeight<Weight1, Weight2, Weight3, Weight4, NoWeight> one() {
    return CompositeWeight<Weight1, Weight2, Weight3, Weight4, NoWeight>(Weight1::one(), Weight2::one(),
                                                                         Weight3::one(), Weight4::one());
  }

  typedef typename Weight1::FloatT FloatT;
  FloatT getValue() const { return w1_.getValue(); }  // for best-path purposes
 private:
  Weight1 w1_;
  Weight2 w2_;
  Weight3 w3_;
  Weight4 w4_;
};


template <class W1, class W2, class W3, class W4, class W5>
bool operator==(CompositeWeight<W1, W2, W3, W4, W5> const& x, CompositeWeight<W1, W2, W3, W4, W5> const& y) {
  return x.weight1() == y.weight1() && x.weight2() == y.weight2() && x.weight3() == y.weight3()
         && x.weight4() == y.weight4() && x.weight5() == y.weight5();
}

template <class W1, class W2, class W3, class W4, class W5>
bool operator!=(CompositeWeight<W1, W2, W3, W4, W5> const& x, CompositeWeight<W1, W2, W3, W4, W5> const& y) {
  return !(x == y);
}


// TODO: optimize by specializing if profiler suggests

template <class W1, class W2, class W3, class W4, class W5>
CompositeWeight<W1, W2, W3, W4, W5> plus(CompositeWeight<W1, W2, W3, W4, W5> const& x,
                                         CompositeWeight<W1, W2, W3, W4, W5> const& y) {
  CompositeWeight<W1, W2, W3, W4, W5> r(x);
  r.plusBy(y);
  return r;
}

template <class W1, class W2, class W3, class W4, class W5>
CompositeWeight<W1, W2, W3, W4, W5> times(CompositeWeight<W1, W2, W3, W4, W5> const& x,
                                          CompositeWeight<W1, W2, W3, W4, W5> const& y) {
  CompositeWeight<W1, W2, W3, W4, W5> r(x);
  r.timesBy(y);
  return r;
}

template <class W1, class W2, class W3, class W4, class W5>
CompositeWeight<W1, W2, W3, W4, W5> divide(CompositeWeight<W1, W2, W3, W4, W5> const& x,
                                           CompositeWeight<W1, W2, W3, W4, W5> const& y) {
  W1 w1(divide(x.weight1(), y.weight1()));
  W2 w2(divide(x.weight2(), y.weight2()));
  W3 w3(divide(x.weight3(), y.weight3()));
  W4 w4(divide(x.weight4(), y.weight4()));
  W5 w5(divide(x.weight5(), y.weight5()));
  return CompositeWeight<W1, W2, W3, W4, W5>(w1, w2, w3, w4, w5);
}

template <class W1, class W2, class W3, class W4, class W5>
CompositeWeight<W1, W2, W3, W4, W5> minus(CompositeWeight<W1, W2, W3, W4, W5> const& x,
                                          CompositeWeight<W1, W2, W3, W4, W5> const& y) {
  W1 w1(minus(x.weight1(), y.weight1()));
  W2 w2(minus(x.weight2(), y.weight2()));
  W3 w3(minus(x.weight3(), y.weight3()));
  W4 w4(minus(x.weight4(), y.weight4()));
  W5 w5(minus(x.weight5(), y.weight5()));
  return CompositeWeight<W1, W2, W3, W4, W5>(w1, w2, w3, w4, w5);
}

template <class W1, class W2, class W3, class W4, class W5>
inline bool operator<(CompositeWeight<W1, W2, W3, W4, W5> const& x,
                      CompositeWeight<W1, W2, W3, W4, W5> const& y) {
  return x.getValue() < y.getValue();
}


template <class W1, class W2, class W3, class W4, class W5>
CompositeWeight<W1, W2, W3, W4, W5>
    CompositeWeight<W1, W2, W3, W4, W5>::kZero(CompositeWeight<W1, W2, W3, W4, W5>::zero());

template <class W1, class W2, class W3, class W4, class W5>
CompositeWeight<W1, W2, W3, W4, W5>
    CompositeWeight<W1, W2, W3, W4, W5>::kOne(CompositeWeight<W1, W2, W3, W4, W5>::one());

// Output operators: (Partial) template specializations.

//  (We could also specialize the times() etc. operations that way
// just for efficiency, but the compiler will prob. already optimize
// appropriately.)

template <class W1, class W2, class W3, class W4, class W5>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, W3, W4, W5> const& w) {
  return out << "(" << w.weight1() << ", " << w.weight2() << ", " << w.weight3() << ", " << w.weight4()
             << ", " << w.weight5() << ")";
}

template <class W1, class W2, class W3, class W4>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, W3, W4, NoWeight> const& w) {
  return out << "(" << w.weight1() << ", " << w.weight2() << ", " << w.weight3() << ", " << w.weight4() << ")";
}

template <class W1, class W2, class W3>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, W3, NoWeight, NoWeight> const& w) {
  return out << "(" << w.weight1() << ", " << w.weight2() << ", " << w.weight3() << ")";
}

template <class W1, class W2>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, NoWeight, NoWeight, NoWeight> const& w) {
  return out << "(" << w.weight1() << ", " << w.weight2() << ")";
}

template <class W1>
std::ostream& operator<<(std::ostream& out,
                         CompositeWeight<W1, NoWeight, NoWeight, NoWeight, NoWeight> const& w) {
  return out << "(" << w.weight1() << ")";
}

inline std::ostream& operator<<(std::ostream& out,
                                CompositeWeight<NoWeight, NoWeight, NoWeight, NoWeight, NoWeight> const& w) {
  return out << "("
             << ")";
}


}}

#endif
