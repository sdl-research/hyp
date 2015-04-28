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

    derivation tree for hypergraph n-best

    TODO: test that we throw CycleException in all cases rather than infinitely
    looping (in case the best derivation is divergent due to negative cost, and
    so has self-loops)
*/

#ifndef HYP__HG_DERIVATION_HPP
#define HYP__HG_DERIVATION_HPP
#pragma once


/// only include nary_tree via this file (separate compilation units: differences ok)
#define CHILD_INDEX_TYPE ::sdl::Hypergraph::TailId

// see also GetString for methods that transform derivations/yields to strings

#include <stack>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/Exception.hpp>
#include <sdl/Hypergraph/StateIdTranslation.hpp>
#include <sdl/Hypergraph/HypergraphCopy.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Util/RefCount.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Util/PointerWithFlag.hpp>

#include <sdl/graehl/shared/nary_tree.hpp>
#include <sdl/graehl/shared/word_spacer.hpp>

#include <boost/range/algorithm/transform.hpp>
#include <boost/get_pointer.hpp>

#include <functional>
#include <sdl/Util/Enum.hpp>
#include <sdl/Hypergraph/DerivationStringOptions.hpp>

namespace sdl {
namespace Hypergraph {


enum { kIsNonBest = 0, kIsBest = 1 };
enum { kRenumberStates = 0, kKeepStateIds = 1 };


template <class T>
struct First {
  typedef T const& result_type;
  template <class Pair>
  result_type operator()(Pair const& pair) const {
    return pair.first;
  }
};

inline void appendStates(StateString& ss, DerivationYield const& dy) {
  std::size_t i = ss.size();
  ss.resize(i + dy.size());
  StateString::iterator o = ss.begin() + i;
  boost::transform(dy, o, First<StateId>());
}

inline StateString statesFrom(DerivationYield const& dy) {
  StateString r;
  appendStates(r, dy);
  return r;
}

/* A derivation is a tree of arc handles w/ #children = #tails. there may be cycles and reuse. cycles would
   lead to memory leaks, though (refcount). TODO: provide a "safe free" or use an external mem pool instead of
   refcount. A "color" field is included for cycle-avoiding and copy-subtree-once depth first search and other
   algorithms

   note: labels aren't included in the derivation; see the original HG for that. state ids are included only
   indirectly by referring to original arcs (especially for axioms, you have to see the stateid of the parent
   hyperedge). i may want to change this (include stateid in axioms or even redundantly in all). axioms don't
   have a nullary edge. they have arc handle a=0. maybe i want to represent states explicitly in the future,
   or else strip from children all the axioms in BestPath?

*/
// TODO: exclude axioms from derivation tree? i.e. derivation child i is the ith non-axiom tail in arc()? can
// make some code (printing, visiting, etc) behave the same for either decision

template <class A>
struct Derivation : public graehl::shared_nary_tree<Derivation<A>, Util::RefCount> {
  typedef graehl::shared_nary_tree<Derivation<A>, Util::RefCount> Tree;
  typedef A Arc;
  typedef typename Arc::Weight Weight;
  typedef typename Tree::child_type child_type;  // intrusive shared pointer to Derivation
  typedef typename Tree::children_type children_type;  // vector
  typedef typename Tree::user_allocator Alloc;
  typedef TailId child_index;
  typedef IHypergraph<Arc> Hg;
  typedef Derivation self_type;
  typedef self_type Deriv;
  typedef child_type DerivP;
  typedef boost::tuple<DerivP, Weight> DerivAndWeight;
  friend inline Deriv& deriv(DerivAndWeight const& dw) { return *boost::get<0>(dw); }
  friend inline DerivP const& derivP(DerivAndWeight const& dw) { return boost::get<0>(dw); }
  friend inline Weight const& weight(DerivAndWeight const& dw) { return boost::get<1>(dw); }

  Derivation(Arc* a) : a(a), color() {}
  Derivation(Arc* a, TailId nchild) : Tree(nchild), a(a), color() {}
  //TODO: maybe we function fine without axiom placeholders here? e.g. if it's a graph and not fsm there may not even be a second tail.
  Derivation(Arc* a, child_type const& child) : Tree(1, child), a(a), color() {}
  Derivation(Arc* a, child_index nchild, child_type const& firstchild) : Tree(nchild, kAxiom), a(a), color() {
    this->children[0] = firstchild;
  }

  DerivP pointer() { return DerivP(this); }

  static child_type kAxiom;  // no need to use this as opposed to construct(), but feel free
  static inline Derivation* malloc() { return (Derivation*)Alloc::malloc(sizeof(Derivation)); }
  static inline child_type construct(Arc* a, child_type const& child) {
    Derivation* r = malloc();
    new (r) Derivation(a, child);
    return child_type(r);
  }

