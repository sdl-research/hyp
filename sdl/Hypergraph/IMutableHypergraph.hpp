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

    IHypergraph that can be modified in-place.
*/

#ifndef HYP__IMUTABLEHYPERGRAPH_HPP
#define HYP__IMUTABLEHYPERGRAPH_HPP
#pragma once

#include <algorithm>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/StateIdTranslation.hpp>
#include <sdl/Hypergraph/SortArcs.hpp>
#include <sdl/Util/IteratorGenerator.hpp>
#include <sdl/Hypergraph/Visit.hpp>

namespace sdl {
namespace Hypergraph {

struct BlockSpans;

template <class Arc>
struct SortStates;

/**
   An IHypergraph you can modify. In practice this is always a
   MutableHypergraph, which uses arrays to store things.
*/
template <class A>
struct IMutableHypergraph : IHypergraph<A> {
  static char const* staticType() { return "IHypergraph"; }

  void setHaveConstraints(bool haveConstraintStarts, bool haveConstraintEnds) {
    if (haveConstraintEnds && !haveConstraintStarts)
      SDL_THROW_LOG(Hypergraph.Constraints, ConfigException,
                    "we always have constraint start-states if we have constraints, and sometimes have "
                    "constraint end-states - so haveConstraintEnds && !haveConstraintStarts is an error");
    this->setPropertyBit(kConstraintStarts, haveConstraintStarts);
    this->setPropertyBit(kConstraintEnds, haveConstraintEnds);
  }

 protected:

  friend struct SortStates<A>;
  StateId sortedStatesNumNotTerminal_;  // this is set only if properties() & kSortedStates

 public:
  typedef IHypergraph<A> Base;
  typedef typename A::ArcFilter ArcFilter;
  typedef IMutableHypergraph<A> Self;
  typedef shared_ptr<Self> Ptr;
  typedef A Arc;
  typedef typename A::Weight Weight;
  typedef HypergraphBase::ArcsContainer ArcsContainer;
  typedef ArcsContainer::const_iterator OutArcsIter;
  // we could change this to AnyGenerator (at significantly increased cost) if we want to allow alternate
  // IMutableHypergraph impls
  typedef Util::IteratorGenerator<OutArcsIter, Arc*> OutArcsGenerator;
  typedef typename Base::ConstOutArcsGenerator ConstOutArcsGenerator;

  /// offer: take if you don't have one
  void offerVocabulary(IVocabularyPtr v) {
    if (!this->getVocabulary()) this->setVocabulary(v);
  }

  template <class Arc2>
  void offerVocabulary(IHypergraph<Arc2> const& h) {
    this->offerVocabulary(h.getVocabulary());
  }

  virtual ConstOutArcsGenerator outArcsConst(StateId state) const OVERRIDE {
    return ConstOutArcsGenerator(outArcs(state));
  }

  /**
     used by SortStates<A>.
  */
  void setSortedStatesNumNotTerminal(StateId n) { sortedStatesNumNotTerminal_ = n; }

  /**
     remove the out arcs for a state (useful after you update Arc * such that no arcs have it as a tail)
  */
  virtual void clearOutArcs(StateId state) = 0;

  virtual void deleteState(StateId s) {
    if (this->storesOutArcs()) {
      assert(!this->storesInArcs());
      deleteOutArcsImpl(s);
    } else
      deleteInArcsImpl(s);
  }

  void deleteInArcs(StateId s) { deleteInArcsImpl(s); }

  void deleteOutArcs(StateId s) { deleteOutArcsImpl(s); }

  void deleteOutArcsExcept(StateId s, Arc* keepArc) { deleteOutArcsExceptImpl(s, keepArc); }

  virtual void deleteInArcsImpl(StateId) = 0;
  virtual void deleteOutArcsImpl(StateId) = 0;
  virtual void deleteOutArcsExceptImpl(StateId, Arc*) = 0;

  // return number of deleted arcs
  std::size_t restrict(StateIdTranslation& x) { return restrict(x, 0); }

  // for each Arc *arc, if keep(arc), then remap arc through x and
  // place it in the in/out arcs index. similar to HypergraphCopy.hpp
  // but in-place. note: if x.frozen, then keep is implicitly
  // heads+tails-in-x(arc) AND keep(arc)
  virtual std::size_t restrict(StateIdTranslation& x, ArcFilter const& keep) = 0;

