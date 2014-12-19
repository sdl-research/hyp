




#ifndef GRAEHL__SHARED__PERCENT_HPP
#define GRAEHL__SHARED__PERCENT_HPP


#include <graehl/shared/print_width.hpp>
#include <iosfwd>

namespace graehl {

template <int width = 5>

  double frac;
  percent(double f) : frac(f) {}
  percent(double num, double den) : frac(num / den) {}

  template <class C, class T>

    print_max_width_small(o, get_percent(), width - 1);
    o << '%';
  }
  typedef percent<width> self_type;
};

template <class C, class T, int W>

  p.print(os);
  return os;
}

template <int width = 5>

  double num, den;

  portion(double num, double den) : num(num), den(den) {}



  template <class C, class T>

    o << percent<width>(num, den);
    o << " (" << num << "/" << den << ")";
  }
};

template <class C, class T, int W>

  p.print(os);
  return os;
}




#endif