  static inline child_type construct(Arc* a, TailId nchild) {
    Derivation* r = malloc();
    new (r) Derivation(a, nchild);
    return child_type(r);
  }
  static inline child_type construct(Arc* a) {
    Derivation* r = malloc();
    new (r) Derivation(a);
    return child_type(r);
  }

  Arc* a;  // axioms have a==0 so we can keep same tail size/ids instead of skipping them. note that right now
  // they're not labeled w/ stateid; you can get that from their parent - exception: if axiom is
  // final state of hg. oops. so traversals take parent state as arg for now
  bool axiom() const { return !a; }
  Arc& arc() const { return *a; }

  mutable int color;
  enum {
    opened = -1,
    closed = -2,
    usermin = 0,
    usermax = INT_MAX
  };  // opened color should only exist in the middle of a traversal; afterwards, everything is marked closed
  // (note: except if cycle->exception)

  void assertUnopened() const {
    if (color == opened) SDL_THROW0(CycleException);
  }

  // works without revisiting any node
  void setColorSafe(int newcolor = usermin) const {
    SetColorSafe v(newcolor);
    visitDfs(v);
  }

  // you will want to call this between or before each traversal (the traversals don't reset the colors for
  // you!)
  void setColor(int newcolor = usermin) const {
    if (color == opened) return;
    forall (child_type const& c, this->children) { c->setColor(newcolor); }
    color = newcolor;
    // we don't bother avoiding redundant retraversal of children - so you could have an exponential
    // full-binary-tree. to avoid that, would need to make list of things to set then set them later
  }

  // can't use C=bool because vector<bool> doesn't have refs
  template <class C = char>
  struct ComputeOnceBase {
    typedef C result_type;
    /// open node, (if return value was true) child, childClose ..., close node
    /// note: !d.axiom()
    bool open(result_type&, Deriv const&) const { return true; }
    /// visitors/computers get called with axiom() to determine the initial value
    C axiom(StateId state) const { return C(); }
    void child(result_type& r, Deriv const& p, Deriv const& c, TailId i) const {}
    // (void)r;(void)p;(void)c;(void)i; } // for -Wunused
    void childClose(result_type& r, result_type const& cr, Deriv const& p, Deriv const& c, TailId i) const {}
    void close(result_type& r, Deriv const& d) const {}
    /// since we use a vector to hold the result_type for a state, you need to use non-default values if you
    /// don't want to revisit the same subtree repeatedly. return true always if you don't mind
    bool finished(result_type& r) const { return true; }
  };

  // works only for non alternative same-head deriv, e.g. 1best
  struct ComputeWeight : public ComputeOnceBase<Weight> {
    bool open(Weight& r, Deriv const& d) {
      r = d.arc().weight();
      return true;
    }
    Weight axiom(StateId) { return Weight::one(); }
    void childClose(Weight& r, Weight const& cr, Deriv const& p, Deriv const& c, TailId i) const {
      timesBy(cr, r);
    }
    bool finished(Weight& w) const { return !(w == Weight::zero()); }
  };

  struct ComputeCost : public ComputeOnceBase<FeatureValue> {
    bool open(FeatureValue& r, Deriv const& d) {
      r = d.arc().weight().getValue();
      return true;
    }
    Weight axiom(StateId) { return (FeatureValue)0; }
    void childClose(FeatureValue& r, FeatureValue const& cr, Deriv const& p, Deriv const& c, TailId i) const {
      r += cr;
    }
  };

  // TODO(graehl): when stack is limited this can cause crashes...not sure why

  // stack space limited recursion
  // this visitor gets result_type and Derivation refs. also, will skip repeated same-head subderivations
  // (i.e. only good for 1best)
  template <class V>
  typename V::result_type computeOnceDfsOneBest(std::vector<typename V::result_type> once, V& v, StateId state) {
    typedef typename V::result_type R;
    if (axiom()) return v.axiom(state);
    R& r = Util::atExpand(once, arc().head());
    if (!v.finished(r) && v.open(r, *this)) {
      StateIdContainer const& tails = arc().tails();
      for (TailId i = 0, e = (TailId) this->children.size(); i != e; ++i) {
        child_type const& c = this->children[i];
        if (c) {
          v.child(r, *this, *c, i);
          v.childClose(r, c->computeOnceDfsOneBest(once, v, tails[i]), *this, *c, i);
        }
      }
      v.close(r, *this);
    }
    return r;
  }

