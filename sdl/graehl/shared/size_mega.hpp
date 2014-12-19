




#ifndef GRAEHL__SHARED__SIZE_MEGA_HPP
#define GRAEHL__SHARED__SIZE_MEGA_HPP


#include <iomanip>
#include <stdexcept>
#include <graehl/shared/stream_util.hpp>
#include <graehl/shared/print_width.hpp>
#include <graehl/shared/print_read.hpp>
#include <graehl/shared/program_options.hpp>
#include <sstream>
#include <cstddef>
#include <string>


namespace graehl {



template <class size_type, class outputstream>


  typedef double size_compute_type;
  size_compute_type thousand = decimal_thousand ? 1000 : 1024;

  size_compute_type base = thousand;
  const char* suffixes = decimal_thousand ? "kmgt" : "KMGT";
  const char* suff = suffixes;
  for (;;) {
    size_compute_type nextbase = base * thousand;
    if (size < nextbase || suff[1] == 0) {
      double d = size / (double)base;
      print_max_width_small(o, d, max_width);
      return o << *suff;
    }

    base = nextbase;
    ++suff;
  }
  return o;  // unreachable
}

template <class size_type>
size_type scale_mega(char suffix, size_type number = 1) {
  switch (suffix) {
    case 't':
      number *= (1000. * 1000. * 1000. * 1000.);
      break;
    case 'T':
      number *= (1024. * 1024. * 1024. * 1024.);
      break;
    case 'g':
      number *= (1000. * 1000 * 1000);
      break;
    case 'G':
      number *= (1024. * 1024 * 1024);
      break;
    case 'm':
      number *= (1000 * 1000);
      break;
    case 'M':
      number *= (1024 * 1024);
      break;
    case 'k':
      number *= 1000;
      break;
    case 'K':
      number *= 1024;
      break;
    default:


  }
  return number;
}

template <class size_type, class inputstream>
inline size_type parse_size(inputstream& i) {
  double number;

  char c;






  return (size_type)number;




}

template <class size_type>
inline size_type size_from_str(std::string const& str) {



  return ret;
}

template <class size_type>
inline void size_from_str(std::string const& str, size_type& sz) {
  sz = size_from_str<size_type>(str);
}


template <bool decimal_thousand = true, class size_type = double>

  typedef size_mega<decimal_thousand, size_type> self_type;
  size_type size;


  size_mega() : size() {}
  size_mega(self_type const& o) : size(o.size) {}
  size_mega(size_type size_) : size(size_) {}





  template <class Ostream>

    print_size(o, size, decimal_thousand, 5);
  }
  TO_OSTREAM_PRINT
  FROM_ISTREAM_READ
  template <class I>

    size = parse_size<size_type>(i);
  }


};
















typedef size_mega<false, double> size_bytes;
typedef size_mega<false, unsigned long long> size_bytes_integral;
typedef size_mega<false, std::size_t> size_t_bytes;
typedef size_mega<true, std::size_t> size_t_metric;
typedef size_mega<true, double> size_metric;


}  // graehl




  typedef size_t value_type;
  using namespace graehl;

  v = boost::any(graehl::size_from_str<value_type>(get_single_arg(v, values)));
}


}}

#endif
