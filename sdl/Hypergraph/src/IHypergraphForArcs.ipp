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

    (no include guard) - to include at class level for both HypergraphBase
    and IHypergraph<Arc> (so we can have statically known Arc* arg for
    IHypergraph<Arc> vs ArcBase* for HypergraphBase

    //TODO: some of these should be removed or made global templates
*/

/* call v(a) for all Arc *a - possibly more than once if you have nodes
     appearing more than once as a tail. */

  template <class V>
  void forArcsOut(V const& v, bool keepRepeats = false) const {
    if (storesFirstTailOutArcs() || keepRepeats)
      forArcsOutEveryTail(v);
    else
      forArcsOutOnce(v);
  }

  // f(Arc *): Val ; note f by ref
  // note: f(Arc *) because updateBy uses map type
  template <class Val, class F>
  void arcsOnce(F& f, unordered_map<Arc*, Val>& map) const {
    if (storesInArcs())
      arcsInOnce(f, map);
    else
      arcsOutOnce(f, map);
  }

  // safe even if you update arcs as you go e.g. deleting them
  template <class V>
  void forArcsOutOnceSafe(V const& v) const {
    Util::Once once;
    forArcsOutEveryTail(Util::makeVisitOnceRef(v, &once));
    // TODO: JG: more efficient: small open hash inside per-state loop (only
    // iterating over tails), or just quadratic scan (#tails is always small)
  }

  template <class V>
  void forArcsFsm(V const& v) const {
    if (!getVocabulary()) {
      forArcsOutFirstTail(v);
      return;
    }
#define FORSTATE(state)                                \
  do {                                                 \
    if (!hasLexicalLabel(state)) forArcsOut(state, v); \
  } while (0)
    StateId st = start();
    StateId state = 0, e = this->size();
    if (st == Hypergraph::kNoState)
      for (; state < e; ++state) FORSTATE(state);
    else {
      assert(!hasLexicalLabel(st));
      FORSTATE(st);
      for (; state < st; ++state) FORSTATE(state);
      for (++state; state < e; ++state) FORSTATE(state);
#undef FORSTATE
    }
  }

  template <class V>
  void forArcsOutOnce(V const& v) const {
    if (storesFirstTailOutArcs())
      forArcsOutEveryTail(v);
    else if (isFsm())
      forArcsFsm(v);
    else
      forArcsOutOnceSafe(v);
  }

  template <class V>
  void forArcsOut(StateId state, V const& v, bool firstTailOnly) const {
    if (firstTailOnly)
      forArcsOutFirstTail(state, v);
    else
      forArcsOut(state, v);
  }

  /**
     v(tail, &a) -> bool. for arcs a with tail 'tail'.

     \return true iff any v(tail, &a) returned true
  */
  template <class V>
  bool forArcsOutSearch(StateId tail, V const& v) const {
    if (tail == kNoState) return false;
    if (this->properties() & kStoreOutArcs)
      return forArcsOutAny(tail, v);
    else
      // TODO: test
      for (StateId state = 0, e = this->size(); state < e; ++state)
        for (ArcId a = 0, f = this->numInArcs(state); a != f; ++a) {
          Arc* pa = this->inArc(state, a);
          if (Util::contains(pa->tails_, tail))
            if (v(tail, pa)) return true;
        }
    return false;
  }

  // v(a) -> bool. continue search only if false
  template <class V>
  bool forArcsInSearch(StateId head, V const& v, bool firstTailOnly = true) const {
    if (head == kNoState) return false;
    Properties p = this->properties();
    if (p & kStoreInArcs)
      // TODO: test
      return forArcsInAny(head, v);
    else {
      bool storefirst = p & kStoreFirstTailOutArcs;
      for (StateId state = 0, e = this->size(); state < e; ++state)
        for (ArcId a = 0, f = this->numOutArcs(state); a != f; ++a) {
          Arc* pa = this->outArc(state, a);
          if (pa->head_ == head && (storefirst || !firstTailOnly || pa->tails_[0] == state))
            // TODO: test
            if (v(pa)) return true;
        }
    }
    return false;
  }

  // prefer in if have, else out
  template <class V>
  void forArcs(StateId state, V const& v) const {
    if (storesInArcs())
      forArcsIn(state, v);
    else
      forArcsOut(state, v);
  }

  /**
     v(state, &a) -> bool. for arcs a with tail 'state'.

     \return true iff any v(tail, &a) returned true
  */
  template <class V>
  bool forArcsOutAny(StateId state, V const& v) const {
    for (ArcId a = 0, f = this->numOutArcs(state); a < f; ++a)
      if (v(state, this->outArc(state, a))) return true;
    return false;
  }

  /**
     v(&a) -> bool. for arcs a with head 'state'.

     \return true iff any v(&a) returned true
  */
  template <class V>
  bool forArcsInAny(StateId state, V const& v) const {
    for (ArcId a = 0, f = this->numInArcs(state); a < f; ++a)
      if (v(this->inArc(state, a))) return true;
    return false;
  }

  template <class V>
  void forArcsOut(StateId state, V const& v) const {
    for (ArcId a = 0, f = this->numOutArcs(state); a < f; ++a) v(this->outArc(state, a));
  }

  template <class V>
  void forArcsOutEveryTail(V const& v) const {
    for (StateId state = 0, e = this->size(); state < e; ++state) forArcsOut(state, v);
  }

  template <class V>
  void forArcsIn(StateId state, V const& v) const {
    for (ArcId a = 0, f = this->numInArcs(state); a != f; ++a) v(this->inArc(state, a));
  }

  template <class V>
  void forArcsIn(V const& v) const {
    assert(storesInArcs());
    for (StateId state = 0, e = this->size(); state != e; ++state) forArcsIn(state, v);
  }

  // forArcs visits each Arc * once. for sure. also doesn't mind if there are no
  // arc indices (because this is used by destructor and we may wish to allow
  // arcless "hypergraphs" w/ states+labels only?

  // ok to modify arcs without affecting visit-once property
  template <class V>
  void forArcsSafe(V const& v) const {
    // no repeats
    if (this->storesInArcs())
      forArcsIn(v);
    else if (this->storesOutArcs())
      forArcsOutOnceSafe(v);
  }

  template <class V>
  void forArcs(V const& v) const {
    // no repeats
    if (this->storesInArcs())
      forArcsIn(v);
    else if (this->storesOutArcs())
      forArcsOutOnce(v);
    else
      this->throwStoresNoArcs();
  }

  template <class V>
  void forArcsPreferRepeats(V const& v) const {
    if (this->storesOutArcs())
      forArcsOutEveryTail(v);
    else if (this->storesInArcs())
      forArcsIn(v);
    else
      this->throwStoresNoArcs();
  }

  template <class V>
  void forArcsAllowRepeats(V const& v) const {
    if (this->storesInArcs())
      forArcsIn(v);
    else if (this->storesOutArcs())
      forArcsOutEveryTail(v);
    else
      this->throwStoresNoArcs();
  }

  // NO LONGER start state first, if there is one
  // - only WriteOpenFstFormat wanted that, and we replaced it w/ explicit per-state calls.
  template <class V>
  void forArcsOutFirstTail(V const& v) const {
    assert(this->storesOutArcs());
    StateId state = 0, e = this->size();
    for (; state < e; ++state) forArcsOutFirstTail(state, v);
  }

  // visits out arc only when tail node is same as first tail (can still lead to repeats, though not in Fsm)
  template <class V>
  void forArcsOutFirstTail(StateId state, V const& v) const {
    for (ArcId a = 0, f = this->numOutArcs(state); a < f; ++a) {
      Arc* pa = this->outArc(state, a);
      StateIdContainer const& tails = pa->tails_;
      assert(!tails.empty());
      StateId t0 = *tails.begin();
      if (t0 == state) v(pa);
    }
  }

  template <class Val, class F>
  void arcsInOnce(F& f, unordered_map<Arc*, Val>& map) const {
    assert(this->storesInArcs());
    for (StateId state = 0, e = this->size(); state != e; ++state)
      for (ArcId a = 0, n = this->numInArcs(state); a != n; ++a) Util::getLazy(f, map, this->inArc(state, a));
  }

  template <class Val, class F>
  void arcsOutOnce(F& f, unordered_map<Arc*, Val>& map) const {
    assert(this->storesOutArcs());
    for (StateId state = 0, e = this->size(); state != e; ++state)
      for (ArcId a = 0, n = this->numOutArcs(state); a != n; ++a) Util::getLazy(f, map, this->outArc(state, a));
  }

  // v(inputLabel(a->tails_[1]), a) - input label only (fsa)
  template <class V>
  void forArcsFsa(StateId state, V const& v) const {
    for (ArcId a = 0, f = this->numOutArcs(state); a < f; ++a) {
      Arc* pa = this->outArc(state, a);
      v(this->inputLabel(pa->fsmSymbolState()), *pa);
    }
  }