  template <class V>
  typename V::result_type computeOnceDfsOneBest(
      V& v, StateId state,
      StateId reserveStates = 1000)  // pass actual # of states if you like, or max used in deriv + 1
  {
    std::vector<typename V::result_type> once;
    once.reserve(reserveStates);
    return computeOnceDfsOneBest(once, v, state);
  }

  Weight weight() const {
    ComputeWeight w;
    return computeOnceDfs(w, kNoState);
  }

  Weight cost() const {
    ComputeCost w;
    return computeOnceDfs(w, kNoState);
  }

  friend inline Weight weight(Derivation const& deriv) { return deriv.weight(); }
  friend inline Weight weight(DerivP const& deriv) { return deriv->weight(); }

  friend inline FeatureValue cost(Derivation const& deriv) { return deriv.cost(); }
  friend inline FeatureValue cost(DerivP const& deriv) { return deriv->cost(); }

  struct VisitTreeBase {
    // return true to expand children
    bool open(StateId head, Deriv const& d) const { return true; }
    void child(Deriv const& p, Deriv const& c, TailId i) const {}
    void close(StateId head, Deriv const& d) const {}
  };

  struct VisitDfsBase {
    // return true to expand children
    bool open(Deriv const& d) const { return true; }
    void child(Deriv const& p, Deriv const& c, TailId i) const {}
    void close(Deriv const& d) const {}
  };

  struct SetColorSafe : public VisitDfsBase {
    int c;
    explicit SetColorSafe(int c) : c(c) {}
    void close(Deriv const& d) const { d.color = c; }
  };

  // postfix sequence of pair <StateId, Arc *>. axioms get <StateId, NULL>. output iter O gets Arc *
  template <class Out>
  struct VisitOut : public VisitTreeBase {
    Out o;
    WhichDerivationStates whichStates;  // actually, stateids
    VisitOut(Out o, WhichDerivationStates whichStates = kWholeTree) : o(o), whichStates(whichStates) {}
    void out(StateId head, Arc* a) {
      bool add = whichStates == kWholeTree || a && whichStates == kNonLeafOnly
                 || !a && whichStates == kLeafOnly;
      if (add) {
        *o = StateArc(head, a);
        ++o;
      }
    }
    void close(StateId head, Deriv const& d) { out(head, d.a); }
  };

  template <class Out>
  static VisitOut<Out> visitOut(Out const& o, WhichDerivationStates whichStates = kLeafOnly) {
    return VisitOut<Out>(o, whichStates);
  }

  template <class Container>
  static VisitOut<Util::Adder<Container> const&> visitAdd(Container& o,
                                                          WhichDerivationStates whichStates = kLeafOnly) {
    return visitOut(adder(o), whichStates);
  }

  template <class Out>
  struct VisitArcsOut : public VisitTreeBase {
    Out o;
    VisitArcsOut(Out o) : o(o) {}
    void out(Arc* a = 0) const {
      if (!a) return;
      *o = a;
      ++o;
    }
    void close(StateId, Deriv const& d) const { out(d.a); }
  };

  template <class Out>
  static VisitArcsOut<Out> visitArcsOut(Out const& o) {
    return VisitArcsOut<Out>(o);
  }

  template <class Container>
  static VisitArcsOut<Util::Adder<Container> > visitArcsAdd(Container& o) {
    return visitArcsOut(Util::adder(o));
  }

  /// all the arcs (including duplicate) used in this derivation
  void appendArcs(std::vector<Arc*>& ret, bool cycleDetect = true) {
    visitTree(visitArcsAdd(ret), kNoState, cycleDetect);
  }

  /**
     left->right postorder vector of arcs in derivation.
  */
  std::vector<Arc*> getArcs() {
    std::vector<Arc*> ret;
    appendArcs(ret);
    return ret;
  }

  template <class Container>
  void yieldAdd(Container& cont, StateId head,
                WhichDerivationStates whichStates = kLeafOnly)  // note: start state will appear in yield
  {
    VisitOut<Util::Adder<Container> > addVis(cont, whichStates);
    visitTree(addVis);
  }

  DerivationYield yield(StateId head, WhichDerivationStates whichStates = kLeafOnly) {
    DerivationYield r;
    yieldAdd(r, head, whichStates);
    return r;
  }

  template <class PushBackStates>
  void appendStatesAll(PushBackStates &out, StateId head) const {
    if (a) {
      StateIdContainer const& tails = a->tails();
      ArcId i = 0, ntails = tails.size(), nchild = this->children.size();
      assert(nchild <= ntails);
      for (; i < nchild; ++i)
        this->children[i]->appendStatesAll(out, tails[i]);
      for (; i < ntails; ++i)
        out.push_back(tails[i]);
    }
    out.push_back(head);
  }

