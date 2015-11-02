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

    accumulate weighted set of ngrams up to max len

*/

#ifndef HYP__HYPERGRAPH_NGRAM_WEIGHT_HPP
#define HYP__HYPERGRAPH_NGRAM_WEIGHT_HPP
#pragma once


#include <algorithm>
#include <vector>
#include <map>
#include <cstddef>
#include <iterator>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

#include <sdl/IntTypes.hpp>
#include <sdl/Syms.hpp>
#include <sdl/SharedPtr.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Util/Map.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Util/Constants.hpp>
#include <sdl/Util/Compare.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Types.hpp>

namespace sdl {

struct PtrEqual {
  template <class Ptr>
  bool operator()(Ptr const& p1, Ptr const& p2) const {
    return *p1 == *p2;
  }
};

struct FirstPtrAndSecondEqual {
  template <class Pair>
  bool operator()(Pair const& p1, Pair const& p2) const {
    return *p1.first == *p2.first && p1.second == p2.second;
  }
};

namespace Hypergraph {

/**
   Weight value is a set of ngrams (of length n or shorter) that
   *exactly* span a path from start to end.

   This weight is useful in an all-pairs shortest distance algorithm,
   where the result is, for each pair of states (s, t), the set of
   ngrams that start at s and end at t. In other words, the result is,
   for each pair of states (s, t), a set enumerating all paths of
   length n or shorter that start in s and end in t (n is typically 6
   or 7).

   The times operation combines shorter ngrams into longer ones:
   (m1, m2, ...) x (n1, n2, ...) = (m1n1, m1n2, ..., m2n1, m2n2, ...).

   Note that the times operation is not commutative.

   Note the intended behavior is to not collect longer ngrams, so if
   you call times(["a","b","c"], ["d"]) the result will be *empty* for
   n=3 (trigrams).

   the setting of maxlen to 0 means that you allow either an empty set of ngrams
   or a single empty-string ngram. this is the case for one() and zero()
   both. one() doesn't explicitly store an empty-string ngram, but behaves as
   though it has one for the purposes of plus and times.

   the result of times where each ngram has a different maxlen has the greater
   of the two maxlen. similarly for plus. effectively, maxlen is a contagious
   global setting.

   block symbols aren't included in ngrams

*/
template <class W>
class NgramWeightTpl {

  typedef NgramWeightTpl<W> Self;

 public:
  void set(std::string const& str) {
    SDL_THROW_LOG(Hypergraph.NgramWeightTpl, UnimplementedException, "Not implemented");
  }

  typedef typename W::FloatT FloatT;  // not useful except to make IHypergraph<Arc>::heuristic compile
  static Self kOne;
  static Self kZero;
  bool isZero() const { return zero_; }
  typedef void HasIsZero;
  bool isOne() const { return !zero_ && ngrams_.empty(); }
  bool isEquivalentToOne() const {
    if (isOne()) return true;
    if (ngrams_.size() != 1) return false;
    value_type const& val = *ngrams_.begin();
    return val.first->empty() && Hypergraph::isOne(val.second);
  }
  typedef void HasIsOne;

  typedef W Weight;

  typedef Syms Ngram;
  typedef shared_ptr<Ngram> NgramPtr;

  /**
     default is insufficent because NgramPtr compare by pointer.
  */
  bool operator==(Self const& b) const {
    if (zero_ != b.zero_) return false;
    if (ngrams_.size() != b.ngrams_.size()) return false;
    return std::mismatch(ngrams_.begin(), ngrams_.end(), b.ngrams_.begin(), FirstPtrAndSecondEqual()).second
           == b.ngrams_.end();
  }

  /**
     checks for an equivalent representation of Weight::one()
  */
  bool equivalent(Self const& b) const {
    if (isOne()) return b.isEquivalentToOne();
    if (b.isOne()) return isEquivalentToOne();
    return operator==(b);
  }

  // TODO: we could use a sorted single vector of bytes for the keys, and a parallel vector of weights, if
  // profiling justifies more speedup. or drop shared ptrs per markus' review which was a 20% speedup
  typedef std::map<NgramPtr, Weight, Util::LessByValue<Ngram> > NgramPtrMap;
  typedef typename NgramPtrMap::value_type value_type;
  typedef typename NgramPtrMap::const_iterator const_iterator;
  typedef typename NgramPtrMap::iterator iterator;

#if SDL_WEIGHT_USE_AT_STATIC_INIT
  // reasonably cheap construction. spare us the thread synch difficulty
  static inline constexpr Self one() { return Self(); }
  static inline constexpr Self zero() { return Self(false, false); }
#else
  // The one weight has no ngram:
  static Self const& one() { return kOne; }

  // The zero weight has an ngram of size 1 containing the '0' marker
  // (TODO: better solution would be for zero to have a NULL vector of ngrams_):
  static Self const& zero() { return kZero; }
#endif

