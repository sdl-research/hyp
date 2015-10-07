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
#ifndef HYP__HYPERGRAPH_ARC_HPP
#define HYP__HYPERGRAPH_ARC_HPP
#pragma once

/*
  a hyperarc ArcTpl<Weight> has:

  head state

  1 or more tail states (we do NOT allow 0 tails - use an epsilon-labeled or unlabeled state for a single
  tail, e.g. start state)

  weight

  (see also ArcWithData which has additional data attached)

  it's intended that Arc * be owned by their container and deleted on destruction (don't want smart ptr cost
  or holding by value)

*/

#include <sdl/Hypergraph/ArcBase.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/SmallVector.hpp>
#include <sdl/Util/ZeroInitializedArray.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/Hash.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>

#include <cstring>
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <boost/function.hpp>


namespace sdl {
namespace Hypergraph {

struct WeightCount {
  std::size_t n, n1;
  WeightCount() : n(0), n1(0) {}
  bool unweighted() const { return n1 == n; }
  bool weighted() const { return !unweighted(); }
  template <class A>
  void operator()(A const* a) {
    ++n;
    if (a->weight() == A::Weight::one()) ++n1;
  }
  template <class Out>
  void print(Out& o) const {
    o << n << " hyperarcs, " << n1;
    if (n1 == n) o << " (all)";
    o << " unweighted";
  }

  friend inline std::ostream& operator<<(std::ostream& o, WeightCount const& w) {
    w.print(o);
    return o;
  }
};

template <class Arc>
struct ArcPrinter {
  std::ostream& o;
  ArcPrinter(std::ostream& o = std::cout) : o(o) {}
  ArcPrinter(ArcPrinter const& o) : o(o.o) {}
  void operator()(ArcBase const* a) const {
    operator()(*(Arc const*)a);
  }
  void operator()(Arc const& a) const {
    o << a << '\n';
  }
};

/// syntactic sugar, used for new Arc(Head(s), Tails(t, u), Weight::one())
inline StateId Head(StateId s) {
  return s;
}

inline StateIdContainer Tails(StateId t1) {
  StateIdContainer r;
  r.push_back(t1);
  return r;
}
inline StateIdContainer Tails(StateId t1, StateId t2) {
  StateIdContainer r;
  r.push_back(t1);
  r.push_back(t2);
  return r;
}
inline StateIdContainer Tails(StateId t1, StateId t2, StateId t3) {
  StateIdContainer r;
  r.push_back(t1);
  r.push_back(t2);
  r.push_back(t3);
  return r;
}
inline StateIdContainer Tails(StateId t1, StateId t2, StateId t3, StateId t4) {
  StateIdContainer r;
  r.push_back(t1);
  r.push_back(t2);
  r.push_back(t3);
  r.push_back(t4);
  return r;
}
inline StateIdContainer Tails(StateId t1, StateId t2, StateId t3, StateId t4, StateId t5) {
  StateIdContainer r;
  r.push_back(t1);
  r.push_back(t2);
  r.push_back(t3);
  r.push_back(t4);
  r.push_back(t5);
  return r;
}
template <class Iter>
inline StateIdContainer Tails(Iter begin, Iter end) {
  StateIdContainer r;
  for (; begin != end; ++begin) {
    r.push_back(*begin);
  }
  return r;
}


/// tags to distinguish ArcTpl(...) constructors
struct HeadAndWeight {};
struct HeadAndTail {};

/**
   Hypergraph arc (i.e., hyperedge).

   TODO: introduce a common base class no matter what W is

   TODO: remove virtual dtor to save memory (use whole-hypergraph deleter fn
   object instead)

*/
template <class W>
struct ArcTpl : ArcBase {
 public:
  typedef W Weight;
  Weight weight_;

  friend inline std::size_t hash_value(ArcTpl const& self) { return self.hash(); }
  std::size_t hash() const {
    using namespace Util;
    return MurmurHash(tails_.begin(), sizeof(StateId) * tails_.size(),
                      mixedbits(head_) + Util::hashFloat(weight_.value_));
  }