  template <class PushBackStates>
  void appendStatesLeaf(PushBackStates &out, StateId head) const {
    if (a) {
      StateIdContainer const& tails = a->tails();
      ArcId i = 0, ntails = tails.size(), nchild = this->children.size();
      assert(nchild <= ntails);
      for (; i < nchild; ++i)
        this->children[i]->appendStatesLeaf(out, tails[i]);
      for (; i < ntails; ++i)
        out.push_back(tails[i]);
    } else
      out.push_back(head);
  }

  template <class PushBackStates>
  void appendStates(PushBackStates &out, StateId head, WhichDerivationStates whichStates = kLeafOnly) const {
    if (whichStates == kLeafOnly)
      appendStatesLeaf(out, head);
    else
      appendStatesAll(out, head);
  }

  /// assumes no cycles
  StateString yieldStates(StateId head, WhichDerivationStates whichStates = kLeafOnly) const {
    StateString r;
    appendStates(r, head, whichStates);
    return r;
  }

  /// this is used to print derivations, so will need to use explicit stack.


  struct ColorCycleGuard {
    void prepare(Derivation const& d) const { d.setColorSafe(); }
    void openNoCycle(Derivation const& d) const {
      d.assertUnopened();
      d.color = opened;
    }
    void closeNoCycle(Derivation const& d) const { d.color = closed; }
  };

  struct NoCycleGuard {
    void prepare(Derivation const& d) const {}
    void openNoCycle(Derivation const& d) const {}
    void closeNoCycle(Derivation const& d) const {}
  };

  StateId headUnlessAxiom(StateId elseState = kNoState) const { return a ? a->head() : elseState; }

  struct TreeVisitItem {
    StateId head;
    Derivation const* dp;
    TailId pushChild;  // 0 = open. -1 = close.
    bool isPop() const { return pushChild == (TailId)-1; }
    void setPop() { pushChild = (TailId)-1; }
    TreeVisitItem(StateId head, Derivation const* dp, TailId pushChild)
        : head(head), dp(dp), pushChild(pushChild) {}

    template <class Out>
    void print(Out& o) const {
      o << head << '[' << pushChild << ']' << '=' << dp;
    }

    template <class C, class T>
    friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, TreeVisitItem const& self) {
      self.print(o);
      return o;
    }
    template <class C, class T>
    friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, TreeVisitItem const* selfp) {
      o << "TreeVisitItem@0x" << (void*)selfp << ": ";
      if (selfp) selfp->print(o);
      return o;
    }
  };

  template <class V, class CycleGuard = ColorCycleGuard>
  struct TreeVisit : private CycleGuard {
    typedef std::stack<TreeVisitItem> Agenda;
    V v;
    Agenda agenda;
    TreeVisit(V const& v, Derivation const* d, StateId head = kNoState) : v(v) {
      this->prepare(*d);
      agenda.push(TreeVisitItem(head, d, 0));
      while (!agenda.empty()) processAgenda();
    }
    void processAgenda() {
      TreeVisitItem tv(agenda.top());
      agenda.pop();
      Derivation const& d = *tv.dp;  // we never have null deriv p; instead, axiom placeholder with d.a==0.
      TailId N = (TailId)d.children.size();
      if (!N) {
        v.open(tv.head, d);
        v.close(tv.head, d);
        return;
      }
      if (tv.isPop())
        closeGuarded(tv.head, d);
      else {
        TailId chTail = tv.pushChild;
        if (chTail == 0) {  // open / first child
          openGuarded(tv.head, d);
          tv.setPop();
          agenda.push(tv);
        }
        Derivation const& c = *d.children[chTail];
        v.child(d, c, chTail);

        if (chTail + 1 < N) {
          tv.pushChild = chTail + 1;
          agenda.push(tv);  // next child, if there is one
        }

        assert(d.a);  // because d has N>0 children.
        StateId childHead = d.a->tails()[chTail];
        agenda.push(TreeVisitItem(childHead, &c, 0));  // actually open child
      }
    }

    bool openGuarded(StateId head, Derivation const& d) {
      CycleGuard::openNoCycle(d);
      return v.open(head, d);
    }
    void closeGuarded(StateId head, Derivation const& d) {
      CycleGuard::closeNoCycle(d);
      v.close(head, d);
    }
  };

  template <class V>
  void visitDfs(V& v) const {
    DfsVisit<V&> visit(v, this);
  }

  template <class V>
  void visitDfs(V const& v) const {
    DfsVisit<V const&> visit(v, this);
  }

  typedef Derivation const*
      DP;  // pun: LSB 1 means it's a "close" instruction. safe because sizeof(Derivation)>1.
  // this halves the size of the stack.
  typedef std::stack<DP> Agenda;

