





#ifndef GRAEHL__SHARED__IS_NULL_HPP
#define GRAEHL__SHARED__IS_NULL_HPP











#include <graehl/shared/nan.hpp>

//#define FLOAT_NULL HUGE_VALF
//#define DOUBLE_NULL HUGE_VAL
#define FLOAT_NULL float(NAN)
#define DOUBLE_NULL double(NAN)












  return GRAEHL_ISNAN(f);
}


  f = FLOAT_NULL;  // 0./0.;
}


  return GRAEHL_ISNAN(f);
}


  f = DOUBLE_NULL;  // 0./0.;
}






struct as_null {};
// tag for constructors

#define MEMBER_IS_SET_NULL MEMBER_SET_NULL MEMBER_IS_NULL





























#endif