  typedef ArcTpl<W> Arc;
  typedef boost::function<bool(Arc*)> ArcFilter;
  typedef boost::function<void(Arc*)> ArcVisitor;
  // TODO: consider using a fixed-size boost::array, e.g. for 2 tails
  // (binarized hypergraph)
  // better name: StateIds

  static bool fnFilterTrue(Arc*) { return true; }
  static bool fnFilterFalse(Arc*) { return false; }
  static ArcFilter filterTrue() { return (ArcFilter)fnFilterTrue; }
  static ArcFilter filterFalse() { return (ArcFilter)fnFilterFalse; }

  ArcTpl() {
    head_ = Hypergraph::kNoState;
    setOne(weight_);
  }

  explicit ArcTpl(Weight const& weight) : weight_(weight) {}

  typedef typename W::FloatT Cost;

  ArcTpl(Weight const& weight, StateId srcState) : ArcBase(srcState), weight_(weight) {}

  ArcTpl(HeadAndWeight, StateId head, Weight const& weight) : weight_(weight) { head_ = head; }

  ArcTpl(HeadAndTail, StateId head, StateId tail) : ArcBase(tail) {
    head_ = head;
    setOne(weight_);
  }
  ArcTpl(HeadAndTail, StateId head, StateId tail0, StateId tail1) : ArcBase(tail0, tail1) {
    head_ = head;
  }
  ArcTpl(HeadAndTail, StateId head, StateId tail0, StateId tail1, Weight const& weight)
      : ArcBase(tail0, tail1), weight_(weight) {
    head_ = head;
  }

  /// sets only a single tail; you must set head and weight
  explicit ArcTpl(StateId tail) : ArcBase(tail) {}

  /**
     for binarization - head set later
  */
  ArcTpl(StateId srcState, StateId lexState) : ArcBase(srcState, lexState) { setOne(weight_); }

  /// for GraphInlineInputLabels expandInputLabels - otherwise will crash (these aren't real stateids)
  void appendInline(Sym s) { tails_.push_back((StateId)s.id_); }

  /// move
  // virtual dtor might have prevented automatic use of these
  ArcTpl(ArcTpl&& o) = default;
  ArcTpl& operator=(ArcTpl&& o) = default;
  ArcTpl(ArcTpl const& o) = default;
  ArcTpl& operator=(ArcTpl const& o) = default;

  /**
     for making a copy with a tail id substitution. this could be a free fn but
     this is more efficient (no extra copying).
  */
  ArcTpl(ArcTpl const& arc, StateId fromTail, StateId replacementTail)
      : ArcBase(arc.tails_, fromTail, replacementTail), weight_(arc.weight_) {
    head_ = arc.head_;
  }

  /**
     for Derivation: more efficiently copy weight while remapping states
     (starting with newHead; tails to be updated later).
  */
  ArcTpl(ArcTpl const& arc, StateId head) : ArcBase(PresizeTails(), arc.tails_.size()), weight_(arc.weight_) {
    head_ = head;
  }

  // for graph rescorings e.g. LmRescore
  ArcTpl(StateId newTail, StateIdContainer const& tails, StateId head, Weight const& weight)
      : ArcBase(tails), weight_(weight) {
    assert(!tails_.empty());
    head_ = head;
    tails_[0] = newTail;
  }

  explicit ArcTpl(ArcBase const& base)
      : ArcBase(base) {
    setOne(weight_);
  }

  /**
     For hypergraph arcs
  */
  ArcTpl(StateId head, StateIdContainer const& tails, Weight const& weight = Weight::one())
      : ArcBase(tails), weight_(weight) {
    head_ = head;
  }

  // for ConvertCharsToTokens and hypergraph
  ArcTpl(StateId head, Weight const& weight, StateId tail) : ArcBase(tail), weight_(weight) { head_ = head; }

  ArcTpl(StateId head, Cost c, StateId tail) : ArcBase(tail), weight_(c) { head_ = head; }

  /**
     fsm arc
  */
  ArcTpl(StateId from, StateId label, Weight const& weight, StateId to)
      : ArcBase(from, label), weight_(weight) {
    head_ = to;
  }

  ArcTpl(StateId from, StateId label, Cost c, StateId to) : ArcBase(from, label), weight_(c) { head_ = to; }

