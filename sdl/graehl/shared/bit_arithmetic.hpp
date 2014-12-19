





#ifndef GRAEHL__SHARED__BIT_ARITHMETIC_HPP
#define GRAEHL__SHARED__BIT_ARITHMETIC_HPP



#include <limits>
#include <boost/cstdint.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <limits.h>
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif
























namespace graehl {

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;












































  i = i - ((i >> 1) & 0x55555555);
  i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
  return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}


  return count_set_bits((uint32_t)i);
}


  return count_set_bits((uint32_t)x) + count_set_bits((uint32_t)(x >> 32));
}







  return (x << k) | (x >> (32 - k));

}


  return (x >> k) | (x << (32 - k));
}






  return (x << k) | (x >> (64 - k));

}



  return (x >> k) | (x << (64 - k));
}

// bit i=0 = lsb
template <class I, class J>
inline void set(typename boost::enable_if<boost::is_integral<I> >::type& bits, J i) {
  assert(i < (CHAR_BIT * sizeof(I)));
  I mask = (1 << i);
  bits |= mask;
}

template <class I>
inline void set_mask(I& bits, I mask) {
  bits |= mask;
}

template <class I, class J>
inline void reset(typename boost::enable_if<boost::is_integral<I> >::type& bits, J i) {
  assert(i < (CHAR_BIT * sizeof(I)));
  I mask = (1 << i);
  bits &= ~mask;
}

template <class I>
inline void reset_mask(I& bits, I mask) {
  bits &= ~mask;
}

template <class I, class J>
inline void set(typename boost::enable_if<boost::is_integral<I> >::type& bits, J i, bool to) {
  assert(i < (CHAR_BIT * sizeof(I)));
  I mask = (1 << i);




}

template <class I>
inline void set_mask(I& bits, I mask, bool to) {




}

template <class I, class J>
inline bool test(typename boost::enable_if<boost::is_integral<I> >::type bits, J i) {
  assert(i < (CHAR_BIT * sizeof(I)));
  I mask = (1 << i);
  return mask & bits;
}

// if any of mask
template <class I>
inline bool test_mask(typename boost::enable_if<boost::is_integral<I> >::type bits, I mask) {
  return mask & bits;
}

// return true if was already set, then set.
template <class I, class J>
inline bool latch(typename boost::enable_if<boost::is_integral<I> >::type& bits, J i) {
  assert(i < (CHAR_BIT * sizeof(I)));
  I mask = (1 << i);
  bool r = mask & bits;
  bits |= mask;
  return r;
}

template <class I>
inline bool latch_mask(typename boost::enable_if<boost::is_integral<I> >::type& bits, I mask) {
  bool r = mask & bits;
  bits |= mask;
  return r;
}

template <class I>
inline bool test_mask_all(typename boost::enable_if<boost::is_integral<I> >::type bits, I mask) {
  return (mask & bits) == mask;
}


//
// the reason for the remove_cv stuff is that the compiler wants to turn
// the call
//    size_t x;
//    bit_rotate(x,3)
// into an instantiation of
//    template<class size_t&, int const>
//    size_t& bit_rotate_left(size_t&,int const);
//

template <class I, class J>


  typedef typename boost::remove_cv<I>::type IT;
  assert(k < std::numeric_limits<IT>::digits);
  assert(std::numeric_limits<IT>::digits == CHAR_BIT * sizeof(IT));
  return ((x << k) | (x >> (std::numeric_limits<IT>::digits - k)));
}

template <class I, class J>


  typedef typename boost::remove_cv<I>::type IT;
  assert(k < std::numeric_limits<IT>::digits);
  assert(std::numeric_limits<IT>::digits == CHAR_BIT * sizeof(IT));
  return ((x << (std::numeric_limits<IT>::digits - k)) | (x >> k));
}

/// interpret the two bytes at d as a uint16 in little endian order

// FIXME: test if the #ifdef optimization is even needed (compiler may optimize portable to same?)


  return *((const uint16_t*)(d));
#else

#endif
}

}  // graehl

#endif
