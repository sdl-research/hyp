




*/





















namespace Hypergraph {






















template <class W>





























}




template <class W>



































  }


template <class W>























   }































template <class W>


}

/**

*/
template <class W>












}

template <class W, class F>


}

template <class W>









    return W::zero();








    return W::one();
  W r;
  parseWeightString(str, &r);
  return r;
}


template <class W>

  return true;
}

template <class W>

  plusBy(a, b);
  return true;
}

template <class T>

  return (a.getValue()<b.getValue());
}

template <class T>

  if (a.getValue()<b.getValue()) {
    b = a;
    return true;
  }
  return false;
}














inline
bool plusByChanges(BooleanWeight const& a, BooleanWeight const &b) {
  return a.getValue()<b.getValue();
}

inline
bool plusByChanged(BooleanWeight const& a, BooleanWeight &b) {
  if (a.getValue()<b.getValue()) {
    b = a;
    return true;
  }
  return false;
}

template<class T>
T neglogToProb(T const& v) {
  return std::exp(-v);
}

template<class T>
T probToNeglog(T const& prob) {
  return -std::log(prob);
}

template<class T>
struct ProbToNeglog {
  FloatWeightTpl<T> operator()(T prob) const {
    return FloatWeightTpl<T>(-std::log(prob));
  }
};

template<class T>
struct Identity {
  FloatWeightTpl<T> operator()(T prob) const {
    return prob;
  }
};

#if HAVE_OPENFST























#endif
