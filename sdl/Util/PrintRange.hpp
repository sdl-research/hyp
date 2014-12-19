
















#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>











struct Sep {
  char const* s;


  Sep(char const* s = " ") : s(s), squelch(true) {}
  operator char const*() {
    if (squelch) {
      squelch = false;
      return "";
    } else
      return s;
  }


    if (squelch)
      squelch = false;
    else
      o << s;
  }





};






// pass as state to 3-arg print
struct RangeSep {
  typedef char const* S;
  S space, pre, post;
  bool spaceBefore;


  RangeSep(multiline) : space("\n "), pre("{"), post("\n}"), spaceBefore(true) {}

  RangeSep(singleline) : space(" "), pre("["), post("]"), spaceBefore() {}

};



































































  else {




    }



}


template <class O, class C>
O& printRange(O& o, C const& c, RangeSep const& r = RangeSep()) {

































  else {













































  }


// uses 3-arg print()








  else {




    }



}














template <class O, class PrintState, class C>
O& printRangeState(O& o, PrintState const& q, C const& c, RangeSep const& r = RangeSep()) {


}







// pass as state to 3-arg print
template <class X>
struct StateRangeSep {
  RangeSep r;
  X state;
  typedef char const* S;

};

template <class X>


  return StateRangeSep<typename boost::add_reference<typename boost::add_const<X>::type>::type>(x, s);
}

template <class X>
StateRangeSep<typename boost::add_reference<X>::type> stateRange(X& x, RangeSep s = RangeSep(singleline())) {
  return StateRangeSep<typename boost::add_reference<X>::type>(x, s);
}

template <class V>
inline void print(std::ostream& o, V const& v, RangeSep const& r) {
  printRange(o, v, r);
}

template <class V, class X>
inline void print(std::ostream& o, V const& v, StateRangeSep<X> const& r) {
  printRangeState(o, r.state, v, r.r);
}

template <class X>
struct StateRangeRangeSep {
  RangeSep outer;
  StateRangeSep<X> inner;
  StateRangeRangeSep(X state, RangeSep outer = RangeSep(multiline()), RangeSep inner = RangeSep(singleline()))
      : outer(outer), inner(state, inner) {}
};

template <class X>




}

template <class X>


  return StateRangeRangeSep<typename boost::add_reference<X>::type>(x, outer, inner);
}

template <class V, class X>
void print(std::ostream& o, V const& v, StateRangeRangeSep<X> const& rr) {
  printRangeState(o, rr.inner, v, rr.outer);
}




































































































#endif
