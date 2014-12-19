





#ifndef GRAEHL_SHARED__MAX_WIDTH_HPP
#define GRAEHL_SHARED__MAX_WIDTH_HPP


#include <cmath>
#include <iomanip>
#include <iostream>
#include <graehl/shared/print_read.hpp>

namespace graehl {

template <class Stream>

  typedef Stream stream_type;
  stream_type* pstream;
  std::ios::fmtflags saved_flags;
  local_stream_flags(stream_type& stream) : pstream(&stream), saved_flags(stream.flags()) {}

};

template <class Stream>

  typedef Stream stream_type;
  stream_type* pstream;
  std::streamsize saved_prec;


  local_precision(stream_type& stream) : pstream(&stream), saved_prec(stream.precision()) {}

};

template <class O>

  local_stream_flags<O> f;
  local_precision<O> p;
  local_stream_format(O& o) : f(o), p(o) {}
};

// similar to print_width but only works (well) for 10000>d>1 i.e. for size_mega
template <class C, class T>

  typedef std::basic_ostream<C, T> stream_t;
  local_stream_format<stream_t> save(o);
  int p = 0;
  if (width > 0) {
    if (d >= 0 && d < 10000) {
      if (d < 10) {
        p = width - 2;
      } else if (d < 100) {
        p = width - 3;
      } else if (d < 1000) {
        p = width - 4;
      }
    }
  }

  return o << d;
}


// for a positive exponent!

  unsigned dexp = (exp < 100 ? 2 : 3);
  int r = width - dexp - 3;  // . and e and +- (+ is mandatory w/ scientific, as is padding exp to 2 digits)
  return r > 0 ? r : 0;
}

/*
On the default floating-point notation, the precision field specifies the
maximum number of meaningful digits to display both before and after the decimal
point, while in both the fixed and scientific notations, the precision filed
specifies exactly how many digits to display after the decimal point, even if
they are trailing decimal zeros.

bug (w/ g++ lib): scientific forces minimum precision=2
*/
// minprec sig digits within at most width chars
template <class C, class T>

  const double epsilon = 1e-8;
  //    return o << std::setprecision(width) << d;
  if (width >= 20 || d == 0. || width <= 0) {
    o << d;
    return o;
  }

  typedef std::basic_ostream<C, T> stream_t;
  local_stream_format<stream_t> save(o);
  double p = d;
  if (d < 0) {
    p = -d;
    --width;
  }
  double wholes = std::log10(p * (1 + epsilon));  // 1: log=0, digits=1

  if (p < 1) {
    int a = (int)-wholes;
    const int dot0 = 2;
    int need = dot0 + minprec + a;

    return o << std::setprecision(width - dot0 - a) << d;
  } else {
    int a = (int)wholes;
    int need = 1 + a;

    o << std::fixed;
    int need_dot = need + 1;
    return o << std::setprecision(need_dot < width ? width - need_dot : 0) << d;
  }
}







template <class C, class T>

  print_width(o, d, width);
  return o;
}






template <class C, class T>

#if 1
  return print_width(o, d, width);
#else
  typedef std::basic_ostream<C, T> stream_t;
  local_stream_format<stream_t> save(o);
  if (width > 0) {
    double p = std::fabs(d);

    int wholes = (int)std::log10(p);  // 1: log=0, digits=1
    int need = 1 + (wholes < 0 ? -wholes : wholes);
    if (need > width) {
      int unit_e_exp = 1 + 1 + 2;

    } else {
      o << std::fixed;
      int need_dot = need + 1;
      if (need_dot < width)
        o << std::setprecision(width - need_dot);
      else
        o << std::setprecision(0);
    }
  }
  return o << d;
#endif
}

template <class C, class T>

  typedef std::basic_ostream<C, T> stream_t;
  local_stream_format<stream_t> save(o);
  int p = 0;
  if (width > 0) {
    if (d >= 0 && d < 10000) {
      if (d < 10) {
        p = width - 2;
      } else if (d < 100) {
        p = width - 3;
      } else if (d < 1000) {
        p = width - 4;
      }
    }
  }

  return o << d;
}
}

#ifdef SAMPLE
#undef SAMPLE
#include <fstream>
#include <iostream>
using namespace std;
using namespace graehl;



}


  //    cout << fixed << setprecision(0) << 1.234;cout << " .\n";
  //    cout << scientific << setprecision(0) << 1.234e4;cout << " .\n";
  d(.0008123, 5);

  double b = 10;
  for (unsigned w = 4; w < 8; ++w) {
    double x = 5.4321;
    cout << "\n\nwidth=" << w << ":\n";
    for (unsigned i = 0; i < 8; ++i, x *= b) {
      print_width(cout, x, w);
      cout << '\t';
      print_width(cout, 1e99 * x, w);
      cout << '\t';
      print_width(cout, 1 / x, w);
      cout << '\t';
      print_width(cout, -x, w);
      cout << '\t';
      print_width(cout, -1 / x, w);
      cout << "\t.\n";
    }
  }

  return 0;
}
#endif

#endif
