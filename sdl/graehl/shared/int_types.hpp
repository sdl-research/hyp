


















#ifndef INT_DIFFERENT_FROM_INTN
#define INT_DIFFERENT_FROM_INTN 0
#endif





#ifndef HAVE_LONGER_LONG
#define HAVE_LONGER_LONG 0
#endif

































  template <>                                                                                           \
  struct signed_for_int<t> {                                                                            \
    typedef ut unsigned_t;                                                                              \
    typedef it signed_t;                                                                                \
    typedef t original_t;                                                                               \
    enum { toa_bufsize = 3 + std::numeric_limits<t>::digits10, toa_bufsize_minus_1 = toa_bufsize - 1 }; \
  };
































































