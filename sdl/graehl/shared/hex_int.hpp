/*







*/

#ifndef GRAEHL__SHARED__HEX_INT_HPP
#define GRAEHL__SHARED__HEX_INT_HPP


#include <graehl/shared/print_read.hpp>

#include <graehl/shared/string_to.hpp>
#include <graehl/shared/have_64_bits.hpp>

#include <iomanip>

namespace graehl {

template <class I>
struct hex_int {
  typedef typename signed_for_int<I>::unsigned_t U;
  I i;














  hex_int() : i() {}
  explicit hex_int(I i) : i(i) {}


  operator I&() { return i; }


  typedef hex_int<I> self_type;
  TO_OSTREAM_PRINT
  FROM_ISTREAM_READ
  template <class S>
  void print(S& s) const {
    s << '0' << 'x' << std::hex << U(i) << std::dec;
  }
  template <class S>
  void read(S& s) {
    char c;
    if (!s.get(c)) return;
    if (c == '0') {
      i = 0;
      if (!s.get(c)) return;
      if (c == 'x') {
        U u;
        if (!(s >> std::hex >> u >> std::dec)) return;
        i = u;
      } else {


      }
    } else {
      // regular int, maybe signed
      s.unget();
      s >> i;
    }
  }
};

template <class I>
hex_int<I> hex(I i) {
  return hex_int<I>(i);
}

#define DEFINE_HEX_INT(i) typedef hex_int<i> hex_##i;
#define DEFINE_HEX_INTS(i) DEFINE_HEX_INTS(i) DEFINE_HEX_INTS(u##i)

DEFINE_HEX_INT(int8_t);
DEFINE_HEX_INT(int16_t);
DEFINE_HEX_INT(int32_t);
#if HAVE_64_BITS
DEFINE_HEX_INT(int64_t);
#endif











#endif
