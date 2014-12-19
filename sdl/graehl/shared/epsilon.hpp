




#ifndef GRAEHL__SHARED__EPSILON_HPP
#define GRAEHL__SHARED__EPSILON_HPP


#include <cmath>
#include <algorithm>



















#endif

namespace graehl {


#ifndef EPSILON

#endif
static const double ONE_PLUS_EPSILON = 1 + EPSILON;
#endif

//#define ONE_PLUS_EPSILON (1+EPSILON)


/*




  | u - v | <= e * |u| and | u - v | <= e * |v|
  defines a "very close with tolerance e" relationship between u and v
  (1)

  | u - v | <= e * |u| or | u - v | <= e * |v|
  defines a "close enough with tolerance e" relationship between u and v
  (2)







  | u - v | / |u| <= e and | u - v | / |v| <= e
  | u - v | / |u| <= e or | u - v | / |v| <= e
  (1`)
  (2`)
*/


// intent: if you want to be conservative about an assert of a<b, test a<(slightly smaller b)


template <class Float>
inline Float slightly_larger(Float target) {
  return target * ONE_PLUS_EPSILON;
}

template <class Float>
inline Float slightly_smaller(Float target) {
  return target * (1. / ONE_PLUS_EPSILON);
}

// note, more meaningful tests exist for values near 0, see Knuth
// (but for logs, near 0 should be absolute-compared)
inline bool same_within_abs_epsilon(double a, double b, double epsilon = EPSILON) {
  return std::fabs(a - b) < epsilon;
}


  return std::fabs(a - b) <= epsilon * std::fabs(a);
}





  using std::fabs;
  double diff = fabs(a - b);
  return diff <= epsilon * fabs(a) && diff <= epsilon * fabs(b);
}





  using std::fabs;
  double diff = fabs(a - b);
  return diff <= epsilon * fabs(a) || diff <= epsilon * fabs(b);
}









  using std::fabs;
  double diff = fabs(a - b);
  double scale = std::max(min_scale, std::max(fabs(a), fabs(b)));
  return diff <= epsilon * scale;




























































































































































}
}































#endif