  /**
     Default constructor; the ngram maxlen will be unspecified
     (value 0).
  */
  NgramWeightTpl() : zero_(), maxlen_() {}

  explicit NgramWeightTpl(std::size_t maxlen) : zero_(), maxlen_(maxlen) {}

  NgramWeightTpl(bool, bool) : zero_(true), maxlen_() {}  // for zero()

  NgramWeightTpl(Sym lab, std::size_t maxlen) : zero_(), maxlen_(maxlen) {
    if (lab != EPSILON::ID) ngrams_[make_shared<Ngram>(1, lab)] = Weight::one();
  }

  NgramWeightTpl(Sym lab, std::size_t maxlen, Weight const& weight) : zero_(), maxlen_(maxlen) {
    ngrams_[lab == EPSILON::ID ? kEmptyNgram : make_shared<Ngram>(1, lab)] = weight;
  }

  void plusBy(Self const& w2) {
    if (w2.isZero()) {
      return;
    } else if (isZero()) {
      *this = w2;
      return;
    } else {
      assert(!zero_);
      if (isOne()) setExplicitOne();
      if (w2.isOne()) {
        updateBy(PlusBy<W>(), ngrams_, kEmptyNgram, W::one());
        return;
      }
      if (&w2 == this) {
        // should not really happen, ever, but just in case:
        for (iterator i = ngrams_.begin(), e = ngrams_.end(); i != e; ++i)
          Hypergraph::plusBy(i->second, i->second);
        // well, everybody better be ready for plusBy(*this) because I just did it.
        return;
      }
      if (maxlen_ < w2.maxlen_) maxlen_ = w2.maxlen_;
      for (const_iterator i = w2.ngrams_.begin(), e = w2.ngrams_.end(); i != e; ++i) {
        // we would ordinarly just do ngrams_[key(i->first)]+=i->second,
        // except we want a default of Weight::zero(), which is != Weight()
        updateBy(PlusBy<W>(), ngrams_, *i);
      }
    }
  }

  typedef void HasPlusBy;

  const_iterator begin() const { return ngrams_.begin(); }

  const_iterator end() const { return ngrams_.end(); }

  iterator begin() { return ngrams_.begin(); }

  iterator end() { return ngrams_.end(); }

  std::size_t getMaxLen() const { return maxlen_; }

  void setMaxLen(std::size_t len) const { maxlen_ = len; }

  /**
     Removes all ngrams that match a predicate.
     *

     \pred Predicate on the value_type of NgramPtrMap, which is
     std::pair<NgramPtr, Weight>.
     *

     \return The number of ngrams_ removed.
  */
  template <class Predicate>
  void removeNgramIf(Predicate const& pred) {
    eraseIf(ngrams_, pred);
  }

  Weight& plusByNgram(NgramPtr const& pngram, Weight const& weight) {
    updateBy(PlusBy<W>(), ngrams_, pngram, weight);
  }
  Weight& plusByNgram(value_type const& pngramWeight) { updateBy(PlusBy<W>(), ngrams_, pngramWeight); }

  /**
     \return An inserter which can be used for inserting into the set
     of ngrams.
  */
  std::insert_iterator<NgramPtrMap> inserter() { return std::inserter(ngrams_, ngrams_.begin()); }

  void setMaxLen(std::size_t maxlen) { maxlen_ = maxlen; }

  /**
     Returns number of ngrams_
  */
  std::size_t size() const { return ngrams_.size(); }

  bool empty() const { return ngrams_.empty(); }

  /**
     Clones ngrams_ from an input iterator to an output
     iterator.
  */
  template <class InputIter, class OutputIter>
  static void deepCopy(InputIter firstNgramAndWeight, InputIter lastNgramAndWeight, OutputIter result) {
    for (; firstNgramAndWeight != lastNgramAndWeight; ++firstNgramAndWeight)
      *result++ = value_type(make_shared<Ngram>(*firstNgramAndWeight->first), firstNgramAndWeight->second);
  }

  static NgramPtr cloneNgramPtr(NgramPtr const& pNgram) { return make_shared<Ngram>(*pNgram); }

  bool operator<(Self const& other) const {
    SDL_THROW_LOG(Hypergraph.NgramWeightTpl, std::runtime_error, "Not implemented");
  }

 private:
  /**
     NgramWeightTpl::one() effectively has this map contents.
  */

  void setExplicitOne() {
    assert(ngrams_.empty());
    ngrams_.insert(value_type(kEmptyNgram, W::one()));
  }

