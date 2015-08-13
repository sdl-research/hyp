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
   The part of IMutableHypergraph<Arc> that doesn't depend on Arc.

   Because there's no cheap (nonvirtual) diamond inheritance, we have to call
   IHypergraphBase in a slightly verbose manner.
*/
struct IMutableHypergraphBase {

  virtual void removeDeletedArcs(Util::PointerSet const& deletedArcPointers) = 0;

  /// remove kCanonicalLex property
  virtual void clearCanonicalLex() = 0;

  /// leave kCanonicalLex alone (may still be set) but clear cache so currently existing states won't be
  /// reused
  virtual void clearCanonicalLexCache() = 0;

  /// return true if no duplicate states with same labelpair, setting kCanonicalLex and rebuilding cache
  virtual bool forceCanonicalLex() = 0;

  /// return true if no duplicate states with same labelpair. invalidate existing cache
  virtual bool ensureCanonicalLex() = 0;

  /**
     \return existing StateId with given label pair, if any. may only call if canonicalLex()
  */
  virtual StateId canonicalExistingStateForLabelPair(LabelPair io) = 0;
  /**
     reserve at least this many states (optional; for efficiency - not required to call with an upper bound,
     or at all).
  */
  virtual void reserve(StateId) {}

  /** an output symbol of NoSymbol means the output changes when you
      setInputLabel later; an explicit same-symbol will fix the output label
      against that eventuality.
  */
  virtual void setLabelPair(StateId state, LabelPair io) = 0;

  void setLabelPair(StateId state, Sym in, Sym out = NoSymbol) { setLabelPair(state, LabelPair(in, out)); }

  virtual StateId addState() { return addStateId(nextStateId()); }

  // optional to call this - might clear out some garbage. won't set
  // kCanonicalLex if it isn't set already (use forceCanonicalLex for
  // that). \return whether labels are in fact canonical
  virtual bool rebuildCanonicalLex() = 0;

  /// respects kCanonicalLex property
  virtual StateId addState(Sym inputLabel) = 0;

  /**
     addStateId(nextStateId(), ...) is the same as addState(, ...) always.
  */
  virtual StateId nextStateId() const = 0;

  /**
     remove last addState provided no arcs using it have been added
  */
  virtual void removeLastState() = 0;

  /// note: if state was already added, it keeps existing arcs/labels. returns state always - cannot fail
  virtual StateId addStateId(StateId state) = 0;

  virtual void addStateId(StateId state, LabelPair l) {
    addStateId(state);
    setLabelPair(state, l);
  }

  virtual StateId addStateId(StateId stateId, Sym inputLabel, Sym outputLabel) = 0;

  virtual StateId addState(Sym inputLabel, Sym outputLabel) = 0;

  StateId addState(LabelPair io) { return addState(input(io), output(io)); }

  virtual void setInputLabel(StateId state, Sym label) = 0;

  // Remember, setting to NoSymbol (or never setting) means output is
  // same as input.
  virtual void setOutputLabel(StateId state, Sym label) = 0;

  virtual void setVocabulary(IVocabularyPtr const&) = 0;
  void setVocabulary(IPerThreadVocabulary& perThreadVocab) {
    setVocabulary(perThreadVocab.getPerThreadVocabulary());
  }

  /**
     Add existing labels to new vocab, and setVocabulary(vocab).
  */
  void translateToVocabulary(IVocabularyPtr const& newVocab);

  /// make our hg have the same vocab as another
  template <class A>
  void takeVocabulary(IHypergraph<A> const& h) {
    this->setVocabulary(h.getVocabulary());
  }

  // workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual bool hgOutputLabelFollowsInput(StateId state) const = 0;

  virtual bool hgOutputLabelFollowsInput() const = 0;

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual StateId hgGetNumStates() const = 0;

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual LabelPair hgGetLabelPair(StateId state) const = 0;

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual Sym hgGetInputLabel(StateId state) const = 0;

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual IVocabularyPtr hgGetVocabulary() const = 0;

  /// workaround avoiding virtual inheritance for purposes of Invert
  virtual Properties hgProperties() const = 0;
};

/// lets you grab fresh states and return unwanted states back to a free pool
struct AddStateId {
  std::vector<StateId> pool;
  IMutableHypergraphBase& hg;

  AddStateId(IMutableHypergraphBase& hg) : hg(hg) {}

  void addStateId(StateId s) {
    for (StateId from = hg.hgGetNumStates(); from < s; ++from) pool.push_back(from);
    hg.addStateId(s);
  }