  /**
     keep only arcs with keep(arc). if storesInArcs, then the arcs with same head are visited consecutively.
  */
  virtual std::size_t restrict(ArcFilter const& keep) = 0;

  virtual bool isMutable() const OVERRIDE { return true; }

  // if you know you've added arcs out of order (or in order), inform us by setting the property
  void setBestFirstArcs(bool on = true) { setPropertiesAt(kOutArcsSortedBestFirst, on); }

  /// order arcs by weight value - for lazy fs/compose
  void forceBestFirstArcs() {
    this->addProperties(kOutArcsSortedBestFirst);
    for (StateId state = 0, n = this->size(); state < n; ++state) {
      ArcsContainer* arcs = this->maybeOutArcs(state);
      if (arcs) std::sort(arcs->begin(), arcs->end(), CmpByWeight<Arc>());
    }
  }

  StateId numNotTerminalStates() const OVERRIDE {
    return (this->properties() & kSortedStates) ? sortedStatesNumNotTerminal_
                                                : this->countNumNotTerminalStates();
  }

  StateId maxNotTerminalState() const OVERRIDE {
    return (this->properties() & kSortedStates) ? sortedStatesNumNotTerminal_ - 1
                                                : this->maxNotTerminalStateImpl();
  }

  StateId exactSizeForHeads() const OVERRIDE {
    return (this->properties() & kSortedStates) ? sortedStatesNumNotTerminal_
                                                : this->maxNotTerminalStateImpl() + 1;
  }

  StateId sizeForHeads() const OVERRIDE {
    return (this->properties() & kSortedStates) ? sortedStatesNumNotTerminal_ : this->size();
  }

 private:
  /// accept value naively. post: getProperties() returns p
  virtual void setProperties(Properties p) = 0;


  template <class Arc, class SortPolicy>
  friend void sortArcsImpl(IMutableHypergraph<Arc>* hg, SortPolicy const& cmp);

  struct CmpInputFsmLabelsMatch {
    IHypergraph<Arc> const& fsm;
    explicit CmpInputFsmLabelsMatch(IHypergraph<Arc> const& fsm) : fsm(fsm) {}

    /**
       \return whether input label of fsm arc a < that of b.
    */
    template <class Arc>
    bool operator()(Arc const* b, Sym id) const {
      return fsm.inputLabel(b->fsmSymbolState()) < id;
    }

    template <class Arc>
    bool operator()(Sym id, Arc const* b) const {
      return id < fsm.inputLabel(b->fsmSymbolState());
    }

    template <class Arc>
    bool operator()(Arc const* a, Arc const* b) const {
      return fsm.inputLabel(a->fsmSymbolState()) < fsm.inputLabel(b->fsmSymbolState());
    }
  };

 public:
  /**
     \return peekable generator (really, [begin, end) iterator pair - but see
     Util/Generator.hpp) of outarcs from fsm state whose first tail's state's
     input label is equal to our request. requires kSortedOutArcs, which you can
     achieve with SortArcs.hpp sortArcs(this)

     requires isFsm()

     TODO: even more efficient version in MutableHypergraph that uses custom Cmp
     object skipping virtual interfaces?
  */
  OutArcsGenerator outArcsMatchingInput(StateId outFromState, Sym input) const {
    assert(hasProperties(this->properties(), kSortedOutArcs | kFsm));
    if (outFromState == kNoState) return OutArcsGenerator();
    ArcsContainer const* parcs = this->maybeOutArcs(outFromState);
    assert(parcs);
    ArcsContainer const& arcs = *parcs;
    if (arcs.empty()) return OutArcsGenerator();
#define SDL_OPTIMIZE_MATCHING_EPSILON_SIGMA 1
// TODO: test actual speed vs code size improvement
#if SDL_OPTIMIZE_MATCHING_EPSILON_SIGMA
    if (input <= SIGMA::ID) {
      // we always check for EPSILON and SIGMA in composition; binary search
      // would be silly since they're always at the start of outarcs.
      OutArcsIter b = arcs.begin(), i = b, e = arcs.end();
      if (input == EPSILON::ID) {  // optimize for a common query
        for (;; ++i)
          if (i == e || this->inputLabel((*i)->fsmSymbolState()) != EPSILON::ID)
            return OutArcsGenerator(b, i);
      }
      // TODO: maybe only optimize epsilon
      assert(input == SIGMA::ID);
      Sym sym;
      for (;; ++i) {
        if (i == e) return OutArcsGenerator();
        if ((sym = this->inputLabel((*i)->fsmSymbolState())) > EPSILON::ID) {
          if (sym != SIGMA::ID) return OutArcsGenerator();
          b = i;
          for (;;)
            if (++i == e || this->inputLabel((*i)->fsmSymbolState()) != SIGMA::ID)
              return OutArcsGenerator(b, i);
        }
      }
      assert(0);  // for loop is no-escape (return-only)
    }
#endif
    return OutArcsGenerator(std::equal_range(arcs.begin(), arcs.end(), input, CmpInputFsmLabelsMatch(*this)));
  }

