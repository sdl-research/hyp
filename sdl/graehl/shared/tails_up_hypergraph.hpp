




// g: ref to original hypergraph
// terminal_arcs: those with empty tails (already reachable)




// nested class BestTree and Reach are algorithm objects:


   typedef TailsUpHypergraph<H> T;
   T t(g);
   T::template BestTree<> alg(t, mu, pi);
   for each final (tail) vertex v with final cost f:
   alg.axiom(v, f);
   alg.finish();

   or:

   alg.init(); // same as above but also sets pi for terminal arcs
   alg.finish();

   or:

   alg.go(); // does init() and finish()

   or ... assign to mu final costs yourself and then
   alg.queue_all(); // adds non-infinity cost only
   alg.finish();

   also
   typename RemainInfCostFact::reference hyperarc_remain_and_cost_map()
   returns pmap with pmap[hyperarc].remain() == 0 if the arc was usable from the final tail set
   and pmap[hyperarc].cost() being the cheapest cost to reaching all final tails


*/

#ifndef GRAEHL_SHARED__TAILS_UP_HYPERGRAPH_HPP
#define GRAEHL_SHARED__TAILS_UP_HYPERGRAPH_HPP








#define TUHG(x) x



#else
#define TUHG(x)

#endif





#define TUHG_CHECK_HEAP 0

#if TUHG_CHECK_HEAP


#define DEFAULT_DBG_D_ARY_VERIFY_HEAP 0
#else

#endif









#include <graehl/shared/os.hpp>
#include <graehl/shared/containers.hpp>
#include <graehl/shared/hypergraph.hpp>

#include <graehl/shared/print_read.hpp>
#include <graehl/shared/word_spacer.hpp>
#include <graehl/shared/d_ary_heap.hpp>


#include <boost/property_map/property_map.hpp>
#include <boost/type_traits/remove_reference.hpp>





namespace graehl {




DECLARE_DBG_LEVEL(TUHG)


struct BestTreeStats {



  typedef BestTreeStats self_type;
  template <class O>
  void print(O& o) const {
    o << "BestTreeStats:"
      << " blocked " << n_blocked_rereach << " negative-cost improvements"
      << "; evaluated " << n_relax << " edges"






  }
  TO_OSTREAM_PRINT
};







































*/
template <class ED>
struct HTailMult {
  ED ed;  // hyperarc with this tail
  unsigned multiplicity;  // tail multiplicity - usually 1
  HTailMult(ED e) : ed(e), multiplicity(1) {}
  typedef HTailMult<ED> self_type;


    o << '"';
    ed->print(o);
    o << '"';
    o << "\"x" << multiplicity;
  }
  TO_OSTREAM_PRINT
};

// stores G ref

          class VertMapFactory = property_factory<G, vertex_tag>,
          class ContS = VectorS  // how we hold the adjacent edges for tail vert.
          >
struct TailsUpHypergraph {
  // ED: hyperedge descriptor. VD: vertex
  typedef TailsUpHypergraph<G, EdgeMapFactory, VertMapFactory, ContS> Self;
  typedef G graph;

  graph const& g;
  VertMapFactory vert_fact;
  EdgeMapFactory edge_fact;

  typedef boost::graph_traits<graph> GT;
  typedef graehl::edge_traits<graph> ET;
  typedef graehl::path_traits<graph> PT;
  typedef typename ET::tail_iterator Ti;

  typedef typename GT::edge_descriptor ED;
  typedef typename GT::vertex_descriptor VD;

  // typedef HTailMult<ED> TailMult;
  typedef ED Tail;

  typedef typename ContS::template container<ED>::type TerminalArcs;
  TerminalArcs terminal_arcs;

  typedef typename ContS::template container<Tail>::type Adj;
  typedef typename VertMapFactory::template rebind<Adj>::impl Adjs;
  Adjs adj;
  typedef typename EdgeMapFactory::template rebind<unsigned> TailsFactory;
  typedef typename TailsFactory::impl EdgeLeftImpl;
  typedef typename TailsFactory::reference EdgeLeftMap;
  EdgeLeftImpl unique_tails;
  EdgeLeftMap unique_tails_pmap;



      : g(g_)
      , vert_fact(vert_fact_)
      , edge_fact(edge_fact_)
      , adj(vert_fact.template init<Adj>())
      , unique_tails(edge_fact.template init<unsigned>())
      , unique_tails_pmap(unique_tails)

