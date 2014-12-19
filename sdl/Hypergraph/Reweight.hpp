





















namespace Hypergraph {


  //  typedef boost::optional<double> D;



  }
  ReweightOptions() {
    head_normalize = fsm_normalize = false;
    set_null(set);
    random_add = add = scale = set;

  }


















    if (non_null(set))
      w = set;











      w += add;



      w *= scale;


  }

  bool head_normalize;
  bool fsm_normalize;
  bool trivial() const {



  }































  bool normalize() const {
    return head_normalize || fsm_normalize;
  }
  void validate() {
    if (head_normalize && fsm_normalize)

  }
























};

template <class A>



  ReweightOptions opt;

  Sums normsum;
  const bool norm;


  {

    if (opt.normalize())

  }














    assert(norm);

  }


    A &a = *ap;








  }
};





}






  }
  Properties inAddProps() const {

  }



  }
};









#endif