  OutArcsGenerator outArcs(StateId outFromState) const {
    ArcsContainer const& arcs = outFromState == kNoState ? emptyArcs_ : *this->maybeOutArcs(outFromState);
    return OutArcsGenerator(arcs.begin(), arcs.end());
  }

  // remove all arcs/states+labels. only call this if you used new Arc() to create added arcs.
  virtual void clearImpl(Properties prop) = 0;

  void clear(Properties prop) {
    SDL_TRACE(Hypergraph, "clear requested prop=" << PrintProperties(prop));
    clearImpl(prop);
    Properties p = this->properties();
    SDL_TRACE(Hypergraph, "clearImpl set p=" << PrintProperties(p));
    p &= ~kHasOutputLabels;
    p |= kFsmProperties;
    if (p & kStoreAnyOutArcs) p |= kSortedOutArcs;
    this->setProperties(p);
    SDL_TRACE(Hypergraph, "clear postlude prop=" << printProperties(*this));
  }

  void clear() { this->clear(this->uncomputedProperties()); }

  void setEmptyIfNoArcs(bool addstart = true, bool addfinal = false) {
    if (this->prunedEmpty()) setEmpty(addstart, addfinal);
  }

  void setEmpty(bool addstart = false, bool addfinal = false) {
    clear();
    if (addstart) this->setStart(this->addState());
    if (addfinal) this->setFinal(this->addState());
  }

  /// order of args: fromState -> toState with (labelState, weight) tails are (toState, labelState)
  Arc* addArcFsm(StateId fromState, StateId toState, StateId labelState, Weight weight = Weight::one()) {
    Arc* a = new Arc(fromState, labelState, weight, toState);
    this->addArc(a);
    return a;
  }

  Arc* addArcGraph(StateId fromState, StateId toState, Weight weight = Weight::one()) {
    Arc* a = new Arc(toState, weight, fromState);
    this->addArc(a);
    return a;
  }

  Arc* addArcFsa(StateId from, StateId to, Sym label = EPSILON::ID, Weight w = Weight::one()) {
    Arc* a = new Arc(from, this->addState(label), w, to);
    this->addArc(a);
    return a;
  }

  Arc* addArcEpsilon(StateId from, StateId to, Weight w = Weight::one()) {
    Arc* a = new Arc(from, this->addState(EPSILON::ID), w, to);
    this->addArc(a);
    return a;
  }

  Arc* addArcGraphEpsilon(StateId from, StateId to, Weight w = Weight::one()) {
    Arc* a = new Arc(from, w, to);
    this->addArc(a);
    return a;
  }

  Arc* addArcFst(StateId from, StateId to, Sym label, Sym labelout, Weight w = Weight::one()) {
    Arc* a = new Arc(from, this->addState(LabelPair(label, labelout)), w, to);
    this->addArc(a);
    return a;
  }

  Arc* addArcFst(StateId from, StateId to, LabelPair inout, Weight w = Weight::one()) {
    Arc* a = new Arc(from, this->addState(inout), w, to);
    this->addArc(a);
    return a;
  }

  /// call after modifying an already added arc to add a tail
  virtual void addedTail(Arc* a, StateId tail) = 0;

  IMutableHypergraph() : IHypergraph<A>("IMutableHypergraph"), sortedStatesNumNotTerminal_() {}

  // function overwrite with covariant return type:
  virtual IMutableHypergraph<A>* clone() const OVERRIDE = 0;

  /// addArc after calling addStateId on head, tails.
  virtual void addArcCreatingStates(Arc*) = 0;

  void forceNotAllOutArcs() OVERRIDE {
    Properties p = this->uncomputedProperties();
    if ((p & kStoreOutArcs) && !(p & kStoreFirstTailOutArcs)) {
      this->forceFirstTailOutArcs();
    }
  }


  void forceHasArcs(bool outPreferred = false) { forceStoreArcs(outPreferred); }

