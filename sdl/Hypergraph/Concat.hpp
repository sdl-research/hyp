















namespace Hypergraph {


  bool mergeFsmFinalStart;
  bool cfgFinalLabel;
  bool checkEmptyRight;
  std::string defaultFinalLabel;


      , cfgFinalLabel(true)
      , checkEmptyRight(false)
      , defaultFinalLabel("ConcatFinal")
  // TODO: kCanonicalLex - requires modifying StateIdTranslation
  {}














};

// concat of yields of crossprod A x B - note: if either empty, so is result
template <class A>

  static char const* name() { return "Concat"; }
  typedef A Arc;
  typedef IHypergraph<A> HG;

  PHG rh;
  ConcatOptions opt;


    // can't update h with r if h is same as r
    copyIfSame(rh, h);
    return false;
  }

  void inplace(IMutableHypergraph<A>& l) const {
    IHypergraph<A> const& r = *rh;

    if (pruneEmpty(l)) {

      return;
    }
    if (r.prunedEmpty() || (opt.checkEmptyRight && empty(r))) {

      l.setEmpty();
      return;
    }



    assert(rf != Hypergraph::kNoState && lf != Hypergraph::kNoState);
    if (l.isFsm() && r.isFsm()) {  // for FSM, add epsilon between final1 and start2
      if (rs == Hypergraph::kNoState || ls == Hypergraph::kNoState)











    } else {  // for CFG, add S->S1 S2. note: since only one axiom is allowed, share (remap) start states
      if (rs != Hypergraph::kNoState) {
        if (ls != Hypergraph::kNoState)

        else

      }


      StateIdContainer ff;
      ff.push_back(lf);
      assert(lf != Hypergraph::kNoState);
      ff.push_back(x.stateFor(rf));
      assert(ff.back() != Hypergraph::kNoState);
      StateId newf = l.addState();
      if (opt.cfgFinalLabel)

      else


      A* ss = new A(newf, ff);
      l.addArc(ss);  // S->Sl Sr
    }
  }
  void operator()(IHypergraph<A> const& h, IMutableHypergraph<A>* o) const {

  }
};

template <class Arc>


  inplace(*pLeftHg, x);
}

template <class Arc>



  inout(leftHg, result, x);
}





