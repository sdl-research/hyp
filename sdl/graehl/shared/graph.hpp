






#ifndef GRAEHL_SHARED__GRAPH_HPP
#define GRAEHL_SHARED__GRAPH_HPP



#include <boost/graph/graph_traits.hpp>




namespace graehl {






















  typedef typename boost::graph_traits<G>::edge_descriptor descriptor;
  typedef typename boost::graph_traits<G>::edge_iterator iterator;

};



  typedef typename boost::graph_traits<G>::vertex_descriptor descriptor;
  typedef typename boost::graph_traits<G>::vertex_iterator iterator;

};

template <class Tag, class G, class E, class F>
void visit(Tag t, G& g, F f);

template <class Tag, class G, class E>


}

template <class Tag, class G, class E>

  return vertices(g);
}

template <class Tag, class G, class E>

  return edges(g);
}

// const g
template <class Tag, class G, class E>

  return vertices(g);
}

template <class Tag, class G, class E>

  return edges(g);
}


// TEST: does V v mean we get a chance to V &v or V const& v ???

template <class G, class T, class F>
inline void visit(T, G& g, F const& f) {



}

template <class G, class T, class F>
inline void visit(T, G& g, F& f) {



}

// const g
template <class G, class T, class F>
inline void visit(T, G const& g, F const& f) {



}

template <class G, class T, class F>
inline void visit(T, G const& g, F& f) {



}




#endif
