





































namespace Util {



















template <class T>
inline T logPlus(T f1, T f2) {
  T d = f1 - f2;
  if (d > 0) {
    return f2 - logExp(-d);

    return f1 - logExp(d);
  }
}

/**




*/
template <class FloatT>
struct NeglogTimesFct {








};

/**




*/
template <class FloatT>
struct NeglogDivideFct {








};

/**




*/
template <class FloatT>
struct NeglogPlusFct {




      return b;
    }

      return a;
    }
    if (a <= b) {



    }
  }











};

/**




*/
template <class FloatT>
struct NeglogSubFct {




    const FloatT d = a - b;
    if (d <= 0)
      return a - Util::logExpMinus(d);
    else if (d < FloatConstants<FloatT>::epsilon)
      return zero();
    else

    return b;
  }









};




#endif