  // default copy/move/etc c'tor

  virtual ~ArcTpl() {}

  void setWeight(Weight const& weight) { weight_ = weight; }

  Weight const& weight() const { return weight_; }

  Weight& weight() { return weight_; }

  bool operator==(Arc const& o) const {
    return head_ == o.head_ && weight_ == o.weight_ && tails_ == o.tails_;
  }

  // TODO: some weights are pretty heavyweight to <. implement a 3-valued cmp in
  // WeightUtil?
  bool operator<(Arc const& other) const {
    // heads equal: continue with weight
    if (weight_ < other.weight_) return true;
    if (other.weight_ < weight_) return false;

    StateIdContainer const& a = tails();
    StateIdContainer const& b = other.tails();

    TailId asz = a.size(), bsz = b.size();

    if (asz < bsz) return true;
    if (bsz < asz) return false;

    if (head_ < other.head()) return true;
    if (other.head() < head_) return false;

    if (!asz) return false;  // equal 0-tails

    return std::memcmp(a.begin(), b.begin(), asz * sizeof(StateId)) < 0;
  }
};

/**
   Prints arc without vocabulary so with numeric labeled-state ids - for labels,
   use printArc instead.
*/
template <class W>
std::ostream& operator<<(std::ostream& out, ArcTpl<W> const& arc) {
  arc.print(out);
  out << " / " << arc.weight();
  return out;
}

template <class W>
std::ostream& operator<<(std::ostream& o, const ArcTpl<W>* selfp) {
  o << "ArcTpl@0x" << (void*)selfp << ": ";
  if (selfp) o << *selfp;
  return o;
}

struct ArcWithDataDeleter {
  virtual void dispose(void*) const = 0;
  virtual ~ArcWithDataDeleter() {}
};

template <class T>
struct ArcWithDataDeleterTpl : public ArcWithDataDeleter {
  void dispose(void* ptr) const OVERRIDE { delete (T*)ptr; }
};

template <class T>
struct SingletonArcWithDataDeleter {
  static ArcWithDataDeleterTpl<T> deleter;
};

template <class T>
ArcWithDataDeleterTpl<T> SingletonArcWithDataDeleter<T>::deleter;

/**
   ArcWithDataTpl is like ArcTpl, but you can store extra data in it. One could
   instead store that data in the weight, but that is sometimes inappropriate
   because the data shouldn't be modified by times, plus, etc

   this is interesting because later modules may look for data still attatched, and
   the last user can free it, without needing to copy the arcs (actually, you
   waste a void* of space by not copying, but at least that's all you waste) to
   regular ArcTpl.

   TODO: make ArcWithData merely provide setData/getData (void*). subclasses may
   have the dispose-any (deleter interface) or may assert that only one static
   type be used/deleted, or may not delete anything (caller manages). these
   every-arc-the-same-type-of-data use cases seem more common than the
   deleter-based one, and we save space by splitting them up this way.

   put another way, there's no need to pay for a pointer for a vtable AND a
   pointer for a dispose object.

   TODO: (keeping virtual dtor) non-void* data versions that allocate
   contiguous-space state: variable-sized-pod (can't use regular new/constructor)
   and/or fixed-sized pod (which can). but then you wouldn't want to pass these on
   to later modules that don't know about the data or else you're just wasting
   space (since you can't free it) - so you'd make a copy of the arc as a regular
   arc in which case there's no need for virtual anything.

*/
template <class W>
class ArcWithDataTpl : public ArcTpl<W> {
 public:
  typedef W Weight;
  typedef ArcTpl<W> Base;
  typedef ArcWithDataTpl<W> Arc;
  typedef boost::function<bool(Arc*)> ArcFilter;
  typedef boost::function<void(Arc*)> ArcVisitor;

  static bool fnFilterTrue(Arc*) { return true; }
  static bool fnFilterFalse(Arc*) { return false; }
  static ArcFilter filterTrue() { return (ArcFilter)fnFilterTrue; }
  static ArcFilter filterFalse() { return (ArcFilter)fnFilterFalse; }


  ArcWithDataTpl() : data_(), deleter_() {}