  template <class V>
  struct DfsVisit {
    Util::Once once;
    V v;
    Agenda agenda;
    DfsVisit(V v, DP d) : v(v) {
      agenda.push(d);
      while (!agenda.empty()) processAgenda();
    }
    void processAgenda() {
      DP dp = agenda.top();
      agenda.pop();
      if (Util::lsb(dp)) return v.close(*Util::withoutLsb(dp));
      if (once.first(dp) && v.open(*dp)) {
        TailId n = (TailId)dp->children.size();
        if (!n) return v.close(*dp);
        agenda.push(Util::withLsb(dp, 1));
        children_type const& ch = dp->children;
        for (TailId i = 0; i < n; ++i)
          v.child(*dp, *ch[i], i);  // trigger these in ascending order as expected
        while (n) {
          --n;
          agenda.push(get_pointer(ch[n]));
        }
      }
    }
  };

  template <class V>
  void visitTree(V const& v, StateId head = kNoState, bool cycleDetect = true) const {
    visitTreeImpl<V const&>(v, head, cycleDetect);
  }

  template <class V>
  void visitTree(V& v, StateId head = kNoState, bool cycleDetect = true) const {
    visitTreeImpl<V&>(v, head, cycleDetect);
  }

  // expand dag to tree, while optionally still checking for cycles (throwing) if cycleDetect
  template <class V>
  void visitTreeImpl(V v, StateId head, bool cycleDetect = true) const {
    if (cycleDetect)
      TreeVisit<V, ColorCycleGuard>(v, this, head);
    else
      TreeVisit<V, NoCycleGuard>(v, this, head);
  }

  // call visitOnceDfs unless you're sure colors weren't left partially open/closed from exception etc. or
  // call setColorSafe()
  template <class V>
  void visitDfsUsingColors(V& v, bool no_cycle_allowed = true) {
    rvisitDfs(0, v, no_cycle_allowed);
  }

  template <class V>
  void rvisitDfs(V& v, bool no_cycle_allowed = true) {
    if (no_cycle_allowed)
      assertUnopened();
    else if (color == opened)
      return;
    if (color == closed) return;
    color = opened;

    v.open(*this);
    for (TailId i = 0, e = this->children.size(); i != e; ++i) {
      child_type const& c = this->children[i];
      v.child(*this, *c, i);
      c->rvisitDfs(v, no_cycle_allowed);
    }
    v.close(*this);

    color = closed;
  }

  // TODO: maybe make derivation printing order friendly for the way we binarize fsm? i.e. postorder (bottom
  // up) not preorder (top down)
  // TODO: implement a backreference-style sharing-visible printer e.g. #1([arc] #1) is a loop (instead of
  // assertUnopened())
  template <class Out>
  void print(Out& o, Hg const& h, unsigned levels = (unsigned)-1) const {
    VisitPrint<Out> printer(o, h, levels);
    visitDfs(printer);
  }

  template <class Out>
  bool openPrint(Out& o, Hg const& hg, unsigned& levels) const {
    if (!a) return false;
    o << '[';
    printArc(o, a, hg);
    o << ']';
    if (!levels) {
      o << "{...}";
      return false;
    }
    --levels;
    o << '{';
    return true;
  }

  template <class Out>
  struct VisitPrint : public VisitDfsBase {
    Out& o;
    Hg const& hg;
    unsigned levels;
    VisitPrint(Out& o, Hg const& hg, unsigned levels = (unsigned)-1) : o(o), hg(hg), levels(levels) {}
    bool open(Deriv const& d) { return d.openPrint(o, hg, levels); }
    void child(Deriv const& p, Deriv const& c, TailId i) const {
      if (i) o << ' ';
    }
    void close(Deriv const& d) {
      ++levels;
      o << '}';
    }
  };

  template <class Out>
  void rprint(Out& o, Hg const& h, unsigned levels = (unsigned)-1) const {
    // don't use dfs because i want to reprint shared substructure. color used because i want to avoid
    // infinite loops
    assertUnopened();
    color = opened;
    if (!openPrint(o, h, levels)) return;
    graehl::word_spacer sp;
    forall (child_type const& c, this->children) {
      o << sp;
      c->rprint(o, h, levels);
    }
    o << '}';
    color = closed;
  }
  template <class Out>
  friend Out& print(Out& o, self_type const& self, Hg const& h) {
    self.print(o, h);
    return o;
  }
  template <class Out>
  friend Out& print(Out& o, child_type const& c, Hg const& h) {
    c->print(o, h);
    return o;
  }
  template <class Out>
  friend Out& print(Out& o, self_type const& self, unsigned levels) {
    self.print(o, levels);
    return o;
  }
  template <class Out>
  friend Out& print(Out& o, child_type const& c, unsigned levels) {
    c->print(o, levels);
    return o;
  }

