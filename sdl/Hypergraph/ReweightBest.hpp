












*/










namespace Hypergraph {




  }
};

// nbest visitor
template <class A>

  ReweightBestOptions const& opt;
  typedef ReweightDerivation self_type;
  IMutableHypergraph<A>& h;



  typedef Derivation<A> Deriv;

    if (d.axiom()) return;
    A& a = d.arc();

  }



    return true;
  }
};



  ReweightBestOptions opt;
  template <class A>

    ReweightDerivation<A> v(h, opt);
    visitNbest(v, 1, h);
  }
};