    record_edges();
  }
  TailsUpHypergraph(graph const& g_)
      : g(g_)
      , vert_fact(VertMapFactory(g))
      , edge_fact(EdgeMapFactory(g))
      , adj(vert_fact.template init<Adj>())
      , unique_tails(edge_fact.template init<unsigned>())
      , unique_tails_pmap(unique_tails)

    record_edges();
  }


  void operator()(ED ed) {




      terminal_arcs.push_back(ed);
    } else {
      unsigned ntails_uniq = 0;
      do {


          // last hyperarc with same tail = same hyperarc
        } else {  // new (unique) tail
          add(a, Tail(ed));  // default multiplicity=1
          ++ntails_uniq;
        }


      put(unique_tails_pmap, ed, ntails_uniq);
    }
  }

  // caller must init to 0
  template <class EdgePMap>
  void count_unique_tails(EdgePMap& e) {
    for (typename Adjs::const_iterator i = adj.begin(), e = adj.end(); i != e; ++i) {
      for (typename Adj::const_iterator j = i->begin(), ej = i->end(); j != ej; ++j) {
        // const TailMult &ad=*j;
        ++e[*j];
      }
    }
  }

  typedef std::size_t heap_loc_t;

  typedef typename PT::cost_type cost_type;





















  // costmap must be initialized to initial costs (for start vertices) or infinity (otherwise) by user


  // edgecostmap should be initialized to edge costs

            // class VertexPredMap=property_factory<graph, VD>::template rebind<ED>::reference
            class VertexPredMap = typename VertMapFactory::template rebind<ED>::reference,
            // dummy_property_map

  struct BestTree {
    // typedef typename VertMapFactory::template rebind<ED>::impl DefaultPi;
    // typedef typename VertMapFactory::template rebind<cost_type>::impl DefaultMu;
    Self& tu;
    graph const& g;


    VertexCostMap mu;
    VertexPredMap pi;
    typedef typename VertMapFactory::template rebind<heap_loc_t> LocFact;
    typedef typename LocFact::impl Locs;
    typedef typename LocFact::reference LocsP;

    Locs loc;
    LocsP locp;

    // typedef typename boost::unwrap_reference<VertexCostMap>::type::value_type Cost;
    // typedef typename boost::unwrap_reference<EdgeCostMap>::type::value_type Cost;
    // typedef typename boost::property_traits<EdgeCostMap>::value_type Cost;
    typedef cost_type Cost;

    typedef typename ET::tails_size_type Ntails;

    struct RemainInf : public std::pair<Ntails, Cost> {
      typedef std::pair<Ntails, Cost> P;
      RemainInf() : P(0, PT::start()) {}
      RemainInf(Ntails const& n, Cost const& c) : P(n, c) {}
      Ntails remain() const { return this->first; }
      Cost const& cost() const { return this->second; }
      // operator Cost() const { return this->second; }
      // typedef Cost value_type;
      Ntails& remain() { return this->first; }
      Cost& cost() { return this->second; }


        o << "(" << remain() << "," << cost() << ")";
      }
      typedef RemainInf self_type;
      TO_OSTREAM_PRINT
    };

    typedef typename EdgeMapFactory::template rebind<Ntails> RemainTailsFact;  // lower bound on edge costs
    typedef typename RemainTailsFact::impl RemainTails;
    RemainTails remain;
    typedef typename RemainTailsFact::reference RemainTailsPmap;
    RemainTailsPmap remain_pmap;
    EdgeCostMap ec;

    typedef built_pmap<vertex_tag, graph, unsigned> Rereach;
    typedef boost::shared_ptr<Rereach> RereachPtr;
    typedef typename Rereach::property_map_type RereachP;
    RereachPtr rereachptr;
    RereachP rereach;




    unsigned already_reached(VD v) const {



    }
    void mark_reached(VD v) {

#ifdef NDEBUG
      else  // we don't use locp for anything if allow_rereach. but this pretties up the debug output
#endif



    }

    BestTreeStats stat;

    Heap heap;



        , g(tu.g)

        , mu(mu_)
        , pi(pi_)
        , loc(tu.vert_fact.template init<heap_loc_t>())
        , locp(loc)
        , remain(tu.edge_fact.template init<Ntails>())
        , remain_pmap(remain)
        , ec(ec)




      copy_pmap(edgeT, g, remain_pmap, tu.unique_tails_pmap);


      init_costs();
    }





    void init_costs(Cost cinit = PT::unreachable()) {



        VD v = *i;
        put(mu, *i, cinit);






      }
    }
    void init() {  // fill from hg terminal arcs


        ED h = *i;
        VD v = source(h, g);
        Cost hc = get(ec, h);
        // typename unwrap_reference<VertexCostMap>::type &dmu(mu);
        axiom(v, hc);
      }
    }






    void axiom(VD axiom, Cost const& c = PT::start(), ED h = ED()) {

      Cost& mc = mu[axiom];
      if (PT::update(c, mc)) {

        safe_queue(axiom);
        put(pi, axiom, h);
      } else {

      }
    }



    void add_unsorted(VD v) {  // call finish() after
      heap.add_unsorted(v);
      TUHG_SHOWQ(1, "added_unsorted", v);
    }

    struct add_axioms {
      BestTree& b;
      explicit add_axioms(BestTree& b) : b(b) {}

    };





      // put(locp, v,0); // not necessary: property factory (even new int[N] will always default init
      if (!is_queued(v) && get(mu, v) != PT::unreachable()) {
        add_unsorted(v);
      }
    }

    // must have no duplicates, and have already set mu


    bool is_queued(VD v) const {


      return heap.contains(v);
    }

    void pop() {
      ++stat.n_pop;
      TUHG_SHOWQ(1, "pop", heap.top());
      heap.pop();
    }

    void relax(VD head, ED e, Cost const& c) {
      ++stat.n_relax;
      Cost& m = mu[head];


      if (PT::update(c, m)) {






          ++stat.n_update;

          heap.push_or_update(head);

        TUHG_SHOWQ(3, "relaxed", head);
      } else {

      }
    }

    // skipping unreached tails:
    cost_type recompute_cost(ED e) {
      cost_type c = get(ec, e);





        cost_type tc = get(mu, t);




      }

      return c;
    }

    void reach(VD tail) {
      TUHG_SHOWQ(2, "reach", tail);
      const Adj& a = tu[tail];
      // FOREACH(const TailMult &ad, a) { // for each hyperarc v participates in as a tail
      bool tail_already = tail_already_reached(tail);
      mark_reached(tail);

      for (typename Adj::const_iterator j = a.begin(), ej = a.end(); j != ej; ++j) {
        ED e = *j;
        VD head = target(e, g);


          // don't even propagate improved costs, because we can't otherwise guarantee no infinite loop.
          ++stat.n_blocked_rereach;







        } else {



          // const TailMult &ad=*j;





          Ntails& tails_unreached = remain_pmap[e];
          if (!tail_already) {
            assert(tails_unreached > 0);
            --tails_unreached;

          }
          if (!tails_unreached)































    void finish() {

      heap.heapify();

      TUHG_SHOWP_ALL(1, "pre-finish");
      while (!heap.empty()) {
        VD top = this->top();
        TUHG_SHOWQ(6, "at-top", top);

        pop();
        reach(top);
        TUHG_SHOWP_ALL(9, "post-reach");
      }
      stat.n_unpopped = heap.size();
      TUHG_SHOWP_ALL(5, "post-finish");

    }

      init();
      finish();
    }
  };