  StateId addState() {
    if (pool.empty()) {
      return hg.addState();
    } else {
      StateId r = pool.back();
      pool.pop_back();
      return r;
    }
  }

  void setSameTerminals(IHypergraphStates const& hgIn) {
    for (StateId s = 0, n = hgIn.size(); s < n; ++s) {
      if (hgIn.hasTerminalLabel(s)) {
        addStateId(s);
        hg.setLabelPair(s, hgIn.labelPair(s));
      }
    }
  }
};

/**
   An IHypergraph you can modify. In practice this is always a
   MutableHypergraph, which uses arrays to store things.
*/
template <class A>
struct IMutableHypergraph : IHypergraph<A>, IMutableHypergraphBase {

  void setHaveConstraints(bool haveConstraintStarts, bool haveConstraintEnds) {
    if (haveConstraintEnds && !haveConstraintStarts)
      SDL_THROW_LOG(Hypergraph.Constraints, ConfigException,
                    "we always have constraint start-states if we have constraints, and sometimes have "
                    "constraint end-states - so haveConstraintEnds && !haveConstraintStarts is an error");
    setPropertyBit(kConstraintStarts, haveConstraintStarts);
    setPropertyBit(kConstraintEnds, haveConstraintEnds);
  }

  void setStart(StateId s) { this->start_ = s; }

  void setFinal(StateId s) { this->final_ = s; }

 protected:
  // we hope all these are implemented extremely efficiently (by cloning part of
  // vtable). but (see MutableHypergraph.hpp) we can also directly implement in
  // terms of non-virtual shared subclass impl just in case

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual bool hgOutputLabelFollowsInput(StateId state) const OVERRIDE {
    return this->outputLabelFollowsInput(state);
  }

  virtual bool hgOutputLabelFollowsInput() const OVERRIDE { return this->outputLabelFollowsInput(); }

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual StateId hgGetNumStates() const OVERRIDE { return this->size(); }

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual LabelPair hgGetLabelPair(StateId state) const OVERRIDE { return this->labelPair(state); }

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual Sym hgGetInputLabel(StateId state) const OVERRIDE { return this->inputLabel(state); }

  virtual Properties hgProperties() const OVERRIDE { return this->properties(); }

  /// workaround avoiding virtual inheritance for purposes of translateToVocabulary
  virtual IVocabularyPtr hgGetVocabulary() const OVERRIDE { return this->getVocabulary(); }

  friend struct SortStates<A>;
  StateId sortedStatesNumNotTerminal_;  // this is set only if properties() & kSortedStates

 public:
  typedef IHypergraph<A> Base;
  typedef typename A::ArcFilter ArcFilter;
  typedef IMutableHypergraph<A> Self;
  typedef shared_ptr<Self> Ptr;
  typedef A Arc;
  typedef typename A::Weight Weight;
  typedef typename Base::ArcsContainer ArcsContainer;
  typedef typename ArcsContainer::const_iterator OutArcsIter;
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
      if (arcs) std::sort(arcs->begin(), arcs->end(), CmpByWeight());
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


  /**
     if on, set bit, else clear it. does not do anything except change the
     property bit value so should be used by library writers only.

     TODO: make protected?
  */
  virtual void setPropertyBit(Properties bit, bool on = true) {
    Properties p = this->hgUncomputedProperties();
    setProperties(on ? (bit | p) : (~bit & p));
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
    if (p & kStoresAnyOutArcs) p |= kSortedOutArcs;
    this->setProperties(p);
    SDL_TRACE(Hypergraph, "clear postlude prop=" << printProperties(*this));
  }

  void clear() { clear(this->hgUncomputedProperties()); }

  void setEmptyIfNoArcs(bool addstart = true, bool addfinal = false) {
    if (this->prunedEmpty()) setEmpty(addstart, addfinal);
  }

  void setEmpty(bool addstart = false, bool addfinal = false) {
    clear();
    if (addstart) setStart(addState());
    if (addfinal) setFinal(addState());
  }

  /// order of args: fromState -> toState with (labelState, weight) tails are (toState, labelState)
  Arc* addArcFsm(StateId fromState, StateId toState, StateId labelState, Weight weight = Weight::one()) {
    Arc* a = new Arc(fromState, labelState, weight, toState);
    addArc(a);
    return a;
  }

  Arc* addArcGraph(StateId fromState, StateId toState, Weight weight = Weight::one()) {
    Arc* a = new Arc(toState, weight, fromState);
    addArc(a);
    return a;
  }

