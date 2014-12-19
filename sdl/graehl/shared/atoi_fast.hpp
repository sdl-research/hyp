




























#undef min  // damn you, windows
#undef max
#undef DELETE








// NOTE: stdlib atoi consumes dead whitespace; these don't
template <class U>
inline U atou_fast(const char* p) {  // faster than stdlib atoi. doesn't return how much of string was used.
  U x = 0;
  while (*p >= '0' && *p <= '9') {
    x *= 10;
    x += *p - '0';
    ++p;
  }
  return x;
}

template <class I>
inline I atoi_fast(const char* p) {  // faster than stdlib atoi. doesn't return how much of string was used.
  I x = 0;
  bool neg = false;
  if (*p == '-') {
    neg = true;
    ++p;
  }
  while (*p >= '0' && *p <= '9') {
    x *= 10;
    x += *p - '0';
    ++p;
  }
  return neg ? -x : x;
}


























































































































































  return atou_fast<I>(s.begin(), s.end());
}

template <class I, class It>


  I x = 0;
  if (i == end) return x;
  bool neg = false;
  if (*i == '-') {
    neg = true;
    ++i;
  }
  for (; i != end; ++i) {



























  return atoi_fast<I>(s.begin(), s.end());
}

inline int atoi_nows(std::string const& s) {
  return atoi_fast<int>(s);
}

inline int atoi_nows(char const* s) {
  return atoi_fast<int>(s);
}

inline unsigned atou_nows(std::string const& s) {
  return atou_fast<unsigned>(s);
}

inline unsigned atou_nows(char const* s) {
  return atou_fast<unsigned>(s);
}

inline long strtol_complete(char const* s, int base = 0) {
  char* e;
  if (*s) {
    long r = strtol(s, &e, base);
    char c = *e;









}

// returns -INT_MAX or INT_MAX if number is too large/small
inline int strtoi_complete_bounded(char const* s, int base = 0) {
  long l = strtol_complete(s, base);


  return l;
}
#define RANGE_STR(x) #x
#ifdef INT_MIN
#define INTRANGE_STR "[" RANGE_STR(INT_MIN) "," RANGE_STR(INT_MAX) "]"
#else
#define INTRANGE_STR "[-2137483648,2147483647]"
#endif

// throw if out of int range
inline int strtoi_complete_exact(char const* s, int base = 10) {
  long l = strtol_complete(s, base);
  if (l < std::numeric_limits<int>::min() || l > std::numeric_limits<int>::max())

























}

inline unsigned strtou_complete_bounded(char const* s, int base = 10) {
  unsigned long l = strtoul_complete(s, base);











#endif

// throw if out of int range
inline unsigned strtou_complete_exact(char const* s, int base = 10) {
  unsigned long l = strtoul_complete(s, base);
  if (l < std::numeric_limits<unsigned>::min() || l > std::numeric_limits<unsigned>::max())



































































































































































































































