


























#include <boost/math/special_functions/log1p.hpp>



#if HAVE_OPENFST
// OpenFst weight compatability - needed for ToReplaceFst
#define DEFINE_OPENFST_COMPAT_FUNCTIONS(name)                           \
  static std::string const& Type() { static const std::string r = #name; return r; } \
  static const Self One() { return Self::one(); }                       \
  static const Self Zero() { return Self::zero(); }                     \










  template<class T>                             \
  bool operator cmp(c<T> const& w1,             \
                    c<T> const& w2) {           \
    return w1.getValue() cmp w2.getValue();     \
  }














namespace Hypergraph {

/**


 */
template<class T>
class FloatWeightTpl : public WeightBase {



 public:
  typedef T FloatT;



















  T &value() {

  }

  T getValue() const {

  }

  void setValue(T v) {

  }












};







// TODO: Remove this.
template<class T>
void parseWeightString(std::string const& str, FloatWeightTpl<T>* w) {
  if (!str.empty()) {

  }
}

template<class T>





inline bool approxEqual(const FloatWeightTpl<T>& w1,
                        const FloatWeightTpl<T>& w2,
                        T epsilon = FloatConstants<T>::epsilon) {

}































class BooleanWeight : public FloatWeightTpl<bool> {
 public:
  typedef bool FloatT;

  BooleanWeight() : FloatWeightTpl<bool>() {}




  typedef BooleanWeight Self;
  DEFINE_OPENFST_COMPAT_FUNCTIONS(Boolean)





  }





  }
};

template<class T>
class ViterbiWeightTpl : public FloatWeightTpl<T> {


 public:





  typedef T FloatT;








  Self & operator = (Base const& other) {
    this->setValue(other.getValue());
    return *this;
  }











  }




  }


  DEFINE_OPENFST_COMPAT_FUNCTIONS(Viterbi)
};


template<class T>
ViterbiWeightTpl<T> plus(ViterbiWeightTpl<T> const& w1,
                         ViterbiWeightTpl<T> const& w2) {
  return w1.getValue() < w2.getValue() ? w1 : w2;
}

template<class T>
ViterbiWeightTpl<T> minus(ViterbiWeightTpl<T> const& w1,
                          ViterbiWeightTpl<T> const& w2) {

  return w1; // make compiler happy
}

template<class T>
inline
ViterbiWeightTpl<T> times(ViterbiWeightTpl<T> const& w1,
                          ViterbiWeightTpl<T> const& w2) {
  if (w1 == ViterbiWeightTpl<T>::zero()
      || w2 == ViterbiWeightTpl<T>::zero())
    return ViterbiWeightTpl<T>::zero();
  return ViterbiWeightTpl<T>(w1.getValue() + w2.getValue());
}

template<class T>
inline
bool less(ViterbiWeightTpl<T> const& w1,
          ViterbiWeightTpl<T> const& w2) {
  return w1.getValue() < w2.getValue();
}

template<class T>
inline
ViterbiWeightTpl<T> divide(ViterbiWeightTpl<T> const& w1,
                           ViterbiWeightTpl<T> const& w2) {
  if (w1 == ViterbiWeightTpl<T>::zero()
      || w2 == ViterbiWeightTpl<T>::zero())
    // Technically can't divide by 0. but practically ok.
    return ViterbiWeightTpl<T>::zero();
  return ViterbiWeightTpl<T>(w1.getValue() - w2.getValue());
}

template<class T>
inline
ViterbiWeightTpl<T> invert(ViterbiWeightTpl<T> const& w) {
  return ViterbiWeightTpl<T>(-w.getValue());
}

template<class T>
inline
ViterbiWeightTpl<T> pow(ViterbiWeightTpl<T> const& w, T k) {
  return ViterbiWeightTpl<T>(k*w.getValue());
}

template<class T>
class LogWeightTpl : public FloatWeightTpl<T> {



 public:






