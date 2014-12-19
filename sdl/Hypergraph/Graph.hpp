











#include <algorithm>


namespace Hypergraph {

template <class SpecialSymbol>
struct isSpecial {

    return s==SpecialSymbol::ID;
  }
};

struct isEps {

    return s==EPSILON::ID;
  }
};

struct isRho {

    return s==RHO::ID;
  }
};

struct isPhi {

    return s==PHI::ID;
  }
};

struct isSigma {

    return s==SIGMA::ID;
  }
};

struct Closure {
  bool converged;
  Closure() : converged(false) {  }
  typedef std::vector<StateId> Reach;
  typedef std::vector<Reach> ReachLen;
  ReachLen reachlen; // reachlen[i] = reachable states via paths of len <=i
  Reach const& get_reach(unsigned i) const {
    return i < reachlen.size() ? reachlen[i] : reachlen.back();
  }
};

// alternative: DFS from every t to build reachable set. bit operations should make this competitive
template <class Adjs> // e.g. vector<BitSet>
void warshall_transitive_close(Adjs &adjs) {
  typedef unsigned I;

  for (I t = 0; t < N; ++t) {
    typename Adjs::value_type &a = adjs[t];
    for (I h = 0; h<N; ++h)

        adjs[t] |= adjs[h];
      }

  }
}





  typedef StateId Edge;

  typedef std::vector<Out> Outs;

  typedef std::vector<Adj> Adjs;

  Outs outs;
  typedef unsigned I;
  I size() const { // # vecs

  }

  void fromAdjs(Adjs const& as) {

    outs.clear();
    outs.resize(N);
    for (I i = 0; i<N; ++i)

  }

  void toAdjs(Adjs &as) {
    I N = size();
    as.resize(N);
    for (I i = 0; i<N; ++i) {
      Adj &a = as[i];
      a.resize(N);

    }
  }

  void transitiveClose() {
    Adjs as;
    toAdjs(as);
    warshall_transitive_close(as);
    fromAdjs(as);
  }

  template <class A, class LabelP>
  struct adder {
    Outs &outs;
    IHypergraph<A> const& hg;



    void operator()(A * a) const {
      arc(*a);
    }
    void arc(A const& a) const {


    }
  };
  template <class A, class LabelP>



  }



    for (unsigned i = 0; i<outs.size(); ++i) {
      o << i<<" ->";

          o<<' '<<d;

    }
  }
  inline friend std::ostream& operator<<(std::ostream &o, Graph const& g) { g.print(o); return o; }
};




#endif
