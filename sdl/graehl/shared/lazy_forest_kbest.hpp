#ifndef GRAEHL__SHARED__LAZY_FOREST_KBEST_HPP
#define GRAEHL__SHARED__LAZY_FOREST_KBEST_HPP















/**

   Uses a top-down, lazy, best-first kbest-derivation from a forest algorithm
   described at http://www.cis.upenn.edu/~lhuang3/huang-iwpt-correct.pdf.
   Generating, say, the 1-million best should be no problem.









   a lazy_forest (graehl/shared/lazy_forest_kbest.hpp) that is essentially a
   (non-lazy) copy of the parse forest, with per-or-node priority queues and
   memo facilitating the lazy kbest generation.

   You can generate kbest items incrementally, on demand.


















**/

/*
  struct Factory
  {
  typedef Result *derivation_type;
  static derivation_type NONE() { return (derivation_type)0;}
  static derivation_type PENDING() { return (derivation_type)1;}


  {
  return new Result(prototype, old_child, new_child, changed_child_index);
  }




  };



  bool derivation_better_than(derivation_type a, derivation_type b);

  or



  or specialize in ns graehl:

  namespace graehl {
  template<>
  struct lazy_kbest_derivation_traits<Deriv> {

  {

  }
  {

  then build a lazy_forest<Factory> binary hypergraph
*/

// TODO: use d_ary_heap.hpp (faster than binary)






#ifndef ERRORQ
#include <iostream>






#endif






#include <boost/noncopyable.hpp>
#include <graehl/shared/percent.hpp>
#include <graehl/shared/assertlvl.hpp>


#include <vector>
#include <stdexcept>
#include <algorithm>


#include <graehl/shared/test.hpp>
# define LAZY_FOREST_EXAMPLES
#endif



















#  define KBESTINFOT(x)
#  define KBESTNESTT
# endif











# endif
#endif




namespace graehl {




  return o << '[' << bp[0] << ',' << bp[1] << ']';
}

/*
  template <class Deriv> inline
  bool derivation_better_than(const Deriv &me, const Deriv &than)
  {
  return me > than;
  }
*/



template <class Deriv>

  // Deriv(a) < Deriv(b) iff b is better than a.

};





}












};

/**




   build a copy of your (at most binary) derivation forest, then query its root for the 1st, 2nd, ... best
   <code>
   struct DerivationFactory
   {


   /// can override derivation_better_than(derivation_type, derivation_type).
   /// derivation_type should be a lightweight (value) object

   typedef Result *derivation_type;






   static derivation_type PENDING();
   static derivation_type NONE();
   /// derivation_type must support: initialization by (copy), assignment, ==, !=





   {
   return new Result(prototype, old_child, new_child, changed_child_index);
   }
   };
   </code>
**/
// TODO: implement unique visitor of all the lazykbest subresults (hash by pointer to derivation?)

struct none_t {};
struct pending_t {};


  template <class Init>
  dummy_init_type(Init const& g) {}
};















  enum { trivial = 1 };  // avoids printing stats on % filtered
  /*
    typedef some_stateful_derivation_predicate filter_type;


  */

    template <class Init>
    dummy_init_type(Init const& g) {}
  };

  typedef dummy_init_type init_type;
  permissive_kbest_filter() {}
  permissive_kbest_filter(init_type const& g) {}

  template <class E>




  template <class O, class E>



};


  typedef permissive_kbest_filter filter_type;
  bool filter_init() const  // gets passed to constructor for filter_type
  {
    return true;
  }
};

template <class filter>

  typedef filter filter_type;

};


template <class filter>

  typedef filter filter_type;
  filter f;
  copy_filter_factory() {}
  copy_filter_factory(filter const& f) : f(f) {}
  copy_filter_factory(copy_filter_factory const& o) : f(o.f) {}

};








































  typedef std::size_t count_t;




  count_t n_passed;
  count_t n_filtered;
  bool trivial_filter;

    trivial_filter = trivial_filt;

  }



  template <class C, class T>


    if (trivial_filter) {
      assert(!n_filtered);

      return;
    }


  }
  typedef lazy_kbest_stats self_type;
};

template <class C, class T>

  kb.print(os);
  return os;
}