  void forceStoreArcs(bool outPreferred = true) {
    if (!this->storesArcs()) forceProperties(outPreferred ? kStoreFirstTailOutArcs : kStoreInArcs);
  }

  void forceOnlyProperties(Properties properties) {
    forceProperties(properties, kAllProperties - properties);
  }
  void forceProperties(Properties properties, bool on = true) {
    if (on)
      forcePropertiesOnOff(properties, 0);
    else
      forcePropertiesOnOff(0, properties);
  }

  /**
     try to activate the bits in 'on' and then deactivate those in off.

     TODO: this should be outside of IMutableHypergraph because it calls sort,
     and we don't want to depend on SortArcs.hpp for everything

     TODO: does this handle the new kStoreFirstTailOutArcs as being exclusive of
     kStoreOutArcs?
  */
  virtual void forcePropertiesOnOff(Properties on, Properties off) {
    if (on & off)
      SDL_THROW_LOG(Hypergraph.forceProperties, ConfigException, "you asked to both enable and disable "
                                                                     << PrintProperties(on & off));
    if (unknownPropertyBits(on))
      SDL_THROW_LOG(Hypergraph.forceProperties, ConfigException,
                    "you asked to turn on undefined property bits " << PrintProperties(on));
    if (unknownPropertyBits(off))
      SDL_THROW_LOG(Hypergraph.forceProperties, ConfigException,
                    "you asked to turn off undefined property bits " << PrintProperties(off));
    Properties p = this->properties();
    if (this->prunedEmpty()) {  // empty, who cares - any props are fine
      this->setEmpty();
      this->setProperties(on | (p & ~off));
      return;
    }
    Properties add = on & ~p;
    if (add) {
      // TODO: we could automatically call SortStates, but it would be hard to make it compile due to
      // dependencies, i think. for now i've friended so SortStates can call setProperties
      if (add & kSortedStates) this->setProperties(p |= kSortedStates);
      if (add & kStoreInArcs) this->forceInArcs();
      if (add & kStoreOutArcs) {
        if (add & kStoreFirstTailOutArcs)
          SDL_THROW_LOG(IMutableHypergraph.forceProperties, ProgrammerMistakeException,
                        "Can't have both first-tail-only and all-tails out-arcs");
        this->forceOutArcs();
      }
      add = on & ~p;
      /* I think we want kStoreFirstTailOutArcs to be satisfied by kStoreArcsPerState. that's easiest achieved
       * by setting StoreFirstTailOut whenever we have StoreOut - and means we shouldn't forceFirstTailOutArcs
       * if we already called forceOutArcs. thus checking properties after. */
      if (add & kStoreFirstTailOutArcs) this->forceFirstTailOutArcs();
      if (add & (kFsm | kGraph)) this->checkGraph();
      if (add & kSortedOutArcs)  // NOTE: if you add arcs, won't remain sorted. so in
        // copyHypergraphWithProperties we redundantly force the property twice.
        // adding arcs clears it no matter what.
        sortArcs(this);
      if (add & kOutArcsSortedBestFirst) {
        if (add & kSortedOutArcs)
          SDL_THROW_LOG(IMutableHypergraph.forceProperties, ConfigException,
                        "can't have both sorted and best-first out arcs");
        this->clearProperties(kSortedOutArcs);
        this->forceBestFirstArcs();
      }
      if ((add & kCanonicalLex) && !this->ensureCanonicalLex()) {
        SDL_DEBUG(Hypergraph, "in setting kCanonicalLex, found duplicate states with same labelpair");
      }
      p = this->properties();
      add = on & ~p;
      if (add)
        SDL_THROW_LOG(Hypergraph, InvalidInputException, "Don't know how to add properties "
                                                             << PrintProperties(add) << " in forceProperties("
                                                             << PrintProperties(on) << ", ON); ended up with "
                                                             << PrintProperties(p));
    }
    Properties remove = off & p;
    if (remove) {
      if (remove & kStoreInArcs) this->removeInArcs();
      bool const removeOut = remove & kStoreOutArcs;
      bool const removeFirstOut = remove & kStoreFirstTailOutArcs;
      bool const haveOut = p & kStoreOutArcs;
      bool const haveFirstOut = p & kStoreFirstTailOutArcs;
      if (removeOut && !haveFirstOut || removeFirstOut && !haveOut) this->removeOutArcs();
      if (removeFirstOut) this->setProperties(p & ~kStoreFirstTailOutArcs);
      if (remove & kCanonicalLex) this->clearCanonicalLex();
      if (remove & kSortedStates) this->setProperties(p & ~kSortedStates);
      remove = off & this->properties();
      if (remove)
        SDL_THROW_LOG(Hypergraph, InvalidInputException, "Don't know how to remove properties "
                                                             << PrintProperties(remove)
                                                             << " in forceProperties(" << PrintProperties(off)
                                                             << ", OFF)");
    }
    assert(this->storesArcs());
  }

