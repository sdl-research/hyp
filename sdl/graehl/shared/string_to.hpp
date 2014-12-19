








  USAGE:

  X string_to<X>(string);
  string to_string(X);





  fast implementation for string, int, unsigned, float, double, and, if HAVE_LONGER_LONG=1, long

  also: to_string calls itos utos etc




  ----






  see also boost.spirit and qi for template-parser-generator stuff that should be about as fast!

  http://alexott.blogspot.com/2010/01/boostspirit2-vs-atoi.html
  and
  http://stackoverflow.com/questions/6404079/alternative-to-boostlexical-cast/6404331#6404331

*/

#ifndef GRAEHL__SHARED__STRING_TO_HPP
#define GRAEHL__SHARED__STRING_TO_HPP























#endif
#ifndef HAVE_STRTOUL
#define HAVE_STRTOUL 1
#endif




















#include <limits>  //numeric_limits
#include <string>
#include <sstream>
#include <stdexcept>



#include <graehl/shared/have_64_bits.hpp>

#include <graehl/shared/itoa.hpp>


#include <graehl/shared/ftoa.hpp>


#endif






















template <class I, class To>

  i >> to;
  if (i.fail()) return false;
  if (complete) {
    char c;
    return !(i >> c);
  }
  return true;
}

template <class Str, class To>

  std::istringstream i(str);
  return try_stream_into(i, to, complete);
}


  return utos(x);
}


  return itos(x);
}

#if HAVE_LONGER_LONG

  x = strtoi_complete_exact(s.c_str());
}

  x = strtoi_complete_exact(s);
}
#endif


  x = strtol_complete(s.c_str());
}

  x = strtol_complete(s);
}




}




#endif






}





























































}




}



}



}







  x = strtoul_complete(s.c_str());
}

  x = strtoul_complete(s);
}

// FIXME: end code duplication

















/* 9 decimal places needed to avoid rounding error in float->string->float. 17 for double->string->double
   in terms of usable decimal places, there are 6 for float and 15 for double
*/
inline std::string to_string_roundtrip(float x) {


}



  return ftos(x);
#else


#endif
}

inline std::string to_string_roundtrip(double x) {


}



  return ftos(x);
#else


#endif
}





    x = std::atof(s);
}





}


}


}

template <class Str>

  str = to;
  return true;
}


  return d;
}

































































































template <class Str>


}







    std::ostringstream o;
    o << d;
    return o.str();

  }
  template <class Str>



  }































}






}










































































































































































    unsigned written = (unsigned)snprintf(begin() + sz, maxLen, fmt, val);








































































































































































































































































}





#endif
