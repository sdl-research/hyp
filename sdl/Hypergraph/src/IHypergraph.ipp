


// templated IHypergraph functions


namespace Hypergraph {

template <class A>
typename IHypergraph<A>::Weight IHypergraph<A>::final(StateId s) const {
  // typedef typename A::Weight Weight;


  if (isFsm()) {
    assert(storesOutArcs());


      // TODO: binary search if sorted - if EPSILON::ID is minimum, reduces to checking first arc.


      }
    }
  }
  return Weight::zero();
}

template <class A>
std::size_t IHypergraph<A>::getNumEdges() const {
  return countArcs().n;
}

template <class A>
bool IHypergraph<A>::hasAllOut(StateId s) const {
  assert(isFsm());
  assert(storesOutArcs());




  }
  return false;
}

template <class A>
bool IHypergraph<A>::checkValid() const {


  assert(maxhead == kNoState || maxhead < N);

  assert(maxtail == kNoState || maxtail < N);

  assert(s == kNoState || s < N);
  assert(f == kNoState || f < N);
  return true;
}

template <class A>
LabelPair IHypergraph<A>::fsmLabelPair(Arc const& a) const {

}





  }



template <class A>
bool IHypergraph<A>::isFsmCheck() const {


























  return r;
}