  Arc* addArcFsa(StateId from, StateId to, Sym label = EPSILON::ID, Weight w = Weight::one()) {
    Arc* a = new Arc(from, addState(label), w, to);
    addArc(a);
    return a;
  }

  Arc* addArcEpsilon(StateId from, StateId to, Weight w = Weight::one()) {
    Arc* a = new Arc(from, addState(EPSILON::ID), w, to);
    addArc(a);
    return a;
  }

  Arc* addArcGraphEpsilon(StateId from, StateId to, Weight w = Weight::one()) {
    Arc* a = new Arc(from, w, to);
    addArc(a);
    return a;
  }

  Arc* addArcFst(StateId from, StateId to, Sym label, Sym labelout, Weight w = Weight::one()) {
    Arc* a = new Arc(from, addState(LabelPair(label, labelout)), w, to);
    addArc(a);
    return a;
  }

  Arc* addArcFst(StateId from, StateId to, LabelPair inout, Weight w = Weight::one()) {
    Arc* a = new Arc(from, addState(inout), w, to);
    addArc(a);
    return a;
  }

  /// call after modifying an already added arc to add a tail
  virtual void addedTail(Arc* a, StateId tail) = 0;

  IMutableHypergraph() : IHypergraph<A>("sdl::IMutableHypergraph"), sortedStatesNumNotTerminal_() {}

  // function overwrite with covariant return type:
  virtual IMutableHypergraph<A>* clone() const OVERRIDE = 0;

  /**
     should set kArcsAdded property so we can detect acyclic, fsm, etc.
  */
  virtual void addArc(Arc*) = 0;

  void forceHasArcs(bool outPreferred = false) { forceStoreArcs(outPreferred); }

  void forceStoreArcs(bool outPreferred = true) {
    if (!this->storesArcs()) forceProperties(outPreferred ? kStoreFirstTailOutArcs : kStoreInArcs);
  }

  void forceFirstTailOutArcsOnly() {
    this->forceFirstTailOutArcs();
    removeInArcs();
  }

