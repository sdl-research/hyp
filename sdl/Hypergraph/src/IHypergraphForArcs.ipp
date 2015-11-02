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
#define FORSTATE(s)                                \
  do {                                                 \
    if (!hasLexicalLabel(s)) forArcsOut(s, v); \
  } while (0)
    StateId st = start();
    StateId s = 0, e = this->size();
    if (st == Hypergraph::kNoState)
      for (; s < e; ++s) FORSTATE(s);
    else {
      assert(!hasLexicalLabel(st));
      FORSTATE(st);
      for (; s < st; ++s) FORSTATE(s);
      for (++s; s < e; ++s) FORSTATE(s);
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
  void forArcsOut(StateId s, V const& v, bool firstTailOnly) const {
    if (firstTailOnly)
      forArcsOutFirstTail(s, v);
    else
      forArcsOut(s, v);
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
      for (StateId s = 0, e = this->size(); s < e; ++s)
        for (ArcId a = 0, f = this->numInArcs(s); a != f; ++a) {
          Arc* pa = this->inArc(s, a);
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
      for (StateId s = 0, e = this->size(); s < e; ++s)
        for (ArcId a = 0, f = this->numOutArcs(s); a != f; ++a) {
          Arc* pa = this->outArc(s, a);
          if (pa->head_ == head && (storefirst || !firstTailOnly || pa->tails_[0] == s))
            // TODO: test
            if (v(pa)) return true;
        }
    }
    return false;
  }

  // prefer in if have, else out
  template <class V>
  void forArcs(StateId s, V const& v) const {
    if (storesInArcs())
      forArcsIn(s, v);
    else
      forArcsOut(s, v);
  }

  /**
     v(s, &a) -> bool. for arcs a with tail 's'.

     \return true iff any v(tail, &a) returned true
  */
  template <class V>
  bool forArcsOutAny(StateId s, V const& v) const {
    for (ArcId a = 0, f = this->numOutArcs(s); a < f; ++a)
      if (v(s, this->outArc(s, a))) return true;
    return false;
  }

  /**
     v(&a) -> bool. for arcs a with head 's'.

     \return true iff any v(&a) returned true
  */
  template <class V>
  bool forArcsInAny(StateId s, V const& v) const {
    for (ArcId a = 0, f = this->numInArcs(s); a < f; ++a)
      if (v(this->inArc(s, a))) return true;
    return false;
  }

  template <class V>
  void forArcsOut(StateId s, V const& v) const {
    for (ArcId a = 0, f = this->numOutArcs(s); a < f; ++a) v(this->outArc(s, a));
  }

  template <class V>
  void forArcsOutEveryTail(V const& v) const {
    for (StateId s = 0, e = this->size(); s < e; ++s) forArcsOut(s, v);
  }

  template <class V>
  void forArcsIn(StateId s, V const& v) const {
    for (ArcId a = 0, f = this->numInArcs(s); a != f; ++a) v(this->inArc(s, a));
  }

  template <class V>
  void forArcsIn(V const& v) const {
    assert(storesInArcs());
    for (StateId s = 0, e = this->size(); s != e; ++s) forArcsIn(s, v);
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

  template <class V>
  void forArcsOutFirstTail(V const& v) const {
    assert(this->storesOutArcs());
    StateId s = 0, e = this->size();
    if (this->storesFirstTailOutArcs()) {
      for (; s < e; ++s) forArcsOut(s, v);
    } else
      for (; s < e; ++s) forArcsOutFirstTail(s, v);
  }

  // visits out arc only when tail node is same as first tail (can still lead to repeats, though not in Fsm)
  template <class V>
  void forArcsOutFirstTail(StateId s, V const& v) const {
    for (ArcId a = 0, f = this->numOutArcs(s); a < f; ++a) {
      Arc* pa = this->outArc(s, a);
      StateIdContainer const& tails = pa->tails_;
      assert(!tails.empty());
      StateId t0 = *tails.begin();
      if (t0 == s) v(pa);
    }
  }

  template <class Val, class F>
  void arcsInOnce(F& f, unordered_map<Arc*, Val>& map) const {
    assert(this->storesInArcs());
    for (StateId s = 0, e = this->size(); s != e; ++s)
      for (ArcId a = 0, n = this->numInArcs(s); a != n; ++a) Util::getLazy(f, map, this->inArc(s, a));
  }

  template <class Val, class F>
  void arcsOutOnce(F& f, unordered_map<Arc*, Val>& map) const {
    assert(this->storesOutArcs());
    for (StateId s = 0, e = this->size(); s != e; ++s)
      for (ArcId a = 0, n = this->numOutArcs(s); a != n; ++a) Util::getLazy(f, map, this->outArc(s, a));
  }

  // v(inputLabel(a->tails_[1]), a) - input label only (fsa)
  template <class V>
  void forArcsFsa(StateId s, V const& v) const {
    for (ArcId a = 0, f = this->numOutArcs(s); a < f; ++a) {
      Arc* pa = this->outArc(s, a);
      v(this->inputLabel(pa->fsmSymbolState()), *pa);
    }
  }
