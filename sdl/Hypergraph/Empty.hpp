













#ifndef NDEBUG

#endif





namespace Hypergraph {






















struct Reach {


  HP h;
  StateSet reached;

  TailsLeft tailsleft;















































    h->forAxioms(*this);
  }


















    reach(s);
  }
























  void reach(StateId s) {








      }
    }
  }

  bool final_reached() const {

    return r;
  }



};

template <class A>


    return true;
  else {


    return !r.final_reached();
  }
}

template <class A>
bool pruneEmpty(IMutableHypergraph<A> & a)
{
  if (empty(a)) {

    return true;
  }
  return false;
}




#endif
