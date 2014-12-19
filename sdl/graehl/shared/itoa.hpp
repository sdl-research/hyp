




#ifndef GRAEHL_SHARED__ITOA_H
#define GRAEHL_SHARED__ITOA_H



#include <string>
#include <cstring>
#include <limits>
#include <cassert>






#endif








namespace graehl {


/*
// The largest 32-bit integer is 4294967295, that is 10 chars
// 1 more for sign, and 1 for 0-termination of string
const unsigned utoa_bufsize=12;
const unsigned utoa_bufsizem1=utoa_bufsize-1;
const unsigned ultoa_bufsize=22;
const unsigned ultoa_bufsizem1=utoa_bufsize-1;
*/


namespace {
char digits[] = "0123456789";
}
#endif

inline char digit_to_char(int d) {
  return

      digits[d];
#else
      '0' + d;
#endif
}



template <class Int>
char* utoa(char* buf, Int n_) {
  typedef typename signed_for_int<Int>::unsigned_t Uint;
  Uint n = n_;
  if (!n) {
    *--buf = '0';
  } else {
    Uint rem;
    // 3digit lookup table, divide by 1000 faster?
    while (n) {

      *--buf = digit_to_char(rem);
    }
  }
  return buf;
}



inline void left_pad(char* left, char* buf, char pad = '0') {

  // return buf;
}

template <class Int>
char* utoa_left_pad(char* buf, char* bufend, Int n, char pad = '0') {
  char* r = utoa(bufend, n);
  assert(buf <= r);
  left_pad(buf, r, pad);
  return buf;
}



// useful for floating point fraction output
template <class Uint_>
char* utoa_drop_trailing_0(char* buf, Uint_ n_, unsigned& n_skipped) {
  typedef typename signed_for_int<Uint_>::unsigned_t Uint;
  Uint n = n_;
  n_skipped = 0;
  if (!n) {
    *--buf = '0';
    return buf;
  } else {
    Uint rem;
    while (n) {

      if (rem) {
        *--buf = digit_to_char(rem);
        // some non-0 trailing digits; now output all digits.
        while (n) {

          *--buf = digit_to_char(rem);
        }
        return buf;
      }
      ++n_skipped;
    }
    assert(0);
    return 0;
  }
}

// desired feature: itoa(unsigned) = utoa(unsigned)
// positive sign: 0 -> +0, 1-> +1. obviously -n -> -n
template <class Int>
// typename signed_for_int<Int>::original_t instead of Int to give more informative wrong-type message?
char* itoa(char* buf, Int i, bool positive_sign = false) {
  typename signed_for_int<Int>::unsigned_t n = i;






















    *--ret = '-';
  } else if (positive_sign)
    *--ret = '+';
  return ret;
}

template <class Int>
char* itoa_left_pad(char* buf, char* bufend, Int i, bool positive_sign = false, char pad = '0') {
  typename signed_for_int<Int>::unsigned_t n = i;
  if (i < 0) {







    *buf = '-';
  } else if (positive_sign)
    *buf = '+';
  char* r = utoa(bufend, n);
  assert(buf < r);
  left_pad(buf + 1, r, pad);
  return buf;
}

template <class Int>
inline std::string itos(Int n) {
  char buf[signed_for_int<Int>::toa_bufsize];
  char* end = buf + signed_for_int<Int>::toa_bufsize;
  char* p = itoa(end, n);
  return std::string(p, end);
}

template <class Int>
inline std::string utos(Int n) {
  char buf[signed_for_int<Int>::toa_bufsize];
  char* end = buf + signed_for_int<Int>::toa_bufsize;
  char* p = utoa(end, n);
  return std::string(p, end);
}

// returns position of '\0' terminating number written starting at to
template <class Int>
inline char* append_utoa(char* to, typename signed_for_int<Int>::unsigned_t n) {
  char buf[signed_for_int<Int>::toa_bufsize];
  char* end = buf + signed_for_int<Int>::toa_bufsize;
  char* p = itoa(end, n);
  int ns = end - p;
  std::memcpy(to, p, ns);
  to += ns;
  *to = 0;
  return to;
}

// returns position of '\0' terminating number written starting at to
template <class Int>
inline char* append_itoa(char* to, typename signed_for_int<Int>::signed_t n) {
  char buf[signed_for_int<Int>::toa_bufsize];
  char* end = buf + signed_for_int<Int>::toa_bufsize;
  char* p = utoa(end, n);
  int ns = end - p;
  std::memcpy(to, p, ns);
  to += ns;
  *to = 0;
  return to;
}




#endif
