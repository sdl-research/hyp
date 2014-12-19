


















#include <stack>

















#include <boost/range/algorithm/transform.hpp>


#include <functional>




namespace Hypergraph {






template <class T>
struct First
{
  typedef T const& result_type;
  template <class Pair>
  result_type operator()(Pair const& pair) const
  { return pair.first; }
};

inline void appendStates(StateString &ss, DerivationYield const& dy)
{
  std::size_t i = ss.size();
  ss.resize(i+dy.size());
  StateString::iterator o = ss.begin()+i;

}

inline StateString statesFrom(DerivationYield const& dy)
{
  StateString r;
  appendStates(r, dy);
  return r;
}

/* A derivation is a tree of arc handles w/ #children = #tails. there may be cycles and reuse. cycles would lead to memory leaks, though (refcount). TODO: provide a "safe free" or use an external mem pool instead of refcount. A "color" field is included for cycle-avoiding and copy-subtree-once depth first search and other algorithms

   note: labels aren't included in the derivation; see the original HG for that. state ids are included only indirectly by referring to original arcs (especially for axioms, you have to see the stateid of the parent hyperedge). i may want to change this (include stateid in axioms or even redundantly in all). axioms don't have a nullary edge. they have arc handle a=0. maybe i want to represent states explicitly in the future, or else strip from children all the axioms in BestPath?

*/
//TODO: exclude axioms from derivation tree? i.e. derivation child i is the ith non-axiom tail in arc()? can make some code (printing, visiting, etc) behave the same for either decision

template <class A>
struct Derivation

{

  typedef A Arc;
  typedef typename Arc::Weight Weight;



  typedef IHypergraph<Arc> H;
  typedef Derivation self_type;
  typedef self_type Deriv;
  typedef child_type DerivP;








  DerivP pointer()
  {
    return DerivP(this);
  }


  static inline Derivation *malloc() { return (Derivation*)Alloc::malloc(sizeof(Derivation)); }

  {
    Derivation *r = malloc();
    new (r) Derivation(a, nchild);
    return child_type(r);
  }

  {
    Derivation *r = malloc();
    new (r) Derivation(a);
    return child_type(r);
  }


  bool axiom() const { return !a; }
  Arc & arc() const
  {

  }

  mutable int color;
  enum { opened = -1, closed = -2, usermin = 0, usermax = INT_MAX }; // opened color should only exist in the middle of a traversal; afterwards, everything is marked closed (note: except if cycle->exception)

  void assertUnopened() const
  {

  }

  // works without revisiting any node
  void setColorSafe(int newcolor = usermin) const
  {
    SetColorSafe v(newcolor);

  }

  // you will want to call this between or before each traversal (the traversals don't reset the colors for you!)
  void setColor(int newcolor = usermin) const
  {
    if (color==opened) return;

      c->setColor(newcolor);
    }
    color = newcolor;
    // we don't bother avoiding redundant retraversal of children - so you could have an exponential full-binary-tree. to avoid that, would need to make list of things to set then set them later
  }

  // can't use C=bool because vector<bool> doesn't have refs
  template <class C = char>
  struct ComputeOnceBase
  {
    typedef C result_type;



    {
      return true;
    }



    // (void)r;(void)p;(void)c;(void)i; } // for -Wunused

    void close(result_type &r, Deriv const& d) const {}

    bool finished(result_type &r) const
    {

    }
  };


  struct ComputeWeight : public ComputeOnceBase<Weight>
  {







      timesBy(cr, r);
    }
    bool finished(Weight &w) const
    {

    }
  };














  // TODO(graehl): when stack is limited this can cause crashes...not sure why


  // this visitor gets result_type and Derivation refs. also, will skip repeated same-head subderivations (i.e. only good for 1best)
  template <class V>

  {
    typedef typename V::result_type R;






        child_type const& c = this->children[i];

          v.child(r, *this, *c, i);

        }
      }
      v.close(r, *this);
    }
    return r;
  }

  template <class V>

  {
    std::vector<typename V::result_type> once;
    once.reserve(reserveStates);

  }


  {
    ComputeWeight w;

  }





  }
























  {
    // return true to expand children
    bool open(Deriv const& d) const { return true; }

    void close(Deriv const& d) const {}
  };


  {
    int c;
    explicit SetColorSafe(int c) : c(c) {}

    {
      d.color = c;
    }
  };

  //postfix sequence of pair <StateId, Arc *>. axioms get <StateId, NULL>. output iter O gets Arc *


  {




    {




        *o = StateArc(head, a);
        ++o;
      }
    }

    {

    }
  };






















































  {


  }


  {
    DerivationYield r;

    return r;
  }


  {

  }































































































































  template <class V>

  {

  }

  template <class V>

  {

  }





  template <class V>

  {








    }










































  }

  // call visitOnceDfs unless you're sure colors weren't left partially open/closed from exception etc. or call setColorSafe()
  template <class V>

  {
    rvisitDfs(0, v, no_cycle_allowed);
  }

  template <class V>
  void rvisitDfs(V &v, bool no_cycle_allowed = true)
  {
    if (no_cycle_allowed) assertUnopened();
    else if (color==opened) return;
    if (color==closed) return;
    color = opened;

    v.open(*this);

      child_type const& c = this->children[i];
      v.child(*this, *c, i);
      c->rvisitDfs(v, no_cycle_allowed);
    }
    v.close(*this);

    color = closed;
  }

  //TODO: maybe make derivation printing order friendly for the way we binarize fsm? i.e. postorder (bottom up) not preorder (top down)
  //TODO: implement a backreference-style sharing-visible printer e.g. #1([arc] #1) is a loop (instead of assertUnopened())




  }


  {

    H const& h;
    unsigned levels;

    bool open(Deriv const& d)
    {
      if (d.axiom()) return false;
      o<<'[';
      writeArc(o, d.arc(), h);
      o<<']';
      if (!levels) {
        o << "{...}";
        return false;
      }
      --levels;
      o<<'{';
      return true;
    }

    {
      if (i)
        o<<' ';
    }
    void close(Deriv const& d) {
      ++levels;
      o<<'}';
    }
  };



    // don't use dfs because i want to reprint shared substructure. color used because i want to avoid infinite loops

    assertUnopened();
    color = opened;
    o<<'[';
    writeArc(o, arc(), h);
    o<<']';
    if (!levels) {
      o << "{...}";
      return;
    }
    --levels;
    o<<'{';
    graehl::word_spacer sp;

      o << sp;
      c->rprint(o, h, levels);
    }
    o<<'}';
    color = closed;
  }










  {
    StateIdTranslation &sx;
    IMutableHypergraph<A> &o;
    ToHypergraphOnce(StateIdTranslation &sx, IMutableHypergraph<A> &o) : sx(sx), o(o) {}
    bool open(Deriv const& d) const
    {




      return true;
    }
  };

  //note: for 2-best and worse, you don't have the same derivation for each instance of a vertex, necessarily. so this might not make sense

  {

    o.forceHasArcs();




  }




























  {





























      }






















































































  }

  typedef child_type pointer_type;
};





// documentation only
template <class A>
struct DerivationPointer
{
  typedef Derivation<A> value_type;
  typedef typename value_type::child_type pointer_type;
};




























































































































#endif