  void forceNotAllOutArcs() OVERRIDE {
    Properties p = this->hgUncomputedProperties();
    if ((p & kStoreOutArcs) && !(p & kStoreFirstTailOutArcs)) {
      this->forceFirstTailOutArcs();
    }
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
      if (add & kStoreInArcs) forceInArcs();
      if (add & kStoreOutArcs) {
        if (add & kStoreFirstTailOutArcs)
          SDL_THROW_LOG(IMutableHypergraph.forceProperties, ProgrammerMistakeException,
                        "Can't have both first-tail-only and all-tails out-arcs");
        forceOutArcs();
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
      if ((add & kCanonicalLex) && !ensureCanonicalLex()) {
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

  virtual void forceInArcs() = 0;

  virtual void forceOutArcs() = 0;

  virtual void removeInArcs() = 0;

  virtual void removeOutArcs() = 0;

  void forceModifiableLabelStates() {
    if (this->storesAllOutArcs()) {
      if (this->isGraph())
        this->forceFirstTailOutArcs();
      else {
        forceInArcs();
        removeOutArcs();
      }
    }
  }

  void removeOutArcsMaybeAddingFirstTail() {
    Properties p = this->properties();
    if (p & kStoreOutArcs) {
      if (p & kStoreInArcs)
        removeOutArcs();
      else
        this->forceFirstTailOutArcs();
    }
  }

  void projectInput() {
    removeOutputLabels();
    this->clearProperties(kHasOutputLabels);
  }

  using IHypergraph<Arc>::inArcs;
  using IHypergraph<Arc>::outArcs;

  /// you may call this before getting ArcsContainer * and be sure that adding
  /// arcs touching states < size won't invalidate. if size is kNoState then
  virtual void prepareAddArcsSize(StateId size) = 0;
  virtual void prepareAddArcs() { prepareAddArcsSize(this->sizeForHeads()); }

  /**
     these may return NULL instead of empty. (the const versions will return a
     pointer to empty instead)

     you should assume that these pointers are invalidated whenever calling
     addArcs with bigger stateids than before, in the relevant position (head
     for in, first tail for first-tail-out-arcs, all tails for out-arcs))

  */
  virtual ArcsContainer* maybeInArcs(StateId state) = 0;

  virtual ArcsContainer const* maybeInArcs(StateId state) const = 0;

  virtual ArcsContainer* maybeOutArcs(StateId state) = 0;

  void outAdjStates(StateId st, StateIdContainer& adjStates) const OVERRIDE {
    ArcsContainer const* a = maybeOutArcs(st);
    if (a) {
      ArcId N = a->size();
      adjStates.resize(N);
      for (unsigned i = 0; i < N; ++i) adjStates[i] = (*a)[i]->head_;
    }
  }

  virtual ArcsContainer const* maybeOutArcs(StateId state) const = 0;

  ArcsContainer const* maybeInArcsConst(StateId state) const { return maybeInArcs(state); }

  ArcsContainer const* maybeOutArcsConst(StateId state) const { return maybeOutArcs(state); }

  ArcsContainer const* maybeArcsConst(StateId state, bool inarcs) const {
    return inarcs ? maybeInArcs(state) : maybeOutArcs(state);
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
      ArcsContainer const* p = maybeArcsConst(s, inarcs);
      if (!p) continue;
      for (typename ArcsContainer::const_iterator i = p->begin(), e = p->end(); i != e; ++i) v(*i);
    }
  }

  template <class V>
  void forArcs(StateId s, V const& v) const {
    Properties p = this->properties();
    bool inarcs = p & kStoreInArcs;
    for (StateId s = 0, N = this->size(); s < N; ++s) {
      ArcsContainer const* p = maybeArcsConst(s, inarcs);
      if (!p) continue;
      for (typename ArcsContainer::const_iterator i = p->begin(), e = p->end(); i != e; ++i) v(*i);
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
     instead of checkGraph, trust the caller.
  */
  virtual void setFsmGraphOneLexical(bool fsm, bool graph, bool one) {
    setPropertiesAt(kGraph, graph);
    setPropertiesAt(kFsm, fsm);
    setPropertiesAt(kOneLexical, one);
  }

  /**
     compute whether hg is a graph, updating properties used to update cached
     kGraph properties. since we can update kFsm at the same time for free (kFsm
     => kGraph), we do that too
  */
  virtual bool checkGraph() {
    bool fsm;
    bool one;
    bool graph = this->isGraphCheck(fsm, one);
    setPropertiesAt(kGraph, graph);
    setPropertiesAt(kFsm, fsm);
    setPropertiesAt(kOneLexical, one);
    return graph;
  }

  void setPropertiesAt(Properties bits, bool on) {
    if (on)
      addProperties(bits);
    else
      clearProperties(bits);
  }

  void setAllStrings(Sym sigma
                     = RHO::ID) {  // whether you use RHO or SIGMA doesn't matter since there will be only 1
    // arc. HOWEVER determinize only supports rho now, not sigma
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
    if (s == kNoState) setStart(s = addState());
    return s;
  }

 protected:
  virtual void removeOutputLabels() = 0;
  ArcsContainer emptyArcs_;  // for start=final=kNoState, fs/Compose/Context etc may still ask for outarcs of
  // start state

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
template <class Arc>
StateId ensureFinal(IMutableHypergraph<Arc>& hg) {
  StateId s = hg.final();
  if (s == kNoState) {
    s = hg.addState();
    hg.setFinal(s);
  }
  return s;
}

template <class Arc>
inline void forceInArcs(IHypergraph<Arc>& hg, char const* prefix = "input") {
  if (!hg.storesInArcs()) {
    IMutableHypergraph<Arc>* mhg = dynamic_cast<IMutableHypergraph<Arc>*>(&hg);
    if (mhg)
      mhg->forceInArcs();
    else
      SDL_THROW_LOG(Hypergraph.forceInArcs, ConfigException, prefix << " hg doesn't have inarcs");
  }
}

template <class Arc>
inline void forceFirstTailOutArcs(IHypergraph<Arc>& hg, char const* prefix = "input") {
  if (!hg.storesOutArcs()) {
    IMutableHypergraph<Arc>* mhg = dynamic_cast<IMutableHypergraph<Arc>*>(&hg);
    if (mhg)
      mhg->forceFirstTailOutArcs();
    else
      SDL_THROW_LOG(Hypergraph.forceInArcs, ConfigException, prefix << " hg doesn't have (graph) out arcs");
  }
}

template <class Arc>
inline void forceInArcsOnly(IHypergraph<Arc>& hg) {
  if (!hg.storesInArcs()) {
    IMutableHypergraph<Arc>* mhg = dynamic_cast<IMutableHypergraph<Arc>*>(&hg);
    if (mhg) {
      mhg->forceInArcs();
      mhg->removeOutArcs();
    } else
      SDL_THROW_LOG(Hypergraph.forceInArcs, ConfigException, "input hg doesn't have inarcs");
  }
}


}}

#endif
