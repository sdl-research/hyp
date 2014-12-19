




  ordered multi-hypergraph

  G A type that is a model of Graph.
  g An object of type G.
  v An object of type boost::graph_traits<G>::vertex_descriptor.

  Associated Types
  boost::graph_traits<G>::traversal_category

  This tag type must be convertible to adjacency_graph_tag.

  boost::graph_traits<G>::adjacency_iterator




  Valid Expressions


  Return type: std::pair<adjacency_iterator, adjacency_iterator>


  http://www.boost.org/libs/graph/doc/adjacency_list.html

*/

#ifndef GRAEHL_SHARED__HYPERGRAPH_HPP
#define GRAEHL_SHARED__HYPERGRAPH_HPP











#include <boost/graph/graph_traits.hpp>



namespace graehl {



};










  typedef T graph;
  typedef boost::graph_traits<graph> GT;
  typedef typename graph::hyperarc_descriptor hyperarc_descriptor;
  typedef typename graph::hyperarc_iterator hyperarc_iterator;
  typedef typename graph::tail_descriptor tail_descriptor;
  typedef typename graph::tail_iterator tail_iterator;









};




  typedef typename hypergraph_traits<G>::hyperarc_descriptor descriptor;
  typedef typename hypergraph_traits<G>::hyperarc_iterator iterator;

};

template <class G>

  return hyperarcs(g);
}
#endif

/*
  struct NoWeight {
  void setOne() {}
  };
*/



// usually: K = key *, you have array of key at key *: vec ... vec+size
// construct OffsetArrayPmap(vec, vec+size) and get an array of size Vs (default constructed)

/*Iterator Must be a model of Random Access Iterator.


  T The value type of the iterator.         std::iterator_traits<RandomAccessIterator>::value_type
  R The reference type of the iterator.     std::iterator_traits<RandomAccessIterator>::reference




/*template <class V, class O>
  struct ArrayPMap;
*/


/*: public ByRef<ArrayPMapImp<V, O> >
  {
  typedef ArrayPMapImp<V, O> Imp;
  typedef typename Imp::category category;
  typedef typename Imp::key_type key_type;
  typedef typename Imp::value_type value_type;
  explicit ArrayPMap(Imp &a) : ByRef<Imp>(a) {}
  };
*/

// HyperarcLeftMap = count of unique tails for each edge, should be initialized to 0 by user
// e.g.
/*
  typedef typename boost::graph_traits<G>::hyperarc_index_map HaIndex;
  typedef ArrayPMap<unsigned, HaIndex> PMap;
  typename PMap::Imp arc_remain(num_hyperarcs(g), HaIndex(g));
  TailsUpHypergraph<G, PMap> r(g, PMap(arc_remain));
*/

/*
  template <class G, class P1, class P2>
  void copy_hyperarc_pmap(G &g, P1 a, P2 b) {
  visit(hyperarc_tag, make_indexed_copier(a, b));
  }
*/




#endif
