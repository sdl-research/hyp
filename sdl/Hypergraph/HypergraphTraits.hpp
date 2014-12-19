













#include <boost/property_map/property_map.hpp>





namespace Hypergraph {

/* following boost graph library conventions, even though hyperarcs aren't really edges (don't have a unique source but instead, several tails) */

struct HypergraphTraversalTag : boost::vertex_list_graph_tag, boost::bidirectional_graph_tag {};

template <class A>
StateId num_vertices(IHypergraph<A> const& h) {

}

template <class A>
std::size_t num_edges(IHypergraph<A> const& h) {
  return h.getNumEdges();
}


template <class A>
StateIdRange vertices(IHypergraph<A> const& h) {
  return h.getStateIds();
}

template <class A>
ArcIdRange in_edges(IHypergraph<A> const& h, StateId s) {

}

template <class A>
ArcId in_degree(IHypergraph<A> const& h, StateId s) {

}

template <class A>
ArcId out_degree(IHypergraph<A> const& h, StateId s) {

}

template <class A>
ArcId degree(IHypergraph<A> const& h, StateId s) {

}

template <class A>
ArcIdRange out_edges(IHypergraph<A> const& h, StateId s) {

}

template <class A>

  return (*(A const*)a).getNumTails();
}

template <class A>

  return (*(A const*)a).tailIds();
}

template <class A>

  return (*(A const*)a).getTail(t);
}

template <class A>


}

template <class A>

  return (*(A const*)a).getTail(0);
}

template <class A>
struct ReadEdgeCostMap {




  typedef boost::readable_property_map_tag category;


  }
};

template <class A>


}

template <class A>
ReadEdgeCostMap<A> readEdgeCostMap(IHypergraph<A> const& h) {
  return ReadEdgeCostMap<A>();
}




namespace boost {

template <class A>

  typedef A Arc;


  typedef directed_tag directed_category;
  typedef allow_parallel_edge_tag edge_parallel_category;








  typedef std::size_t edges_size_type;

};
}

namespace graehl {

template <class A>


template <class A>


  typedef typename path_traits<G>::cost_type cost_type;


  typedef tail_descriptor tails_size_type;
};


// visit is in ns graehl so ADL doesn't cause ambiguity
template <class A, class V>


}

template <class A, class V>


}

//default factory for vertex_tag is fine (descriptors start at 0)
template <class A>

  typedef edge_tag ptag;

  typedef boost::graph_traits<G> GT;
  property_factory() {}
  typename size_traits<G, ptag>::size_type N;

  property_factory(property_factory const& o) : N(o.N) {}
  template <class V>
  std::size_t init() const { return 2*N; }
  template <class V>
  struct rebind {
    property_factory &p;
    rebind(property_factory & p) : p(p) {}
    rebind(rebind const& o) : p(o.p) {}
    std::size_t init() const { return p.template init<V>(); }

    typedef boost::associative_property_map<impl> reference;
  };
};




#endif