template <class DerivationFactory, class FilterFactory = permissive_kbest_filter_factory>


                    {

 public:
  typedef DerivationFactory derivation_factory_type;
  typedef FilterFactory filter_factory_type;
















  typedef typename filter_factory_type::filter_type filter_type;
  typedef lazy_forest<derivation_factory_type, filter_factory_type> forest;
  typedef forest self_type;



  typedef typename derivation_factory_type::derivation_type derivation_type;






  /// bool Visitor(derivation, ith) - if returns false, then stop early.
  /// otherwise stop after generating up to k (up to as many as exist in
  /// forest)
  template <class Visitor>





      EIFDBG(LAZYF, 2, SHOWM2(LAZYF, "enumerate_kbest-pre", i, *this));

      if (ith_best == NONE()) break;
      if (!visit(ith_best, i)) break;
    }


  }


    pq.swap(o.pq);
    memo.swap(o.memo);
  }









  struct hyperedge {
    typedef hyperedge self_type;
    /// antecedent subderivation forest (OR-nodes). if unary,
    /// child[1]==NULL. leaves have child[0]==NULL also.
    forest* child[2];
    /// index into kth best for that child's OR-node: which of the possible
    /// children we chose (0=best-cost, 1 next-best ...)

    /// the derivation that resulted from choosing the childbp[0]th out of
    /// child[0] and childbp[1]th out of child[1]
    derivation_type derivation;




    }


    void set(derivation_type _derivation, forest* c0, forest* c1) {
      childbp[0] = childbp[1] = 0;
      child[0] = c0;
      child[1] = c1;
      derivation = _derivation;
    }
    // NB: std::pop_heap puts largest element at top (max-heap)



    }
    // means: this<o iff o better than this. good.

    template <class O>
    void print(O& o) const {
      o << "{hyperedge(";
      if (child[0]) {
        o << child[0] << '[' << childbp[0] << ']';

      }
      o << ")=" << derivation;
      o << '}';
    }














  };






































  template <class O>

    print(o, 2);
  }



  template <class O>

    o << "{NODE @" << this << '[' << memo.size() << ']';

    o << " #queued=" << pq_size();



      o << pq[i];
      o << "}}}";
    }



      o << "}}}";
      if (s > 2) {


        o << "}}}";
      }
      // o << " pq=" << pq;
      // o<< pq; // " << memo=" << memo
    }
    o << '}';
  }

  /// if you have any state in your factory, assign to it here










  /// return the nth best (starting from 0) or NONE() (test with is_null(d)
  /// if the finite # of derivations in the forest is exhausted.
  /// IDEA: LAZY!!
  /// - only do the work of computing succesors to nth best when somebody ASKS





  /// (this is true after sort() or first add_sorted()) IF: a new n is asked
  /// for: must be 1 off the end of memo; push it as PENDING and go to work:
  /// {get succesors to pq[0] and heapify, storing memo[n]=top(). if no more
  /// left, memo[n]=NONE()} You're DONE when: pq is empty, or memo[n] = NONE()


    if (n < memo.size()) {


      if (memo[n] == PENDING()) {






        memo[n] = NONE();
      }
      return memo[n];  // may be NONE
    } else {
      assertlvl(19, n == memo.size());

      memo.push_back(PENDING());
      derivation_type& d = memo.back();
      for (;;) {





        if (filter().permit(d)) {


          return d;



        d = PENDING();
      }
    }
  }
  /// returns last non-DONE derivation (one must exist!)
  derivation_type last_best() const {
    assertlvl(11, memo.size() && memo.front() != NONE());

    assertlvl(11, memo.size() > 1);
    return *(memo.end() - 2);
  }




    for (;;) {
      if (r == 0) return 0;
      --r;

    }
  }

  /// returns best non-DONE derivation (one must exist!)
  derivation_type first_best() const {
    assertlvl(11, memo.size() && memo.front() != NONE() && memo.front() != PENDING());
    return memo.front();
  }

  /// Get next best derivation, or NONE if no more.
  //// INVARIANT: top() contains the next best entry







    pop();  // since we made a copy already into pending...






    if (pending.child[0]) {  // increment first




      }
    }
    if (pq.empty())
      return NONE();
    else {
      EIFDBG(LAZYF, 2, SHOWM2(LAZYF, "next_best", top().derivation, this));
      return top().derivation;
    }
  }




    memo.push_back(r);
    if (!filter().permit(r))
      throw std::runtime_error("lazy_forest_kbest: the first-best derivation can never be filtered out");

  }

  /// may be followed by add() or add_sorted() equivalently



    add(r, left, right);

  }

  /// must be added from best to worst order ( r1 > r2 > r3 > ... )









    }

    add(r, left, right);

  }



  /// may add in any order, but must call sort() before any get_best()



    pq.push_back(hyperedge(r, left, right));

  }


    std::make_heap(pq.begin(), pq.end());


  }









    if (pq.size() > 2 && pq[1] < pq[2])  // STL heap=maxheap. 2 better than 1.
      std::swap(pq[0], pq[2]);
    else
      std::swap(pq[0], pq[1]);
    return !best_is_selfloop();
  }







      throw lazy_derivation_cycle();


  }

  // note: postpone_selfloop() may make this not true.


  }

  // private:
  typedef std::vector<hyperedge> pq_t;
  typedef std::vector<derivation_type> memo_t;

  // MEMBERS:
  pq_t pq;  // INVARIANT: pq[0] contains the last entry added to memo
  memo_t memo;

  // try worsening ith (0=left, 1=right) child and adding to queue


    lazy_forest& child_node = *pending.child[i];

    derivation_type old_child = child_node.memo[child_i];







    if (new_child != NONE()) {  // has child-succesor






      push(pending);
    }
    --child_i;


  }

  void push(const hyperedge& e) {
    pq.push_back(e);
    std::push_heap(pq.begin(), pq.end());




  }
  void pop() {




    pq.pop_back();
  }

};





#endif
