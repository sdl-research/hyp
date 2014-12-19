
















namespace Hypergraph {

template <class A>

  out << sid;














  }
  return out;
}

template <class A>
void print(std::ostream& o, StateId s, IHypergraph<A> const& hg) {
  writeState(o, s, hg);
}








/**


*/
template <class A>






  return out;
}

template <class A>
void print(std::ostream& o, A const& a, IHypergraph<A> const& hg) {
  writeArc(o, a, hg);
}




}






/**

*/
template <class A>


  typedef A Arc;
  typedef typename Arc::Weight Weight;

  HypergraphTextWriter() {}

  virtual ~HypergraphTextWriter() {}








  }

  /**


  */

};

/**


*/
template <class A>
class HypergraphTextWriter_Bottomup : public HypergraphTextWriter<A> {

 public:
  typedef typename A::Weight Weight;





      out << "FINAL <- ";

    }

      out << "START <- ";

    }





      }
    }
  }

 private:


};

/**


*/
template <class A>
class HypergraphTextWriter_Topdown : public HypergraphTextWriter<A> {

 public:
  typedef typename A::Weight Weight;





      out << "FINAL <- ";

    }

      out << "START <- ";

    }



    }
  }

 private:

};

template <class A>

  bool e = fullEmptyCheck ? empty(hypergraph) : hypergraph.prunedEmpty();




      w.reset(new HypergraphTextWriter_Topdown<A>(pVoc));

      w.reset(new HypergraphTextWriter_Bottomup<A>(pVoc));
    else

    w->write(out, hypergraph);
  }
  return out;
}



  writeHypergraph(out, hypergraph);
  return out;
}
























#endif
