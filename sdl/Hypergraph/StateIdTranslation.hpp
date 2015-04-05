// Copyright 2014-2015 SDL plc
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

    StateIdMapping: function that maps original stateids to destination hg stateids, for either
   HypergraphCopy.hpp copy() or IMutableHypergraph restrict()

    used instead of boost::function<StateId (StateID)> for metadata e.g. some transformations are
   stateAdding() and some aren't, and performance

    StateIdTranslation: caches StateIdMapping so we don't overgenerate states (in case StateIdMapping is lazy)

    design rationale - uses are:

    1. compress stateid space while deleting some states, e.g. those with no, or
   low-relative-to-best-probability, paths

    2. sort or partition states (no deletion)

    3. concatenate arcs for union or string concatenation, keeping stateid separate

    the mapping can be lazy or eager:

    A. lazy (allocating starting with max stateid+1 of HG you're copying to) - then we'd want a
   boost::function or whatever, for anything beyond a simple "contiguous as needed" or "fixed offset" (status
   quo)

    B. eager (populate mapping in advance)

    and in-place or copy:

    x. copy: need to add states for range of translation.

    y. in-place: currently, in-place stores a list of all arcs, then clears the resulting HG's adjacencies,
   then transforms labels, start/final state. so, even though the mapping is populated, you need to add states

    and finally, for lazy+pass HG only: kCanonicalLex in resulting transducer - make concatenated share
   lexical states, etc.

    awkwardness:

    Ax (lazy+copy) - either before adding arc create all the states, or when generating the mapping, add the
   state (status quo)

    By (eager+in-place) - for in-place, we don't want states added at all - we haven't removed them yet.
   option: move initialization into post-clear (requires type-erased boost::function etc for generating the
   mapping)

    kCanonicalLex : in-place: if you want same state for same labelpair, then build the original hg w/
   kCanonicalLex. copy: no problem.

*/

#ifndef STATEIDTRANSLATION_LW20111212_HPP
#define STATEIDTRANSLATION_LW20111212_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/FwdDecls.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/graehl/shared/os.hpp>
#include <boost/noncopyable.hpp>

namespace sdl {
namespace Hypergraph {

typedef unordered_map<StateId, StateId> StateIdMap;
// TODO: use vector? kNoState for unset

// map stateids from source to copy or compacted hg. this is either a lazy complete mapping or partial

// TODO: version of this that supports kCanonicalLex

// TODO: use dynamically expanded, or prealloced, vector w/ explicit Constants::kNoState mapping - more
// efficient except in very sparse cases

// note: for 2-best and worse, you don't have the same derivation for each instance of a vertex, necessarily.
// so this might not make sense

struct trivialTag {
  trivialTag() {}
};

namespace {
trivialTag const trivial;
}

struct StateIdMapping {
  virtual char const* name() const { return "StateIdMapping"; }
  virtual bool stateAdding() const { return false; }
  virtual bool identity() const { return false; }
  virtual bool partial() const { return false; }
  /// may return kNoState (partial mapping)
  virtual StateId remap(StateId s) = 0;
  /// may return kNoState; call only for lexical labelpairs (the ones that kCanonicalLex affects)
  virtual StateId remapForLabelPair(StateId s, LabelPair const& io) { return remap(s); }

  virtual ~StateIdMapping() {}
};

typedef shared_ptr<StateIdMapping> StateIdMappingPtr;

struct IdentityIdMapping : public StateIdMapping {
  virtual char const* name() const OVERRIDE { return "IdentityIdMapping"; }
  virtual bool identity() const OVERRIDE { return true; }

  virtual StateId remap(StateId s) OVERRIDE { return s; }
  //  static IdentityIdMapping canonical;
};

struct OffsetIdMapping : public StateIdMapping {
  virtual char const* name() const OVERRIDE { return "OffsetIdMapping"; }
  virtual bool identity() const OVERRIDE { return !offset; }

  virtual StateId remap(StateId s) OVERRIDE { return s + offset; }
  StateId offset;
  OffsetIdMapping(StateId offset) : offset(offset) {}
};

struct ConsecutiveIdMapping : public StateIdMapping {
  virtual char const* name() const OVERRIDE { return "ConsecutiveIdMapping"; }

  virtual StateId remap(StateId s) OVERRIDE { return next++; }
  StateId next;
  ConsecutiveIdMapping(StateId next = 0) : next(next) {}
};

// TODO: common base for IMutableHypergraph for non-arc parts? e.g. addState* methods
// NOTE: these add using the usual interface, so when used in-place we have to copy arcs/labels then clear.
template <class A>
struct StateAddMapping : public StateIdMapping {
  virtual char const* name() const OVERRIDE { return "StateAddMapping"; }
  virtual bool stateAdding() const OVERRIDE { return true; }

  IMutableHypergraph<A>* outHg;
  StateAddMapping(IMutableHypergraph<A>* outHg) : outHg(outHg) {}
  virtual StateId remap(StateId s) OVERRIDE {
    StateId r = outHg->addState();
    SDL_TRACE(Hypergraph.StateIdTranslation,
              "StateAddMapping remap outhg=" << outHg << " s=" << s << " -> r=" << r
                                             << " numStates=" << outHg->size());
    return r;
  }
  virtual StateId remapForLabelPair(StateId s, LabelPair const& io) OVERRIDE {
    return outHg->addState(io);
  }
};

template <class A>
struct StateAddIdentityMapping : public StateIdMapping {
  virtual char const* name() const OVERRIDE { return "StateAddMapping"; }
  virtual bool stateAdding() const OVERRIDE { return true; }

  IMutableHypergraph<A>* outHg;
  StateAddIdentityMapping(IMutableHypergraph<A>* outHg) : outHg(outHg) {}
  // we don't declare this as identity because we want to make sure remap is actually called
  virtual StateId remap(StateId s) OVERRIDE {
    StateId r = outHg->addStateId(s);
    SDL_TRACE(Hypergraph.StateIdTranslation,
              "StateAddMapping remap outhg=" << outHg << " s=" << s << " -> r=" << r
                                             << " numStates=" << outHg->size());
    assert(r == s);
    return s;
  }
  virtual StateId remapForLabelPair(StateId s, LabelPair const& io) OVERRIDE {
    return outHg->addState(io);
  }
};

template <class A>
struct SubsetStateAddMapping : public StateAddMapping<A> {
  virtual char const* name() const OVERRIDE { return "SubsetStateAddMapping"; }

  virtual bool partial() const OVERRIDE { return true; }

  StateSet const& subset;
  SubsetStateAddMapping(IMutableHypergraph<A>* outHg, StateSet const& subset)
      : StateAddMapping<A>(outHg), subset(subset) {}
  virtual StateId remap(StateId s) OVERRIDE { return remapImpl(s); }
  inline StateId remapImpl(StateId s) {
    return Util::contains(subset, s) ? this->outHg->addState() : kNoState;
  }
  virtual StateId remapForLabelPair(StateId s, LabelPair const& io) OVERRIDE {
    return Util::contains(subset, s) ? this->outHg->addState(io) : kNoState;
  }
};

struct SubsetIdentityMapping : public StateIdMapping {
  virtual char const* name() const OVERRIDE { return "SubsetIdentityMapping"; }

  virtual bool partial() const OVERRIDE { return true; }

  StateSet const& subset;
  SubsetIdentityMapping(StateSet const& subset) : subset(subset) {}
  virtual StateId remap(StateId s) OVERRIDE { return remapImpl(s); }
  inline StateId remapImpl(StateId s) { return Util::contains(subset, s) ? s : kNoState; }
};

struct StateIdTranslation : boost::noncopyable {
  StateIdMap cache;
  typedef StateIdTranslation self_type;

 private:
  StateIdMapping* map;
  StateIdMappingPtr impl;
  bool partialMap, identityMap, stateAddingMap;

 public:
  std::size_t size() const { return cache.size(); }
  bool partial() const { return partialMap; }
  bool identity() const { return identityMap; }
  bool stateAdding() const { return stateAddingMap; }
  StateIdTranslation(trivialTag) { setIdentity(); }
  /// pointer is from new! will be deleted
  StateIdTranslation(StateIdMapping* map_ = 0) { resetNew(map_); }
  /// reference: not deleted
  StateIdTranslation(StateIdMapping& map_) { reset(&map_); }
  /// 0 pointer gets Identity mapping
  StateIdTranslation(StateIdMappingPtr const& p) : impl(p) {
    if (!impl) impl.reset(new IdentityIdMapping());
    reset(impl.get());
  }
  operator bool() const { return map; }
  void reset(StateIdMapping* map_) {
    map = map_;
    if (map) {
      partialMap = map->partial();
      identityMap = map->identity();
      stateAddingMap = map->stateAdding();
    }
    cache.clear();
    frozenMap = false;  // if true, partial mapping and cache is authoratitive.
  }
  void resetNew(StateIdMapping* map_) {
    impl.reset(map_);
    reset(map_);
  }
  void clear() { resetNew(0); }

  /// adds states to o in order requested
  template <class A>
  StateIdTranslation(IMutableHypergraph<A>* o) {
    resetNew(new StateAddMapping<A>(o));
  }
  /// same, but for only source states in subset
  template <class A>
  StateIdTranslation(IMutableHypergraph<A>* o, StateSet const& subset) {
    resetNew(new SubsetStateAddMapping<A>(o, subset));
  }
  void setIdentity() { resetNew(new IdentityIdMapping()); }

 private:
  bool frozenMap;  // TODO: make hypergraphcopy use this instead of separate Subset methods
 public:
  void freeze() {
    frozenMap = true;
    partialMap = true;
  }
  bool frozen() const { return frozenMap; }
  StateId existingState(StateId s) const {
    return Util::getOrElse(cache, s, kNoState);
    //    if (s==kNoState) return s; // handled above already
  }

  /// be careful: make sure 'to' is a value obtained by stateFor or an existing state
  void add(StateId from, StateId to) { cache.insert(std::pair<StateId, StateId>(from, to)); }
  void setSameAs(StateId a, StateId b) { add(a, stateFor(b)); }

  /// this could be used to predict stateids for a previously empty hypergraph. usually the other version is
  /// used
  StateId stateForImpl(StateId s) {
    if (s == kNoState) return s;
    if (frozenMap) return existingState(s);
    StateId* r;
    if (Util::update(cache, s, r)) { return (*r = map->remap(s)); } else
      return *r;
  }

  inline StateId stateFor(StateId s) {
    if (s == kNoState) return s;
    StateId r = stateForImpl(s);
    // SDL_TRACE(Hypergraph.StateIdTranslation, "stateFor(" << s<<")=" << r);
    return r;
  }

  StateId addState(StateId s, LabelPair const& io) {
    StateId r = addStateImpl(s, io);
    // SDL_TRACE(Hypergraph.StateIdTranslation, "addState(" << s<<"," << io << ")=" << r);
    return r;
  }

  // for axioms (lexical states w/ no arcs)
  StateId addStateImpl(StateId s, LabelPair const& io) {
    if (frozenMap) return existingState(s);
    StateId* r;
    if (Util::update(cache, s, r)) { return (*r = map->remapForLabelPair(s, io)); } else
      return *r;
  }

  // modify arc in place, returning true if it's kept. you should pre-map the axioms first if you want
  // kCanonicalLex. but maybe some of the axioms aren't useful! hm. maybe detect axiom in here? would need the
  // hg or pre-computed state->axiom? bitvector.
  /// NOTE: this may add states that turn out to be useless, unless you prpopulate then freeze the state
  /// mapping
  template <class A>
  bool relabelArc(A& a) {
    if (identityMap && !stateAddingMap) return true;
    StateIdContainer& ts = a.tails();
    StateId head = stateFor(a.head());
    if (partialMap) {
      if (head == kNoState) return false;
      a.setHead(head);
      for (unsigned i = 0; i < ts.size(); ++i) {
        StateId s = stateFor(ts[i]);
        if (s == kNoState) return false;
        ts[i] = s;
      }
      return true;
    } else {
      a.setHead(head);
      for (unsigned i = 0; i < ts.size(); ++i) ts[i] = stateFor(ts[i]);
      return true;
    }
  }

  template <class A>
  A* copyArc(A const& a) {
    if (identityMap) return new A(a);
    assert(stateAddingMap);
    if (partialMap) {
      StateIdContainer const& ts = a.tails();
      StateId head = stateFor(a.head());
      if (head == kNoState) return 0;
      TailId N = (TailId)ts.size();
      StateIdContainer tails(N);
      for (TailId i = 0; i < N; ++i) {
        StateId ti = stateFor(ts[i]);
        if (ti == kNoState) return 0;
        tails[i] = ti;
      }
      return new A(head, tails, a.weight());
    } else { return copyArcWithMappedStates(*this, a); }
  }

  template <class A>
  void transferLabelsPartial(IHypergraph<A> const& ihg, IMutableHypergraph<A>& ohg) {
    forall (StateIdMap::value_type const& io, cache)
      ohg.setLabelPair(io.second, ihg.labelPair(io.first));
  }

  template <class A>
  void transferLabels(IHypergraph<A> const& ihg, IMutableHypergraph<A>& ohg) {
    if (frozenMap) { transferLabelsPartial(ihg, ohg); } else {
      StateId s = 0, e = ihg.size();
      StateId onStates = ohg.size();  // lower bound on true number of states
      for (; s != e; ++s)
        if (ihg.hasLabel(s)) {
          StateId so = stateFor(s);
          if (!stateAddingMap && so >= onStates) {
            ohg.addStateId(so);
            onStates = ohg.size();
          }
          if (so != kNoState) ohg.setLabelPair(so, ihg.labelPair(s));
        }
    }
  }

  template <class Out>
  void print(Out& o) const {
    o << map->name();
    if (identityMap) { o << "=Identity"; } else {
      if (frozenMap) o << "(frozen)";
      o << "= {" << Util::print(cache, Util::multiLine()) << "}";
    }
  }

  template <class C, class T>
  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, StateIdTranslation const& self) {
    self.print(o);
    return o;
  }
  template <class C, class T>
  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, StateIdTranslation const* selfp) {
    o << "StateIdTranslation@0x" << (void*)selfp << ": ";
    if (selfp) selfp->print(o);
    return o;
  }
};


}}

#endif