  struct ToHypergraphOnce : public VisitDfsBase {
    StateIdTranslation& sx;
    IMutableHypergraph<A>& o;
    ToHypergraphOnce(StateIdTranslation& sx, IMutableHypergraph<A>& o) : sx(sx), o(o) {}
    bool open(Deriv const& d) const {
      if (d.axiom()) return false;
      A* a = sx.copyArc(d.arc());
      o.addArc(a);
      return true;
    }
  };

  // note: for 2-best and worse, you don't have the same derivation for each instance of a vertex,
  // necessarily. so this might not make sense
  void translateToHypergraph(IHypergraph<A> const& i, IMutableHypergraph<A>& o,
                             StateIdMappingPtr const& idmap, bool best = kIsNonBest) {
    o.clear(kCanonicalLex | kDefaultProperties);
    o.forceHasArcs();
    o.takeVocabulary(i);
    StateIdTranslation sx(idmap);
    translateToHypergraph(sx, o, best);
    copySubset(sx, i, &o, kNoArcs, kStartFinal, kNoClear);
  }


  // best means that this is a 1-best derivation, so no need to split in case of subtree sharing
  IMutableHypergraph<A>* translateToHypergraphNew(IHypergraph<A> const& i, bool keepStateIds = kRenumberStates,
                                                  bool best = kIsNonBest) {
    IMutableHypergraph<A>* pOut = new MutableHypergraph<A>();
    translateToHypergraph(i, *pOut, keepStateIds, best);
    return pOut;
  }
  shared_ptr<IMutableHypergraph<A> > translateToHypergraph(IHypergraph<A> const& i,
                                                           bool keepStateIds = kRenumberStates,
                                                           bool best = kIsNonBest) {
    return shared_ptr<IMutableHypergraph<A> >(translateToHypergraphNew(i, keepStateIds, best));
  }

  // TODO: specify output vocab != input vocab?
  void translateToHypergraph(IHypergraph<A> const& i, IMutableHypergraph<A>& o,
                             bool keepStateIds = kRenumberStates, bool best = kIsNonBest) {
    if (!best)
      translateToSingleDerivationHypergraph(i, o);
    else {
      StateIdMappingPtr idmap(keepStateIds ? static_cast<StateIdMapping*>(new StateAddIdentityMapping<A>(&o))
                                           : static_cast<StateIdMapping*>(new StateAddMapping<A>(&o)));
      translateToHypergraph(i, o, idmap, best);
    }
  }

  void translateToHypergraph(StateIdTranslation& sx, IMutableHypergraph<A>& o, bool best = kIsNonBest,
                             bool resetColor = true) {
    ToHypergraphOnce v(sx, o);
    visitDfs(v);
  }

  // ok for same-head different-subderivation. must pass final state in case final = axiom to get
  // stateid/label right
  template <class V>
  typename V::result_type computeOnceDfs(V& v, StateId final) const {
    unordered_map<Derivation const*, typename V::result_type> once;
    return computeOnceDfs(once, v, final);
  }

  template <class V>
  typename V::result_type computeOnceDfs(unordered_map<Derivation const*, typename V::result_type>& once,
                                         V& v, StateId state) const {
    typedef typename V::result_type R;
    if (axiom())
      return v.axiom(state);  // NOTE: derivation axioms are placeholders and equal by pointer. so every time
    // you recompute (but don't store in once)
    R* pr;
    if (Util::update(once, this, pr)) {  // for non-axioms, one result per
      R& r = *pr;
      if (v.open(r, *this)) {
        StateIdContainer const& tails = arc().tails();
        for (TailId i = 0, e = (TailId) this->children.size(); i != e; ++i) {
          child_type const& c = this->children[i];
          v.child(r, *this, *c, i);
          v.childClose(r, c->computeOnceDfs(once, v, tails[i]), *this, *c, i);
        }
        v.close(r, *this);
      }
    }
    return *pr;
  }

  /**
     return sum of # of non-axiom nodes in derivation tree. TODO: check for
     cycles (so far only used in unit tests, so low priority)
  */
  std::size_t countNonAxioms() const {
    if (axiom()) return 0;
    std::size_t sum = 1;
    for (TailId i = 0, e = (TailId) this->children.size(); i != e; ++i)
      sum += this->children[i]->countNonAxioms();
    return sum;
  }


  /// .second is the output arc in progress for the .first head state.
  typedef std::pair<StateId, Arc*> NewStateAndInArc;

  /// for translateToSingleDerivationHypergraph
  struct ToSingleDerivationHypergraph : ComputeOnceBase<NewStateAndInArc> {
    std::vector<StateId> forAxiom;

