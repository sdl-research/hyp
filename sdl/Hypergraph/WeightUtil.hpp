









































































































































































































































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
struct Identity {
  FloatWeightTpl<T> operator()(T prob) const {
    return prob;
  }
};


