  void forceModifiableLabelStates() {
    if (this->storesAllOutArcs()) {
      if (this->isGraph())
        this->forceFirstTailOutArcs();
      else {
        this->forceInArcs();
        this->removeOutArcs();
      }
    }
  }

  void removeOutArcsMaybeAddingFirstTail() {
    Properties p = this->properties();
    if (p & kStoreOutArcs) {
      if (p & kStoreInArcs)
        this->removeOutArcs();
      else
        this->forceFirstTailOutArcs();
    }
  }

  void projectInput() {
    this->removeOutputLabels();
    this->clearProperties(kHasOutputLabels);
  }

  using IHypergraph<Arc>::inArcs;
  using IHypergraph<Arc>::outArcs;

  /// you may call this before getting ArcsContainer * and be sure that adding
  /// arcs touching states < size won't invalidate. if size is kNoState then
  virtual void prepareAddArcsSize(StateId size) = 0;
  virtual void prepareAddArcs() { prepareAddArcsSize(this->sizeForHeads()); }

  void outAdjStates(StateId st, StateIdContainer& adjStates) const OVERRIDE {
    ArcsContainer const* a = this->maybeOutArcs(st);
    if (a) {
      ArcId N = a->size();
      adjStates.resize(N);
      for (unsigned i = 0; i < N; ++i) adjStates[i] = (*a)[i]->head_;
    }
  }

  /// if any arcs were modified structurally (or states labels changed), you
  /// should call this so kGraph and kFsm will be recomputed properly
  virtual void notifyArcsModified() = 0;

  /// faster than IHypergraph::forArcs
  template <class V>
  void forArcs(V const& v) const {
    Properties p = this->properties();
    bool inarcs = p & kStoreInArcs;
    if (!inarcs)
      if (!(p & kStoreFirstTailOutArcs)) return IHypergraph<Arc>::forArcs(v);
    for (StateId s = 0, N = this->size(); s < N; ++s) {
      ArcsContainer const* p = this->maybeArcsConst(s, inarcs);
      if (!p) continue;
      for (ArcsContainer::const_iterator i = p->begin(), e = p->end(); i != e; ++i) v((Arc*)*i);
    }
  }

  template <class V>
  void forArcs(StateId s, V const& v) const {
    Properties p = this->properties();
    bool inarcs = p & kStoreInArcs;
    for (StateId s = 0, N = this->size(); s < N; ++s) {
      ArcsContainer const* p = this->maybeArcsConst(s, inarcs);
      if (!p) continue;
      for (ArcsContainer::const_iterator i = p->begin(), e = p->end(); i != e; ++i) v((Arc*)*i);
    }
  }

  /**
     \return whether projectOutput is a noop, i.e. that the output symbols will change when the input symbols
     are updated.
  */
  virtual bool emptyOutputLabels() const { return false; }

  /// remove input labels (for all labeled states)
  void projectOutput() {
    if (!emptyOutputLabels()) {
      visitLabeledStates(*this, SetInLabelToOut());
      this->removeOutputLabels();
    }
    this->clearProperties(kHasOutputLabels);
  }

  // for now this is free access. setting the property doesn't enforce it (unlike forceProperties, which makes
  // an effort
  void addProperties(Properties p) { setProperties(this->properties() | p); }
  void clearProperties(Properties p) { setProperties(this->properties() & ~p); }

  /**
     compute whether hg is a graph, updating properties used to update cached
     kGraph properties. since we can update kFsm at the same time for free (kFsm
     => kGraph), we do that too
  */
  virtual bool checkGraph() {
    bool fsm;
    bool one;
    bool graph = this->isGraphCheck(fsm, one);
    this->setFsmGraphOneLexical(fsm, graph, one);
    return graph;
  }

  void setPropertiesAt(Properties bits, bool on) {
    if (on)
      addProperties(bits);
    else
      clearProperties(bits);
  }