    IHypergraph<Arc> const& ihg;
    IMutableHypergraph<Arc>& o;

    NewStateAndInArc axiom(StateId iSt) {
      assert(ihg.isAxiom(iSt));
      StateId& axiomSt = forAxiom[iSt];
      if (axiomSt == kNoState) axiomSt = o.addState(ihg.labelPair(iSt));
      return NewStateAndInArc(axiomSt, (Arc*)NULL);
    }

    ToSingleDerivationHypergraph(IHypergraph<Arc> const& ihg, IMutableHypergraph<Arc>& o)
        : ihg(ihg), o(o), forAxiom(ihg.size(), kNoState) {}

    bool open(NewStateAndInArc& r, Deriv const& d) {
      Arc const& arc = d.arc();
      r.second
          = new Arc(arc, r.first = o.addState());  // sets new head; tails to be overwritten in childClose
      o.setLabelPair(r.first, ihg.labelPair(arc.head()));
      return true;
    }

    void childClose(NewStateAndInArc& rSt, NewStateAndInArc const& cSt, Deriv const&, Deriv const&, TailId i) {
      rSt.second->tails()[i] = cSt.first;
    }

    void close(NewStateAndInArc& r, Deriv const&) {
      o.addArc(r.second);  // couldn't add until we have finalized state ids for tails (children)
    }
  };

  /**
     \param i input hg containing this derivation

     \param o output hg: hg with a single derivation, isomorphic to *this. same
     weights, same labels. o's final state is the root of our derivation's
     head. we also set start state of o to be start state of i, if any. (if
     *this derivation is leaf, then it's presumed that it's deriving i's final
     state). note that o will have sharing of subtrees if *this does.

     \param oProperties - set o's properties to this. if 0, use i's properties.

  */
  void translateToSingleDerivationHypergraph(IHypergraph<A> const& i, IMutableHypergraph<A>& o,
                                             Properties oProperties = 0) {
    StateId iRoot = axiom() ? i.final() : arc().head();
    if (iRoot == kNoState)
      SDL_THROW_LOG(Hypergraph.Derivation, EmptySetException,
                    "can't build derivation hg for singleton derivation and input hg with no final state");

    o.clear(oProperties ? oProperties : i.properties());
    o.takeVocabulary(i);

    ToSingleDerivationHypergraph toDerivationHg(i, o);

    // TODO: option for fsm only to produce topo-sorted states (nonlex last, start first, final last). can
    // also do topo sort for hg, but may not want axioms last? for now we just have state 0 final to not break
    // regression output

    StateId istart = i.start();
    o.setFinal(computeOnceDfs(toDerivationHg, iRoot).first);  // stateid 0 for final
    o.setStart(istart == kNoState ? kNoState : toDerivationHg.axiom(istart).first);
  }

  typedef child_type pointer_type;
};

// watch out: entire derivation = kAxiom implies a start=final=axiom toHypergraph i.e. containing empty string
// only w/ wt 1. in this case the state id is irrelevant - make it up
template <class A>
typename Derivation<A>::DerivP Derivation<A>::kAxiom
    = Derivation<A>::construct(0);  // 0 arc pointer - to get state you need to look at tails of parent.

// documentation only
template <class A>
struct DerivationPointer {
  typedef Derivation<A> value_type;
  typedef typename value_type::child_type pointer_type;
};


// top-down recursion. assumes 1 in arc per state else throws exception
template <class Arc>
typename Derivation<Arc>::DerivP singleDerivation(IHypergraph<Arc> const& hg, StateSet& seen, StateId from) {
  assert(from != kNoState);
  typedef Derivation<Arc> Deriv;
  typedef typename Deriv::child_type Child;
  typedef typename Deriv::children_type Children;
  if (hg.isAxiom(from)) return Deriv::kAxiom;
  if (!Util::latch(seen, from))
    SDL_THROW2(CycleException, "Cycle observed - state appeared twice in stack:", from);
  ArcIdRange r = hg.inArcIds(from);
  std::size_t nr = (std::size_t)boost::size(r);
  if (nr != 1) {
    if (nr > 1)
      SDL_THROW3(MultipleDerivationsException, from, " state has >1 incoming arcs:", nr);
    else
      SDL_THROW2(EmptySetException, "No incoming arcs for state", from);
  }
  ArcId aid = *boost::begin(r);
  Arc* arc = hg.inArc(from, aid);
  StateIdContainer const& tails = arc->tails();
  TailId n = (TailId)tails.size();
  Child ret = Deriv::construct(arc, n);
  Children& ch = ret->children;
  for (TailId i = 0; i < n; ++i) ch[i] = singleDerivation(hg, seen, tails[i]);
  Util::erase(seen, from);
  return ret;
}