#if 0
  //TODO:
  // reachmap must be initialized to false by user
  template <
    class VertexReachMap=typename VertMapFactory::template rebind<bool>::reference
  >
  struct Reach {
    typedef typename VertMapFactory::template rebind<bool>::impl DefaultReach;
    Self &tu;
    VertexReachMap vr;
    EdgeLeftImpl tr;
    unsigned n;
    Reach(Self &r, VertexReachMap v) : tu(r), vr(v), tr(r.unique_tails), n(0) {
      //copy_hyperarc_pmap(g, tu.unique_tails_pmap(), tr);
      //instead relying on impl same type (copy ctor)
    }

    void init_unreach() {
      typename GT::vertex_iterator i, end;
      boost::tie(i, end)==vertices(g);
      for (;i!=end;++i) {
        put(vr,*i.first, false);
      }
    }
    void init() {
      init_unreach();
      for (typename TerminalArcs::iterator i=tu.terminal_arcs.begin(), end=tu.terminal_arcs.end();i!=end;++i) {
        ED h=*i;
        VD v=source(h, g);
        (*this)(v);
      }
    }
    void finish() {
    }
    void go()
    {
      init();
      finish();
    }
    void operator()(VD v) {
      if (get(vr, v))
        return;
      ++n;
      put(vr, v, true); // mark reached
      Adj &a=tu[v];
      for(typename Adj::iterator i=a.begin(), end=a.end(); i!=end; ++i) {
        ED ed=i->ed;
        VD head=source(ed, g);
        if (!get(vr, head)) { // not reached yet
          if (--tr[ed]==0)
            (*this)(head); // reach head
        }
      }

    }
    EdgeLeftMap tails_remain_pmap() {
      return EdgeLeftMap(tr);
    }

  };

  template <class P>
  unsigned reach(VD start, P p) {
    Reach<P> alg(*this, p);
    alg(start);
    return alg.n;
  }
  template <class B, class E, class VertexReachMap>
  unsigned reach(B begin, E end, VertexReachMap p) {
    Reach<VertexReachMap> alg(*this, p);
    std::for_each(begin, end, boost::ref(alg));
    return alg.n;
  }
#endif
};

}  // graehl

#endif
