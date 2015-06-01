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

    usage:
    determinize(i, &o);

    where i is hypergraph that happens to be an unweighted fsa, with epsilons and phi only,
    and o is mutable hypergraph

    TODO: support (acyclic, at least) weighted determinization

    terminology:

    DFA < UFA < FNA <= PNA < NFA

    UFA: unambiguous
    FNA: finitely nondet.
    PNA: polynomially ambig.

    (from
   http://www.google.com/url?sa=t&rct=j&q=ufa%20unambiguous%20automata&source=web&cd=10&sqi=2&ved=0CEwQFjAJ&url=http%3A%2F%2Fciteseerx.ist.psu.edu%2Fviewdoc%2Fdownload%3Fdoi%3D10.1.1.7.667%26rep%3Drep1%26type%3Dps&ei=r1SgTojiJ4PMhAevusStBQ&usg=AFQjCNH4B_nsqJFlng0y0zh0pmBQMkO2pQ
   - an NFA that requires 2^n-1 states as a PNA - note: their NFA allows multiple starting states)

    ambiguity of string: # of accepting runs ; max ambig of str of len n is nondecreasing with n
    UFA: ambig=0 or 1
    FNA or PNA: ambig(n) is O(k) or O(x^k) resp
    SEA: strictly exponentially ambig: ambig(n)=Theta(k^n) (i.e. ENA and not PNA)
    SEA iff exists useful state q and string w so that q->w->q ambiguously (>1 run)

    (from http://lrb.cs.uni-dortmund.de/~martens/data/icalp08.pdf )
    UFA: ptime subset, equivalence, but:

    NP-complete: min UFA L= UFA, min UFA L= DFA
    PSPACE-complete: min NFA L= DFA

    DFA L= NFA is expsize output but low polynomial (in size of output) construction

    DFA->X and X->X min is NP-complete for many classes between X DFA and NFA

    1. choose between >1 init states, then deterministic

    or 2. constant # of runs per string

    or 3. delta-NFA: unambiguous, <=2 (accepting?) runs per string. <=1 (q, a) with |q(a)|=2 (two outgoing
   a-transitions from state q)

    or 3b. thereforeeven DFA-with-2-start-states

    (are all NP-complete to minimize for any FA class including them)

    reason for 3/3b (delta-NFA): many possible *rejecting* computations - if at most one rejection, then the
   class is exactly DFA

*/

/* meanings:

   epsilon and phi don't advance the input pointer when taking the arc (rho and sigma do).

   epsilon and sigma match any symbol;
   phi and rho are "else" - they match any symbol not explicitly named on another outgoing arc
*/
#ifndef HYPERGRAPH_DETERMINIZE_HPP
#define HYPERGRAPH_DETERMINIZE_HPP
#pragma once

#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/SharedPtr.hpp>
#include <sdl/Util/DefaultPrintRange.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Util/ShrinkVector.hpp>
#include <sdl/Util/Latch.hpp>
#include <sdl/Util/Compare.hpp>
#include <sdl/Util/Sorted.hpp>
#include <sdl/Pool/object_pool.hpp>
#include <sdl/Hypergraph/Graph.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/FnReference.hpp>
#include <sdl/Util/Unordered.hpp>

/// 1 => use sorted vector rather than unordered map. but then needs work for >1 thread.
#define SDL_DETERMINIZE_SORT 0

/**
   // 1 means we use an IntSetEqual that knows which arg is the key and has seen bits already set.
   // 1 => group outgoing arcs by symbol, so subsets can be built using single scratch vector
   // 0 => visit arcs in random order, using target bin. subsets then are std::set..

   //TODO: investigate segfault w/ 1
 */
#define SDL_KNOWN_KEY_COMPARE 0

/// extra debug checks (quite slow)
#define SDL_DETERMINIZE_ASSERT_SLOW 0

#if SDL_DETERMINIZE_SORT && SDL_DETERMINIZE_ASSERT_SLOW
#define SDL_DETERMINIZE_ASSERT(x) assert(x)
#else
#define SDL_DETERMINIZE_ASSERT(x)
#endif

namespace sdl {
namespace Hypergraph {

typedef Util::DupCount<sdl::unordered_set<Sym> > DupSymbols;

template <class Arc>
struct IsDetState {  // unlike DupSymbols, allow epsilon to final only!
  IHypergraph<Arc> const& hg;
  StateId final;
  typedef unordered_set<Sym> S;
  S s;
  bool r;
  bool anyeps;
  explicit IsDetState(IHypergraph<Arc> const& hg) : hg(hg), final(hg.final()) {
    r = true;
    anyeps = false;
  }
  template <class A>
  void operator()(A* a) {
    Sym const in = hg.firstLexicalInput(a);
    if (in == EPSILON::ID) anyeps = true;
    if (!(Util::latch(s, in) && (in || a->head() == final))) r = false;
  }
};

template <class A>
bool isDeterminized(StateId s, IHypergraph<A> const& i, bool allow_epsilon = false) {
  if (i.prunedEmpty()) return true;
  IsDetState<A> det(i);
  i.forArcsOutFirstTail(s, Util::visitorReference(det));
  return det.r && (allow_epsilon || !det.anyeps);
}

template <class A>
bool isDeterminized(IHypergraph<A> const& i, bool allow_epsilon = false) {
  if (i.prunedEmpty()) return true;
  StateId ns = i.size();
  for (StateId s = 0; s < ns; ++s)
    if (i.isFsmState(s) && !isDeterminized(s, i, allow_epsilon)) {
      return false;
    }
  return true;
}

template <class A>
bool containsEmptyStringDet(IHypergraph<A> const& i) {
  assert(isDeterminized(i));
  StateId st = i.start(), f = i.final();
  if (st == f) return true;
  for (ArcId a = 0, e = i.numOutArcs(st); a != e; ++a) {
    A* pa = i.outArc(st, a);
    if (pa->head() == f && i.inputLabel(pa->fsmSymbolState()) == EPSILON::ID) return true;
  }
  return false;
}

typedef boost::uint32_t DeterminizeFlags;
const DeterminizeFlags DETERMINIZE_INPUT = 1;
const DeterminizeFlags DETERMINIZE_OUTPUT = 2;
const DeterminizeFlags DETERMINIZE_FST = 4;  // TODO
const DeterminizeFlags DETERMINIZE_EPSILON_NORMAL = 8;
const DeterminizeFlags DETERMINIZE_EPS_NORMAL = DETERMINIZE_EPSILON_NORMAL;
const DeterminizeFlags DETERMINIZE_RHO_NORMAL = 0x10;
const DeterminizeFlags DETERMINIZE_PHI_NORMAL = 0x20;
const DeterminizeFlags DETERMINIZE_SIGMA_NORMAL = 0x40;

#define DET_SPECIAL_SYMBOL(t, x) (!(t & DETERMINIZE_##x##_NORMAL))
#define IS_DET_SPECIAL_SYM(s, t, x) (s == x::ID && DET_SPECIAL_SYMBOL(t, x))

#if SDL_DETERMINIZE_SORT
typedef std::vector<StateId> QSet;  // prevent duplicates via seenq
#else
typedef std::set<StateId> QSet;  // set instead of unordered_set for better hashing/equality
#endif
typedef QSet const* Qp;

DEFINE_PRINTRANGE(QSet)

#if SDL_DETERMINIZE_SORT
typedef unordered_map<Qp, StateId, IntSetHash, IntSetEqual<StateSet> > Explored;
// FIXME: threadsafe IntSetEqual
#else
typedef unordered_map<Qp, StateId, Util::PointedHashCont, Util::PointedEqual> Explored;
#endif
inline std::ostream& operator<<(std::ostream& o, Explored::value_type const& p) {
  o << '(' << *p.first << ',' << p.second << ')';
  return o;
}

struct ToDo {
  Qp qs;  // states
  StateId q;  // result-state
  friend inline std::ostream& operator<<(std::ostream& o, ToDo const& t) {
    return o << "ToDo:" << t.q << "=" << *t.qs;
  }
};
typedef std::vector<ToDo> Agenda;
DEFINE_PRINTRANGE(Agenda)

/**
   these objects must not be shared across threads.

   TODO: (no, maybe, yes) memoization of final-state reachability? - for
   lattices, this means pruning away material early always

   TODO: test speed of epsilon-graph-DFS vs. precomputed
   epsilon-closure-list. list probably faster, but DFS detects
   already-in-subset so saves repeated visiting of epsilon-suffixes - one
   doesn't always beat other
*/
template <class A>
struct DeterminizeFsa {
  IHypergraph<A> const& i;  // input
  IMutableHypergraph<A>* o;  // output
  DeterminizeFlags flags;
  Graph eps;
  StateSet expanded;
#if SDL_DETERMINIZE_SORT
  StateSet seenq;  // SINGLE THREADED
  template <class I>
  void addqs(QSet& qs, I i, I const& end, bool& anyadded) {
    for (; i != end; ++i)
      if (Util::latch(seenq, *i)) {
        Util::add(qs, *i);
        anyadded = true;
      }
  }
  void clearseen(QSet const& qs) {
    for (QSet::const_iterator i = qs.begin(), e = qs.end(); i != e; ++i) reset(seenq, *i);
  }
  bool checkempty() const { return seenq.count() == 0; }
  bool checkseen(QSet const& qs) const {
    for (QSet::const_iterator i = qs.begin(), e = qs.end(); i != e; ++i)
      if (!test(seenq, *i)) return false;
    return true;
  }
  void setseen(QSet const& qs) const {
    for (QSet::const_iterator i = qs.begin(), e = qs.end(); i != e; ++i) setValue(seenq, *i);
  }
  bool checkseeniff(QSet const& qs) {
    for (QSet::const_iterator i = qs.begin(), e = qs.end(); i != e; ++i)
      if (test(seenq, *i))
        reset(seenq, *i);
      else {
        for (QSet::const_iterator j = qs.begin(); j != i; ++j)  // undo resets
          setValue(seenq, *j);
        return false;
      }
    bool ret = true;
    if (seenq.count()) ret = false;
    for (QSet::const_iterator i = qs.begin(), e = qs.end(); i != e; ++i)  // undo resets
      setValue(seenq, *i);
    return ret;
  }
  QSet& uniq(QSet& qs) {  // doesn't clear qseen! result to be passed to subsetId
    SDL_DETERMINIZE_ASSERT(checkempty());
    removeShrink(qs, removeDup(seenq));
    return qs;
  }
#endif
  template <class C>
  void addqs(QSet& qs, C const& c, bool& anyadded) {
#if SDL_DETERMINIZE_SORT
    addqs(qs, c.begin(), c.end(), anyadded);
#else
    Util::latchAny(anyadded, qs, c);
#endif
  }
  Pool::object_pool<QSet> qsp;
  void destroy(QSet* p) {
    // qsp.destroy(p); //FIXME: object_pool has O(N) destroy 1 object
  }

  StateId qfinali;
  typedef std::pair<Sym, StateId> Arc;
  typedef std::vector<Arc> Arcs;
  typedef unordered_set<Sym> Letters;
  typedef Syms LetterList;
  typedef std::vector<StateId> States;
  struct Outi {
    Letters normals;
    Arcs arcs;  // no specials.
    States qrhos;  // "else" dest states
  };
  struct has_rhos {
    bool operator()(Outi const& p) { return !p.qrhos.empty(); }
  };

  Explored explored;
  typedef std::vector<Outi> Rhos;
  // for fixing the meaning of "else" (rho) transitions across i states. also
  // stores (more concise than in i) normal transitions now.
  Rhos rhos;
  Agenda agenda;

  std::vector<StateId> qfinals;
  bool specialEps;
  DeterminizeFsa(IHypergraph<A> const& i, IMutableHypergraph<A>* o, DeterminizeFlags flags)
      : i(i)
      , o(o)
      , flags(flags)
      , expanded(2 * i.size())
#if SDL_DETERMINIZE_SORT
      , seenq(i.size())
      , explored(2 * i.size(), IntSetHash(), IntSetEqual<StateSet>(&seenq))
#else
      , explored(2 * i.size())
#endif
  {
    assert(i.isFsm());
    qfinali = i.final();
    specialEps = DET_SPECIAL_SYMBOL(flags, EPSILON) && i.anyInputLabel(isEps());
    if (specialEps) {
      eps.addFsmArcs(i, isEps());
      eps.transitiveClose();
    }
    initRhos();
    QSet* s = qsp.construct();
    Util::add(*s, i.start());
    o->setStart(subsetId(*s));
    finish_agenda();
    StateId nf = (StateId)qfinals.size();
    if (nf == 0) {
      o->setEmpty();
    } else {
      StateId f;
      if (nf == 1)
        f = qfinals[0];
      else {
        f = o->addState();
        forall (StateId q, qfinals) { o->addArcFsa(q, f); }
      }
      o->setFinal(f);
    }
  }
  bool isFinal(QSet const& qs) {
    return Util::contains(qs, qfinali);  // linear time
  }

  // pre: if SDL_DETERMINIZE_SORT && SDL_KNOWN_KEY_COMPARE, seenq==qs; else seenq==0
  // in all cases, post: seenq==0
  /**
     \return result state for state subset.
  */
  StateId subsetId(QSet& qs) {  // memoized before and after e-closure!
#if SDL_DETERMINIZE_SORT && SDL_KNOWN_KEY_COMPARE
#define FORCE_CLEAR(qs) \
  clearseen(qs);        \
  SDL_DETERMINIZE_ASSERT(checkempty());
#define FORCE_SET(qs)                   \
  SDL_DETERMINIZE_ASSERT(checkempty()); \
  setseen(qs);
#else
#define FORCE_CLEAR(qs)
#define FORCE_SET(qs)
#endif
// because of uniq leaving qs' bits set:
#if SDL_KNOWN_KEY_COMPARE
    SDL_DETERMINIZE_ASSERT(checkseeniff(qs));
#else
    FORCE_CLEAR(qs);
#endif
    StateId* id;
    if (!Util::update(explored, &qs, id)) {
      FORCE_CLEAR(qs);
      destroy(&qs);
      return *id;
    }
    QSet *pcqs = &qs, *newpcqs = 0;
    bool anyadded = false;
    if (specialEps) {
      newpcqs = qsp.construct(qs);
      FORCE_SET(qs);
      forall (StateId s, qs) {  // avoid iterating over in-place set result because of possible invalidation
        // if using BitSet, use |= op
        addqs(*newpcqs, eps.outs[s], anyadded);
        // could also memoize every intermediate result, but why bother? bad space/time tradeoff maybe, if we
        // do
      }
      if (anyadded) {
        pcqs = newpcqs;
      } else {
        destroy(newpcqs);
      }
    }
    // POST: anyadded iff need to destroy pcqs if it's not newly added to explored
    SDL_DETERMINIZE_ASSERT(checkseeniff(*pcqs));  // POST if SDL_DETERMINIZE_SORT: seen iff pcqs
    StateId* cid = 0;  // init to quiet false warning
    StateId subsetState;
    if (!anyadded || Util::update(explored, pcqs, cid)) {
      subsetState = o->addState();
      if (isFinal(qs)) {
        Util::add(qfinals, subsetState);
      }
      ToDo t;
      t.qs = pcqs;
      t.q = subsetState;
      Util::add(agenda, t);  // don't care fifo vs lifo vs ???
      *id = subsetState;
      if (anyadded) *cid = subsetState;
    } else {
      // but closed pcqs was already existing
      assert(anyadded);  // because !(!anyadded || X) implies anyadded
      destroy(newpcqs);
      subsetState = *cid;
    }

    FORCE_CLEAR(*pcqs);
    return subsetState;
  }

#if SDL_DETERMINIZE_SORT
  typedef unordered_map<Sym, QSet> Delta;  // single vector which grows? how contig. are symbolids?
  static inline QSet& delta(Delta& d, Sym s) { return d[s]; }
#define DEREF_DELTA
#else
  typedef QSet* Qmutp;
  typedef unordered_map<Sym, Qmutp> Delta;  // single vector which grows? how contig. are symbolids?
  QSet& delta(Delta& d, Sym s) {
    Qmutp* pp;
    if (Util::update(d, s, pp)) return *(*pp = qsp.construct());
    return **pp;
  }
#define DEREF_DELTA *
#endif
  // note: not uniqued at first for SDL_DETERMINIZE_SORT. dqs is destroyed if it already existed in explored
  void addArc(StateId q, QSet& dqs, Sym s) {
    StateId i = subsetId(dqs);
    o->addArcFsa(q, i, s);
  }
  // happens once per qs, because new things go on agenda only once (atomic w/ marking it 'explored')
  void genOut(QSet const& qs, StateId q) {
    if (!Util::latchGrow(expanded, q)) return;
    Delta d;
    std::vector<StateId> qrho;
    QSet& qallrho = *qsp.construct();
    bool addrhodummy;
    forall (StateId s, qs) {
      Outi const& p = rhos[s];
      if (!p.qrhos.empty()) {
        qrho.push_back(s);  // process later once full alphabet for qs is known
        addqs(qallrho, p.qrhos, addrhodummy);
      }
      forall (Arc const& a, p.arcs) {
        Util::add(delta(d, a.first), a.second);  // SDL_DETERMINIZE_SORT: will uniq later
      }
    }
    if (qrho.empty())
      destroy(&qallrho);
    else
      addArc(q, qallrho, RHO::ID);  // handles all symbols not mentioned in any outgoing arc of qs.
    forall (Delta::value_type& v, d) {
      Sym i = v.first;
      QSet& dqs = DEREF_DELTA v.second;
      forall (StateId q, qrho) {  // now handle symbols not mentioned in state q but mentioned elsehwere in qs
        Outi const& p = rhos[q];
        if (!Util::contains(p.normals, i))  // because the normal letters from q-> don't include i, rhos from
          // q->x should be added to d[i]=dqs
          Util::addAll(dqs, p.qrhos);
      }
#if SDL_DETERMINIZE_SORT
// will uniq subsets, then add later
#else
      addArc(q, dqs, i);
#endif
    }
#if SDL_DETERMINIZE_SORT
    forall (Delta::value_type& v, d) { addArc(q, uniq(v.second), v.first); }
#endif
  }

  void finish_agenda() {
    while (!agenda.empty()) {
      ToDo t = Util::top(agenda);
      Util::pop(agenda);
      genOut(*t.qs, t.q);
    }
  }

  struct addLetter {
    DeterminizeFlags t;
    Outi& l;
    addLetter(DeterminizeFlags t, Outi& l) : t(t), l(l) {}
    void operator()(Sym s, A const& a) const {
      StateId head = a.head();
      if (IS_DET_SPECIAL_SYM(s, t, RHO))
        Util::add(l.qrhos, head);
      else if (IS_DET_SPECIAL_SYM(s, t, EPSILON)) {
      } else if (IS_DET_SPECIAL_SYM(s, t, PHI) || IS_DET_SPECIAL_SYM(s, t, SIGMA)) {
        SDL_THROW_LOG(Hypergraph, InvalidInputException,
                      "Determinize: input Hypergraph (Fsm) must have only EPSILON and RHO special symbols "
                      "(RHO/SIGMA are future work)");
      } else {
        Util::add(l.normals, s);
        Util::add(l.arcs, Arc(s, head));
      }
    }
  };
  void initRhos() {
    StateId N = i.size();
    Util::reinit(rhos, N);
    for (StateId s = 0; s < N; ++s) {
      i.forArcsFsa(s, addLetter(flags, rhos[s]));
      // std::sort(rhos.arcs.begin(), rhos.arcs.end()); //  sorted. note: we sort so union-per-letter of
      // dest-states, which uses less memory than map of many-partial-unions-per-letter
    }
  }
};

struct NotDeterminized {
  DeterminizeFlags flags;
  bool operator()(Sym i) const {
    return IS_DET_SPECIAL_SYM(i, flags, PHI) || IS_DET_SPECIAL_SYM(i, flags, SIGMA);
    // epsilon (I handle subset-e-closure automatically), and rho (an "else, consuming") are both ok!
  }
};

template <class Arc>
void determinize_always(IHypergraph<Arc> const& i,  // input
                        IMutableHypergraph<Arc>* o,  // already has desired props
                        DeterminizeFlags flags = DETERMINIZE_INPUT) {
  assert(o->storesArcs());
  typedef MutableHypergraph<Arc> H;
  typedef shared_ptr<H> HP;
  HP fsap;
  IHypergraph<Arc> const* ip = &i;  // copy only if needed
  o->setVocabulary(i.getVocabulary());

  bool dinput = flags & DETERMINIZE_INPUT;
  bool doutput = flags & DETERMINIZE_OUTPUT;
  bool dfst = flags & DETERMINIZE_FST;
  if (dinput && i.hasOutputLabels() || doutput) {
    H* f = new H();
    fsap = HP(f);
    copyHypergraph(i, f);
    if (dinput) f->projectInput();
    if (doutput) f->projectOutput();
    ip = f;
  }
  if (doutput && dinput || dfst)
    SDL_THROW_LOG(Hypergraph, InvalidInputException,
                  "Determinize: choose one of DETERMINIZE_INPUT or DETERMINIZE_OUTPUT (FST not supported "
                  "yet)");

  DeterminizeFsa<Arc> det(*ip, o, flags);
  assert(isDeterminized(*o));
}

template <class Arc>
void assertCanDeterminize(IHypergraph<Arc> const& i, DeterminizeFlags flags) {
  NotDeterminized nd;
  nd.flags = flags;
  if (i.anyInputLabel(nd)) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException,
                  "Can't determinize all special symbols yet - just EPSILON and RHO - try treating them as "
                  "normal letters with flags DETERMINIZE_PHI_NORMAL | DETERMINIZE_SIGMA_NORMAL");
  }
  if (!i.unweighted())
    SDL_THROW_LOG(Hypergraph, InvalidInputException,
                  "Determinize: only supporting unweighted for now (all arcs should have Weight::one())");
  if (!i.isFsm())
    SDL_THROW_LOG(Hypergraph, InvalidInputException,
                  "Determinize: input Hypergraph must be left branching FSA/FST (isFsm())");
}

template <class Arc>
void determinize(IHypergraph<Arc> const& i,  // input
                 IMutableHypergraph<Arc>* o,  // output
                 DeterminizeFlags flags = DETERMINIZE_INPUT, Properties prop_on = kStoreFirstTailOutArcs,
                 Properties prop_off = 0  // kStoreInArcs
                 ) {
  if (empty(i)) {
    o->setEmpty();
  }
  o->forcePropertiesOnOff(prop_on, prop_off);
  assertCanDeterminize(i, flags);
  if (isDeterminized(i, !DET_SPECIAL_SYMBOL(flags, EPSILON)))
    copyHypergraph(i, o);
  else
    determinize_always(i, o, flags);
}

struct Determinize : TransformBase<Transform::Inout, (kFsm | kStoreOutArcs)> {
  DeterminizeFlags flags;
  Properties prop_on, prop_off;
  Determinize(DeterminizeFlags flags = DETERMINIZE_INPUT, Properties prop_on = kStoreOutArcs,
              Properties prop_off = 0  // kStoreInArcs
              )
      : flags(flags), prop_on(prop_on), prop_off(prop_off) {}
  Properties outAddProps() const { return prop_on; }
  Properties outSubProps() const { return prop_off; }
  template <class H>
  bool needs(H const& h) const {
    if (isDeterminized(h, !DET_SPECIAL_SYMBOL(flags, EPSILON))) return false;
    assertCanDeterminize(h, flags);
    return true;
  }
  template <class I>
  void operator()(I& i) const {
    assert(0);
  }  // not finding base method
  template <class I, class Out>
  void inout(I const& i, Out* o) const {
    determinize_always(i, o, flags);
  }
};

template <class A>
IHypergraph<A> const& determinized(IHypergraph<A> const& i, shared_ptr<IHypergraph<A> const>& p,
                                   DeterminizeFlags flags = DETERMINIZE_INPUT,
                                   Properties newOn = kFsm | kStoreOutArcs, Properties newOff = kStoreInArcs) {
  p = ptrNoDelete(i);
  Determinize d(flags, newOn, newOff);
  inplace(p, d);
  return *p;
}

template <class A>
shared_ptr<IHypergraph<A> const> determinized(IHypergraph<A> const& i,
                                              DeterminizeFlags flags = DETERMINIZE_INPUT,
                                              Properties newOn = kFsm | kStoreOutArcs,
                                              Properties newOff = kStoreInArcs) {
  shared_ptr<IHypergraph<A> const> p = ptrNoDelete(i);
  Determinize d(flags, newOn, newOff);
  inplace(p, d);
  return p;
}
}
}


namespace boost {
template <>
struct hash<sdl::Hypergraph::Qp> {
  std::size_t operator()(sdl::Hypergraph::Qp const& x) const { return sdl::Util::hashCont(*x); }
};


}

#endif
