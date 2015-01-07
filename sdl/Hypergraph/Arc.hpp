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

#include <sdl/Hypergraph/Types.hpp>

#include <cstring>
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>

#include <sdl/Util/Forall.hpp>
#include <sdl/Hypergraph/Label.hpp>
#include <sdl/Hypergraph/FwdDecls.hpp>
#include <sdl/Util/SmallVector.hpp>
#include <sdl/Util/ZeroInitializedArray.hpp>
#include <sdl/Util/ObjectCount.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/Hash.hpp>

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

struct Head {
  StateId state_;
  explicit Head(StateId s) : state_(s) {}
  operator StateId() const { return state_; }
};

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


// reference to arc's tails valid only as long as arc isn't modified. equal/hash of an Arc ignoring weight
struct ArcConnections {
  StateId const* tails;
  StateId head;
  TailId nTailBytes;
  StateId const* end() const { return (StateId const*)((char const*)tails + nTailBytes); }

  /**
     for unordered_map.
  */
  bool operator==(ArcConnections const& o) const {
    return head == o.head && nTailBytes == o.nTailBytes && !std::memcmp(tails, o.tails, nTailBytes);
  }
  friend inline std::size_t hash_value(ArcConnections const& c) {
    std::size_t h = c.head;
    boost::hash_range(h, c.tails, c.end());
    return h;
  }

  /**
     for std::map or sorting key.
  */
  bool operator<(ArcConnections const& o) const {
    if (head < o.head) return true;
    if (o.head < head) return false;
    if (nTailBytes < o.nTailBytes) return true;
    if (o.nTailBytes < nTailBytes) return false;
    return std::memcmp(tails, o.tails, nTailBytes) < 0;
  }

  template <class Arc>
  void set(Arc const& a) {
    head = a.head();
    StateIdContainer const& t = a.tails();
    tails = arrayBegin(t);
    nTailBytes = (TailId)(sizeof(StateId) * t.size());
  }

  ArcConnections() {}
  template <class Arc>
  explicit ArcConnections(Arc const* a) {
    set(*a);
  }
};

/**
   Hypergraph arc (i.e., hyperedge).

   TODO: introduce a common base class no matter what W is

   TODO: remove virtual dtor to save memory (use whole-hypergraph deleter fn
   object instead)

*/
template <class W>
class ArcTpl SDL_OBJECT_TRACK_BASE(ArcTpl<W>) {
 public:
  friend inline std::size_t hash_value(ArcTpl const& self) { return self.hash(); }
  std::size_t hash() const {
    using namespace Util;
    return MurmurHash(tails_.begin(), sizeof(StateId) * tails_.size(), mixedbits(head_) + hashWeight(weight_));
  }

  typedef sdl::Hypergraph::StateIdContainer StateIdContainer;
  typedef sdl::Hypergraph::StateId StateId;
  typedef W Weight;

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

  ArcTpl() : head_(Hypergraph::kNoState), tails_(), weight_(Weight::one()) {}

  explicit ArcTpl(Weight const& w) : weight_(w) {}

  ArcTpl(Weight const& w, StateId srcState, StateId lexState) : tails_(2), weight_(w) {
    tails_[0] = srcState;
    tails_[1] = lexState;
  }

  ArcTpl(Weight const& w, StateId srcState) : tails_(1, srcState), weight_(w) {}

  // for PhraseDecoder / ArcWithDataTpl
  template <class Cost>
  ArcTpl(Hypergraph::StateId head, Hypergraph::StateId tail, Cost w)
      : head_(head), tails_(1, tail), weight_(w) {}

  /**
     for binarization - head set later
  */
  ArcTpl(StateId srcState, StateId lexState) : tails_(2), weight_(Weight::one()) {
    tails_[0] = srcState;
    tails_[1] = lexState;
  }

  /**
     for making a copy with a tail id substitution. this could be a free fn but
     this is more efficient (no extra copying).
  */
  ArcTpl(ArcTpl const& arc, StateId fromTail, StateId replacementTail)
      : head_(arc.head_), tails_(arc.tails_.size()), weight_(arc.weight_) {
    std::replace_copy(arc.tails_.begin(), arc.tails_.end(), tails_.begin(), fromTail, replacementTail);
  }

  /**
     for Derivation: more efficiently copy weight while remapping states
     (starting with newHead; tails to be updated later).
  */
  ArcTpl(ArcTpl const& arc, StateId newHead)
      : head_(newHead), tails_(arc.tails_.size()), weight_(arc.weight_) {}