  NgramPtrMap ngrams_;
  static NgramPtr kEmptyNgram;
  // C++ wart: must friend all or none of the template
  template <class W2>
  friend NgramWeightTpl<W2> times(NgramWeightTpl<W2> const& w1, NgramWeightTpl<W2> const& w2);
  Position maxlen_;
 public:
  bool zero_;
};

template <class W>
typename NgramWeightTpl<W>::NgramPtr
    NgramWeightTpl<W>::kEmptyNgram(make_shared<typename NgramWeightTpl<W>::Ngram>());

template <class W>
NgramWeightTpl<W> NgramWeightTpl<W>::kOne;

template <class W>
NgramWeightTpl<W> NgramWeightTpl<W>::kZero(false, false);

template <class W>
NgramWeightTpl<W> plus(NgramWeightTpl<W> const& w1, NgramWeightTpl<W> const& w2) {
  typedef NgramWeightTpl<W> NgramW;
  NgramW r(w1);
  r.plusBy(w2);
  return r;
}

template <class W>
inline NgramWeightTpl<W> times(NgramWeightTpl<W> const& w1, NgramWeightTpl<W> const& w2) {
  typedef NgramWeightTpl<W> Ngw;
  if (w1.isOne()) return w2;
  if (w2.isOne()) return w1;
  if (w1.empty() || w2.empty()) return Ngw::kZero;
  assert(!w1.isZero());
  assert(!w2.isZero());
  const std::size_t maxlen = std::max(w1.getMaxLen(), w2.getMaxLen());
  Ngw product(maxlen);
  assert(!product.isZero());
  typedef typename Ngw::Ngram Ngram;
  typedef typename Ngw::value_type NgramPtrAndWeight;
  typedef typename Ngw::const_iterator Iter;
  for (NgramPtrAndWeight const& p1 : w1) {
    Ngram const& ngram1 = *p1.first;
    std::size_t const len1 = ngram1.size();
    assert(!Vocabulary::countBlockSymbols(ngram1));
    assert(len1 == Vocabulary::countRuleSrcSymbols(ngram1));
    assert(len1 <= w1.getMaxLen());
    assert(len1 <= maxlen);
    std::size_t maxlen2 = maxlen - len1;
    for (Iter i2 = w2.begin(), e2 = w2.end(); i2 != e2; ++i2) {
      NgramPtrAndWeight const& p2 = *i2;
      Ngram const& ngram2 = *p2.first;
      std::size_t const len2 =ngram2.size();
      assert(!Vocabulary::countBlockSymbols(ngram2));
      assert(len2 == Vocabulary::countRuleSrcSymbols(ngram2));
      assert(len2 <= w2.getMaxLen());
      if (len2 > maxlen2) continue;
      W const& wtConcat = times(p1.second, p2.second);
      if (Hypergraph::isZero(wtConcat)) continue;
      NgramPtrAndWeight concatNgWt(make_shared<Ngram>(ngram1.size() + ngram2.size()), wtConcat);
      Util::copyConcatRanges(ngram1, ngram2, concatNgWt.first->begin());  //=n1+n2
      assert(!Util::contains(product.ngrams_, concatNgWt.first));
      product.ngrams_.insert(concatNgWt);
    }
  }
  if (product.size() == 0) return Ngw::kZero;  // not one. once ngrams_ grow too long, you're done.
  return product;
}

template <class W>
bool operator==(NgramWeightTpl<W> const& w1, NgramWeightTpl<W> const& w2) {
  typedef typename NgramWeightTpl<W>::Ngram Ngram;
  typedef typename NgramWeightTpl<W>::NgramPtr NgramPtr;
  if (w1.size() != w2.size()) return false;
  typename NgramWeightTpl<W>::const_iterator it1 = w1.begin();
  typename NgramWeightTpl<W>::const_iterator w1End = w1.end();
  typename NgramWeightTpl<W>::const_iterator it2 = w2.begin();
  for (; it1 != w1End; ++it1, ++it2) {
    NgramPtr pNgram1 = it1->first;
    NgramPtr pNgram2 = it2->first;
    W const& w1 = it1->second;
    W const& w2 = it2->second;
    if (*pNgram1 != *pNgram2 || w1 != w2) return false;
  }
  return true;
}

template <class W>
bool operator!=(NgramWeightTpl<W> const& w1, NgramWeightTpl<W> const& w2) {
  return !(w1 == w2);
}

template <class W>
std::ostream& operator<<(std::ostream& out, NgramWeightTpl<W> const& w) {
  typedef NgramWeightTpl<W> NgramW;
  if (w == NgramW::zero()) {
    out << "Zero";
    return out;
  }
  typedef typename NgramW::Ngram Ngram;
  typedef typename NgramW::NgramPtr NgramPtr;
  typedef std::pair<NgramPtr, W> NgramPtrAndWeight;
  bool first1 = true;
  out << "(";
  for (NgramPtrAndWeight ngramPtrAndWeight : w) {
    bool first = true;
    out << (first1 ? "" : ", ") << "[";
    NgramPtr pNgram = ngramPtrAndWeight.first;
    W const& weight = ngramPtrAndWeight.second;
    for (Sym const& label : *pNgram) {
      out << (first ? "" : " ") << label;
      first = false;
    }
    out << " / " << weight;
    out << "]";
    first1 = false;
  }
  out << ")";
  return out;
}


}}

#endif
