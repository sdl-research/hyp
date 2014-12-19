








































/*

  idea:
  an alg may require some property of HG.

  for the special case of unary transforms (1 in 1 out), maybe we, the caller,
  want to forget (destroy) the original input, or maybe the transform is nondestructive,
  but adds an index. in that case, we want to update in place. but some HG
  aren't mutable. so hold by smart_ptr and maybe update the ptr if mutation isn't supported.





  some algs are more naturally expressed by mutating the input HG. others
  require copying. our purpose in Transform.hpp is to support both ways of
  calling, when just one is implemented

  pass HGs by smart_ptr which may be updated to a new HG (copy of IHG) as
  needed. or if the pointed-to-HG was mutable (and pointer was non-const), make
  the required change. also, second version which always makes a copy.

  when we have more than one type of HG impl, (right now it's just Mutable
  underlying), will need a factory object to make the suggested type of
  replacement HG if the smart_ptr must be updated. can also allocate for you in the always copy version

*/




   struct T : TransformBase;





   struct TM : TransformBase, TransformMainBase<TransformBase> // does static_cast in TMB succeeed?
   struct TM : TransformBase, TransformMainBase<TM, TransformBase> // two static_cast - would definitely work

*/





















namespace Hypergraph {









































static const Properties maybeClearProps = 0;  // kAllProperties

// we seem to be forcing props repeatedly, but at least one (sort) may need re-forcing after adding arcs.






















      m.forceProperties(t.inAddProps());
      t.inplace(m);







  t.inout(i, &m);
  m.forcePropertiesOnOff(t.outAddProps(), t.outSubProps());
}




  if (t.needs(m))
    inplace_always(m, t);

    m.forcePropertiesOnOff(t.outAddProps(), t.outSubProps());
}













  o->forcePropertiesOnOff(t.outAddProps(), t.outSubProps());
  t.inout(*pi, o);
  o->forcePropertiesOnOff(t.outAddProps(), t.outSubProps());
}

















  } else {
    inout_impl(i, o, t);
  }
}



  if (!t.needs(i)) {

    return;
  }




template <class A>











  if (pl.get() == &r) {

    return true;
  }
  return false;
}





  IHypergraph<A> const& i = *holdi;
  if (!t.needs(i)) {

    return;
  }
  MutableHypergraph<A>* o = new MutableHypergraph<A>(t.newOutAddProps());
  cpi.reset(o);




    inplace_always(*o, t);
  } else {

    inout_impl(*i2, o, t);
  }
}




  return inplace(*pi, t);
}

// return true if mutated, false if copy


  IHypergraph<A>& i = *pi;

    inplace(boost::static_pointer_cast<IMutableHypergraph<A> >(pi));
    return true;
  }

  inplace(cpi, t);

  return false;
}





    return true;
  }
  return false;
}


































































// since we use templates, you don't actually need to inherit from this. but provide these members.












  static char const* name() { return "TransformBase"; }

  template <class A>
  bool checkInputs(IHypergraph<A> const& h) const {
    return true;
  }





















  template <class A>
  bool needs(IHypergraph<A> const& h) const {
    return true;
  }
  template <class A>
  bool needsCopy(IHypergraph<A> const& h) const {
    return false;  // e.g. if &h=&rhs for binary transform
  }

  // named differently so we don't hide when we override same name
  template <class A>
  void inplace(IMutableHypergraph<A>& m) const {

  }
  template <class A>
  void inout(IHypergraph<A> const& h, IMutableHypergraph<A>* o) const {

  }


  Properties inAddProps() const { return InAddProps; }
  Properties outAddProps() const { return OutAddProps; }

  Properties outSubProps() const { return OutSubProps; }











































































};









  template <class Config>
  void configure(Config& config) {



























}









}





























































#endif