  ArcWithDataTpl(StateId head, StateIdContainer const& tails, Weight const& w = Weight::one())
      : Base(head, tails, w), data_(), deleter_() {}

  ArcWithDataTpl(StateId from, StateId label, Weight const& w, StateId to)
      : Base(from, label, w, to), data_(), deleter_() {}

  template <class Cost>
  ArcWithDataTpl(StateId head, Cost const& w, StateId tail)
      : Base(head, w, tail), data_(), deleter_() {}

  virtual ~ArcWithDataTpl() {
    if (deleter_ && data_) deleter_->dispose(data_);
  }

  // like shared_ptr, a deleter object knows how to dispose of your void*
  void clear() {
    if (deleter_ && data_) deleter_->dispose(data_);
    data_ = 0;
  }

  /// call this instead of setData if data will never be set except w/ type T + ownTheData
  template <class T>
  void setDataJustThisType(T* d) {
    if (deleter_) {
      assert(deleter_ == &SingletonArcWithDataDeleter<T>::deleter);
      deleter_->dispose(data_);
    } else
      deleter_ = &SingletonArcWithDataDeleter<T>::deleter;
    data_ = d;
  }

  template <class T>
  void setDataUnowned(T* d) {
    deleter_ = 0;
    data_ = d;
  }

  template <class T>
  void setDataAnyType(T* d) {
    deleter_ = &SingletonArcWithDataDeleter<T>::deleter;
    data_ = d;
  }

  void* getData() const { return data_; }

  void* release() {
    deleter_ = 0;
    return data_;
  }
  /// \return ref to data_ contents, assuming data_ points at a type T directly
  template <class T>
  T& dataAs() const {
    return *static_cast<T*>(data_);
  }

  template <class T>
  T const& dataAsConst() const {
    return *static_cast<T*>(data_);
  }

  bool isDataEmpty() const { return data_ == NULL; }

  /// move
  ArcWithDataTpl(ArcWithDataTpl&& o) noexcept : Base(static_cast<Base&&>(o)),
                                                data_(o.data_),
                                                deleter_(o.deleter_) {
    o.deleter_ = 0;
  }
  ArcWithDataTpl& operator=(ArcWithDataTpl&& o) noexcept {
    Base::operator=(static_cast<Base&&>(o));
    data_ = o.data_;
    deleter_ = o.deleter_;
    o.deleter_ = 0;
  }

  /**
     alternative to making copy private: don't copy data. we could copy data but
     not deleter but then lifetime of source (may have been temporary) isn't
     known. this way programmer errors referencing copied data will be noticed.

     another alternative: change from deleter ptr to delete-or-clone ptr, or
     just use sdl::shared_ptr<void> which does refcount/delete
  */
  ArcWithDataTpl(ArcWithDataTpl const& o) : Base(static_cast<Base const&>(o)), data_(), deleter_() {}

  ArcWithDataTpl& operator=(ArcWithDataTpl const& o) {
    operator=(static_cast<Base const&>(o));
    data_ = 0;
    deleter_ = 0;
  }

  void* data_;

 private:
  ArcWithDataDeleter const* deleter_;
};


/**
   \return copy of otherArc, but with all states (head and tails) mapped
   according to the StateIdTranslation.
*/
template <class Arc, class StateIdTranslation>
Arc* copyArcWithMappedStates(StateIdTranslation& tr, Arc const& otherArc) {
  Arc* pResult = new Arc();
  pResult->setHead(tr.stateFor(otherArc.head()));
  pResult->setWeight(otherArc.weight());
  for (TailId i = 0, end = otherArc.getNumTails(); i < end; ++i) {
    const StateId mappedTail = tr.stateFor(otherArc.getTail(i));
    assert(mappedTail != kNoState);
    pResult->addTail(mappedTail);
  }
  return pResult;
}

struct DeleteArcData {
  template <class Weight>
  void operator()(ArcTpl<Weight>* arc) const {
    static_cast<ArcWithDataTpl<Weight>*>(arc)->clear();
  }
  template <class Weight>
  void operator()(ArcWithDataTpl<Weight>* arc) const {
    arc->clear();
  }
};


}}

#endif
