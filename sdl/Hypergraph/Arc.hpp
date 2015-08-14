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

#include <cstring>
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <boost/function.hpp>

#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/Label.hpp>
#include <sdl/Hypergraph/FwdDecls.hpp>
#include <sdl/Util/SmallVector.hpp>
#include <sdl/Util/ZeroInitializedArray.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/Hash.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>

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

struct ArcPrinter {
  std::ostream& o;
  ArcPrinter(std::ostream& o = std::cout) : o(o) {}
  ArcPrinter(ArcPrinter const& o) : o(o.o) {}
  template <class Arc>
  void operator()(Arc* a) const {
    o << *a << '\n';
  }
  template <class Arc>
  void operator()(Arc const& a) const {
    o << a << '\n';
  }
};

// TODO: remove
struct Head {
  StateId state_;
  explicit Head(StateId s) : state_(s) {}
  operator StateId() const { return state_; }
};

// TODO: remove
struct Tails : public StateIdContainer {
  explicit Tails(StateId t1) { this->push_back(t1); }
  explicit Tails(StateId t1, StateId t2) {
    this->push_back(t1);
    this->push_back(t2);
  }
  explicit Tails(StateId t1, StateId t2, StateId t3) {
    this->push_back(t1);
    this->push_back(t2);
    this->push_back(t3);
  }
  explicit Tails(StateId t1, StateId t2, StateId t3, StateId t4) {
    this->push_back(t1);
    this->push_back(t2);
    this->push_back(t3);
    this->push_back(t4);
  }
  explicit Tails(StateId t1, StateId t2, StateId t3, StateId t4, StateId t5) {
    this->push_back(t1);
    this->push_back(t2);
    this->push_back(t3);
    this->push_back(t4);
    this->push_back(t5);
  }
  template <class Iter>
  explicit Tails(Iter begin, Iter end) {
    for (; begin != end; ++begin) {
      this->push_back(*begin);
    }
  }
};


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

  // for ConvertCharsToTokens
  ArcTpl(Head head, Weight const& weight) : weight_(weight) { head_ = head; }
  /// does NOT set weight to 1 or head
  explicit ArcTpl(StateId tail) : ArcBase(tail) {}
  ArcTpl(Head head, StateId tail) : ArcBase(tail) {
    head_ = head;
    setOne(weight_);
  }

  /**
     for binarization - head set later
  */
  ArcTpl(StateId srcState, StateId lexState) : ArcBase(srcState, lexState) { setOne(weight_); }

  /// for GraphInlineInputLabels expandInputLabels - otherwise will crash (these aren't real stateids)
  void appendInline(Sym s) { tails_.push_back((StateId)s.id_); }

#if __cplusplus >= 201103L
  /// move
  // virtual dtor might have prevented automatic use of these
  ArcTpl(ArcTpl&& o) = default;
  ArcTpl& operator=(ArcTpl&& o) = default;
  ArcTpl(ArcTpl const& o) = default;
  ArcTpl& operator=(ArcTpl const& o) = default;
#endif
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
  ArcTpl(ArcTpl const& arc, StateId head)
      : ArcBase(PresizeTails(), arc.tails_.size()), weight_(arc.weight_) {
    head_ = head;
  }

  // for graph rescorings e.g. LmRescore
  ArcTpl(StateId newTail, StateIdContainer const& tails, StateId head, Weight const& weight)
      : ArcBase(tails), weight_(weight) {
    assert(!tails_.empty());
    head_ = head;
    tails_[0] = newTail;
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

  void setHead(StateId s) { head_ = s; }

  StateId head() const { return head_; }

  StateId& head() { return head_; }

  bool isFsmArc() const { return tails_.size() == 2; }

  StateId fsmSrc() const {
    assert(isFsmArc());
    return tails_[0];
  }
  StateId graphSrc() const {
    assert(!tails_.empty());
    return tails_[0];
  }

  StateId fsmSymbolState() const {
    assert(isFsmArc());
    return tails_[1];
  }

  StateIdContainer& tails() { return tails_; }

  StateIdContainer const& tails() const { return tails_; }

  TailIdRange tailIds() const {
    return TailIdRange(TailIdIterator(0), TailIdIterator((TailId)tails_.size()));
  }

  void addTail(StateId s) { tails_.push_back(s); }

  StateId getTail(TailId i) const { return tails_[i]; }

  TailId getNumTails() const { return (TailId)tails_.size(); }

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

template <class Arc>
Sym getFsmInputLabel(IHypergraph<Arc> const& hg, ArcBase const& arc) {
  assert(arc.tails_.size() == 2);
  return hg.inputLabel(arc.tails_[1]);
}

/**
   \return the single lexical symbol if there is one, else NoSymbol. throw if more than one lexical symbol
*/
template <class Arc>
Sym getSingleLexicalLabel(IHypergraph<Arc> const& hg, Arc const& arc, LabelType labelType = kInput) {
  StateIdContainer const& tails = arc.tails();
  for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
    Sym const tailSym = hg.label(*i, labelType);
    if (tailSym.isLexical()) return tailSym;
  }
  return NoSymbol;
}

template <class Arc>
TailId nLexicalLabels(IHypergraph<Arc> const& hg, ArcBase const& arc, LabelType labelType = kInput) {
  StateIdContainer const& tails = arc.tails_;
  TailId nLexical = 0;
  for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i)
    nLexical += (bool)hg.label(*i, labelType).isLexical();
  return nLexical;
}

template <class Arc>
Sym getFsmOutputLabel(IHypergraph<Arc> const& hg, ArcBase const& arc) {
  assert(arc.tails_.size() == 2);
  return hg.outputLabel(arc.tails_[1]);
}

template <class Arc>
void setFsmInputLabel(IMutableHypergraph<Arc>* pHg, ArcBase const& arc, Sym symid) {
  assert(arc.tails_.size() == 2);
  pHg->setInputLabel(arc.tails_[1], symid);
}

template <class Arc>
void setFsmOutputLabel(IMutableHypergraph<Arc>* pHg, ArcBase const& arc, Sym symid) {
  assert(arc.tails_.size() == 2);
  return pHg->setOutputLabel(arc.tails_[1], symid);
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

#if __cplusplus >= 201103L
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
#endif
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