  // for graph rescorings e.g. LmRescore
  ArcTpl(StateId newTail, StateIdContainer const& tails, StateId newHead, Weight const& w)
      : head_(newHead), tails_(tails), weight_(w) {
    assert(!tails_.empty());
    tails_[0] = newTail;
  }

  /**
     For hypergraph arcs
  */
  ArcTpl(StateId h, StateIdContainer const& t, Weight const& w = Weight::one())
      : head_(h), tails_(t), weight_(w) {}

  ArcTpl(StateId h, Weight const& w, StateId t) : head_(h), tails_(1, t), weight_(w) {}

  /**
     For simple arcs like in a finite-state machine.
     TODO Maybe remove, since it is easy to mis-use and directly use
     a label here, instead of a state that has the label.
  */
  ArcTpl(StateId from, StateId label, Weight const& w, StateId to) : head_(to), tails_(), weight_(w) {
    tails_.push_back(from);
    tails_.push_back(label);
  }

  // default copy c'tor


  virtual ~ArcTpl() {}

  void setHead(StateId s) { head_ = s; }

  StateId head() const { return head_; }

  StateId& head() { return head_; }

  bool isFsmArc() const { return tails_.size() == 2; }

  StateId fsmSrc() const {
    assert(isFsmArc());
    return tails_[0];
  }
  StateId getGraphSrc() const {
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

  void setWeight(Weight const& w) { weight_ = w; }

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

 private:
  StateId head_;
  StateIdContainer tails_;
  Weight weight_;
};

/**
   Prints arc without vocabulary so with numeric labeled-state ids - for labels,
   use printArc instead.
*/
template <class W>
std::ostream& operator<<(std::ostream& out, const ArcTpl<W>& arc) {
  out << arc.head() << " <- ";
  forall (StateId sid, arc.tails()) { out << sid << " "; }
  out << "/ " << arc.weight();
  return out;
}

template <class W>
std::ostream& operator<<(std::ostream& o, const ArcTpl<W>* selfp) {
  o << "ArcTpl@0x" << (void*)selfp << ": ";
  if (selfp) o << *selfp;
  return o;
}

template <class Arc>
Sym getFsmInputLabel(IHypergraph<Arc> const& hg, Arc const& arc) {
  assert(hg.isFsmArc(arc));
  return hg.inputLabel(arc.getTail(1));
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
TailId nLexicalLabels(IHypergraph<Arc> const& hg, Arc const& arc, LabelType labelType = kInput) {
  StateIdContainer const& tails = arc.tails();
  TailId nLexical = 0;
  for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i)
    nLexical += (bool)hg.label(*i, labelType).isLexical();
  return nLexical;
}

template <class Arc>
Sym getFsmOutputLabel(IHypergraph<Arc> const& hg, Arc const& arc) {
  assert(hg.isFsmArc(arc));
  return hg.outputLabel(arc.getTail(1));
}

template <class Arc>
void setFsmInputLabel(IMutableHypergraph<Arc>* pHg, Arc const& arc, Sym symid) {
  assert(pHg->isFsmArc(arc));
  pHg->setInputLabel(arc.getTail(1), symid);
}

template <class Arc>
void setFsmOutputLabel(IMutableHypergraph<Arc>* pHg, Arc const& arc, Sym symid) {
  assert(pHg->isFsmArc(arc));
  return pHg->setOutputLabel(arc.getTail(1), symid);
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


  ArcWithDataTpl() : Base(), data_(NULL), deleter_(NULL) {}

  ArcWithDataTpl(Hypergraph::StateId head, Hypergraph::StateIdContainer const& tails,
                 Weight const& w = Weight::one())
      : Base(head, tails, w), data_(NULL), deleter_(NULL) {}

  ArcWithDataTpl(StateId from, StateId label, Weight const& w, StateId to)
      : Base(from, label, w, to), data_(NULL), deleter_(NULL) {}

  template <class Cost>
  ArcWithDataTpl(Hypergraph::StateId head, Hypergraph::StateId tail, Cost const& w)
      : Base(head, tail, w), data_(NULL), deleter_(NULL) {}

  template <class Cost>
  ArcWithDataTpl(Hypergraph::StateId head, Cost const& w, Hypergraph::StateId tail)
      : Base(head, w, tail), data_(NULL), deleter_(NULL) {}

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

 private:
  void* data_;
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