///TODO: remove
template <class Arc>
typename Derivation<Arc>::DerivP singleDerivationFsmOut(IHypergraph<Arc> const& hg, StateSet& seen,
                                                        typename Derivation<Arc>::DerivP const& bottomUpPart,
                                                        StateId from, StateId final) {
  // assert(hg.isFsm())
  assert(from != kNoState);
  assert(final != kNoState);
  if (from == final) return bottomUpPart;
  typedef Derivation<Arc> Deriv;
  typedef typename Deriv::DerivP Child;
  typedef typename Deriv::children_type Children;
  if (!Util::latch(seen, from))
    SDL_THROW2(CycleException, "Cycle observed - state appeared twice in stack:", from);
  ArcId nr = hg.numOutArcs(from);
  if (nr != 1) {
    if (nr > 1)
      SDL_THROW3(MultipleDerivationsException, from, " state has >1 outgoing arcs:", nr);
    else
      SDL_THROW2(EmptySetException, "No outgoing arcs for state", from);
  }
  Arc* arc = hg.outArc(from, 0);
  if (!arc->isFsmArc()) SDL_THROW(NonFsmHypergraphException, Util::print(arc, hg));
  Child ret = Deriv::construct(arc, 2); //TODO: Deriv::construct(arc, bottomUpPart)
  ret->children[0] = bottomUpPart;
  ret->children[1] = Deriv::kAxiom;
  return singleDerivationFsmOut(hg, seen, ret, arc->head(), final);
}

template <class Arc>
typename Derivation<Arc>::DerivP singleDerivationGraphOut(IHypergraph<Arc> const& hg, StateSet& seen,
                                                        typename Derivation<Arc>::DerivP const& bottomUpPart,
                                                        StateId from, StateId final) {
  assert(hg.isGraph());
  assert(from != kNoState);
  assert(final != kNoState);
  if (from == final) return bottomUpPart;
  typedef Derivation<Arc> Deriv;
  typedef typename Deriv::DerivP Child;
  typedef typename Deriv::children_type Children;
  if (!Util::latch(seen, from))
    SDL_THROW2(CycleException, "Cycle observed - state appeared twice in stack:", from);
  ArcId nr = hg.numOutArcs(from);
  if (nr != 1) {
    if (nr > 1)
      SDL_THROW3(MultipleDerivationsException, from, " state has >1 outgoing arcs:", nr);
    else
      SDL_THROW2(EmptySetException, "No outgoing arcs for state", from);
  }
  Arc* arc = hg.outArc(from, 0);
  return singleDerivationGraphOut(hg, seen, Deriv::construct(arc, bottomUpPart), arc->head(), final);
}

// returns null pointer if empty hg
template <class Arc>
typename Derivation<Arc>::DerivP singleDerivation(IHypergraph<Arc> const& hg) {
  try {
    typedef Derivation<Arc> Deriv;
    if (hg.prunedEmpty()) return 0;
    StateSet seen(hg.size());
    if (hg.storesOutArcs() && hg.isGraph()) {
      StateId s = hg.start();
      if (s == kNoState) return 0;  // SDL_THROW(FsmNoStartException, "singleDerivation");
      return singleDerivationGraphOut(hg, seen, Deriv::kAxiom, s, hg.final());
    } else if (hg.storesInArcs()) {
      return singleDerivation(hg, seen, hg.final());
    } else
      return 0;
  } catch (EmptySetException&) {
    return 0;
  }
}

template <class Arc>
typename Derivation<Arc>::DerivAndWeight withWeight(typename Derivation<Arc>::DerivP const& p) {
  return typename Derivation<Arc>::DerivAndWeight(p, p ? p->weight() : Arc::Weight::zero());
}

/**
   for deriv.visitTree(visitor) where visitor(Arc *)
*/
template <class ArcVisitor>
struct TreeVisitArcs {
  ArcVisitor const& visitor;
  explicit TreeVisitArcs(ArcVisitor const& visitor) : visitor(visitor) {}
  template <class Arc>
  bool open(StateId, Derivation<Arc> const&) const {
    return true;
  }
  template <class Arc>
  void child(Derivation<Arc> const&, Derivation<Arc> const&, TailId) const {}
  template <class Arc>
  void close(StateId, Derivation<Arc> const& d) const {
    if (d.a) visitor(d.a);
  }
};

template <class ArcVisitor>
TreeVisitArcs<ArcVisitor> treeVisitArcs(ArcVisitor const& visitor) {
  return TreeVisitArcs<ArcVisitor>(visitor);
}


}}

#endif