  typedef T FloatT;










  }



  }

  Self & operator = (Base const& other) {
    this->setValue(other.getValue());
    return *this;
  }







  DEFINE_OPENFST_COMPAT_FUNCTIONS(Log)
};

template<class T>
LogWeightTpl<T> plus(LogWeightTpl<T> const& w1,
                     LogWeightTpl<T> const& w2) {
  if (w1 == LogWeightTpl<T>::zero())
    return w2;
  if (w2 == LogWeightTpl<T>::zero())
    return w1;

  const T f1 = w1.getValue();
  const T f2 = w2.getValue();
  const T d = f2-f1; // d>0 means prob1>prob2
  if (d>0)
    return LogWeightTpl<T>(f1 - Util::logExp(-d));
  else
    return LogWeightTpl<T>(f2 - Util::logExp(d));
}

template<class T>
LogWeightTpl<T> minus(LogWeightTpl<T> const& w1,
                      LogWeightTpl<T> const& w2) {
  typedef LogWeightTpl<T> W;
  if (w2 == W::zero())
    return w1;

  const T f1 = w1.getValue();
  const T f2 = w2.getValue();
  const T d = f1-f2; // d>0 means prob1>prob2
  if (d<0) // w1>w2 because cost1 < cost2
    return W(f1 - Util::logExpMinus(d));
  else if (d>=FloatConstants<T>::epsilon)

                  "a-b="<< w1.getValue() - w2.getValue() <<" greater than epsilon=" << FloatConstants<T>::epsilon);
  return W::zero();
}

template<class T>
inline
LogWeightTpl<T> times(LogWeightTpl<T> const& w1,
                      LogWeightTpl<T> const& w2) {
  if (w1 == LogWeightTpl<T>::zero()
      || w2 == LogWeightTpl<T>::zero())
    return LogWeightTpl<T>::zero();
  return LogWeightTpl<T>(w1.getValue() + w2.getValue());
}

template<class T>
inline
bool less(LogWeightTpl<T> const& w1,
          LogWeightTpl<T> const& w2) {
  return w1.getValue() < w2.getValue();
}

template<class T>
inline
LogWeightTpl<T> divide(LogWeightTpl<T> const& w1,
                       LogWeightTpl<T> const& w2) {
  if (w1 == LogWeightTpl<T>::zero()
      || w2 == LogWeightTpl<T>::zero()) {
    // Technically can't divide by zero. but practically ok.
    return LogWeightTpl<T>::zero();
  }
  return LogWeightTpl<T>(w1.getValue() - w2.getValue());
}

template<class T>
inline
LogWeightTpl<T> invert(LogWeightTpl<T> const& w) {
  return LogWeightTpl<T>(-w.getValue());
}

template<class T>
inline
LogWeightTpl<T> pow(LogWeightTpl<T> const& w, T k) {
  return LogWeightTpl<T>(k*w.getValue());
}

typedef LogWeightTpl<float> LogWeight;


inline
BooleanWeight plus(BooleanWeight const& w1,
                   BooleanWeight const& w2) {

}


inline
BooleanWeight times(BooleanWeight const& w1,
                    BooleanWeight const& w2) {

}

inline
bool less(BooleanWeight const& w1,
          BooleanWeight const& w2) {
  return w1.getValue() < w2.getValue();
}

inline
BooleanWeight divide(BooleanWeight const& w1,
                     BooleanWeight const& w2) {

  return w1; // make compiler happy
}

inline
BooleanWeight invert(BooleanWeight const& w1,
                     BooleanWeight const& w2) {

  return w1; // make compiler happy
}

inline BooleanWeight pow(BooleanWeight const& w, bool p) {
  return p ? w : BooleanWeight(true);
}

template <class Weight>
char const* weightName(Weight *) { return "Weight"; }

template <class Weight>
char const* weightName() { return weightName((Weight *)0); }

template <class T>
inline char const* weightName(ViterbiWeightTpl<T> *) { return "Viterbi"; }

template <class T>
inline char const* weightName(LogWeightTpl<T> *) { return "Log"; }





#endif