  void setAllStrings(Sym sigma = RHO::ID) {
    // whether you use RHO or SIGMA doesn't matter since there will be only 1
    // arc, except that you should prefer rho since determinize lacks support for sigma.
    clear();
    StateId state = this->addState();
    this->setStart(state);
    this->setFinal(state);
    this->addArcFsa(state, state, sigma);
  }

  // if pcont = 1-pstop, then this has a length model built in (however, all strings get the same prob as if
  // vocab had 1 symbol)
  void setAllStringsWeighted(Sym sigma = RHO::ID, Weight pcont = Weight::one(), Weight pstop = Weight::one()) {
    clear();
    StateId state = this->addState();
    StateId f = this->addState();
    this->setStart(state);
    this->setFinal(f);
    this->addArcFsa(state, state, sigma, pcont);
    this->addArcFsa(state, f, EPSILON::ID, pstop);
  }

  virtual StateId ensureStart() {
    StateId s = this->start();
    if (s == kNoState) this->setStart(s = this->addState());
    return s;
  }

 protected:
  virtual void removeOutputLabels() = 0;

  /// for prunedEmpty() - i.e., start=final=kNoState, algs may still ask for outarcs of start()
  ArcsContainer emptyArcs_;

 private:
  struct SetInLabelToOut {
    template <class H, class SI>
    bool operator()(H const& hg, SI state) const {
      const_cast<Self&>(static_cast<Self const&>(hg)).setInputLabel(state, hg.outputLabel(state));
      return true;
    }
  };
};

template <class Arc>
IMutableHypergraph<Arc> const& mutableHg(IHypergraph<Arc> const& hg) {
  assert(hg.isMutable());
  return static_cast<IMutableHypergraph<Arc> const&>(hg);
}

template <class Arc>
IMutableHypergraph<Arc> const& mutableHg(IMutableHypergraph<Arc> const& hg) {
  return hg;
}

template <class Arc>
IMutableHypergraph<Arc>& mutableHg(IHypergraph<Arc>& hg) {
  assert(hg.isMutable());
  return static_cast<IMutableHypergraph<Arc>&>(hg);
}

template <class Arc>
IMutableHypergraph<Arc>& mutableHg(IMutableHypergraph<Arc>& hg) {
  return hg;
}

/**
   throw exception unless properties are already (on|off), unless hg is mutable, in which case attempt to
   mutableHg.forceProperties(properties, on).
*/
template <class Arc>
inline void forcePropertiesIfMutable(IHypergraph<Arc>& hg, Properties properties, bool on = true) {
  if (hasProperties(hg.properties(), properties, on)) return;
  if (!hg.isMutable())
    SDL_THROW_LOG(
        Hypergraph.forcePropertiesIfMutable, ConfigException,
        "you may not force properties "
            << PrintProperties(properties) << (on ? " on" : " off")
            << " for immutable hg - perhaps you can configure your pipeline to produce directly the "
               "correct properties, or insert a step that first converts to mutable hg");
  static_cast<IMutableHypergraph<Arc>&>(hg).forceProperties(properties, on);
}

/**
   return final state (existing final state, else create new one)
*/
inline StateId ensureFinal(HypergraphBase& hg) {
  assert(hg.isMutable());
  StateId s = hg.final();
  if (s == kNoState) {
    s = hg.addState();
    hg.setFinal(s);
  }
  return s;
}

inline void forceInArcs(HypergraphBase& hg, char const* prefix = "input") {
  if (!hg.storesInArcs()) {
    if (hg.isMutable())
      hg.forceInArcs();
    else
      SDL_THROW_LOG(Hypergraph.forceInArcs, ConfigException, prefix << " hg doesn't have inarcs");
  }
}

inline void forceFirstTailOutArcs(HypergraphBase& hg, char const* prefix = "input") {
  if (!hg.storesOutArcs()) {
    if (hg.isMutable())
      hg.forceFirstTailOutArcs();
    else
      SDL_THROW_LOG(Hypergraph.forceInArcs, ConfigException, prefix << " hg doesn't have (graph) out arcs");
  }
}

inline void forceInArcsOnly(HypergraphBase& hg) {
  if (!hg.storesInArcs()) {
    if (hg.isMutable()) {
      hg.forceInArcs();
      hg.removeOutArcs();
    } else
      SDL_THROW_LOG(Hypergraph.forceInArcs, ConfigException, "input hg doesn't have inarcs");
  }
}


}}

#endif
