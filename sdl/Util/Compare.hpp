








#include <cstring>



namespace Util {

// >0 <-> a>b
// ==0 <-> a==b
template <class V>
int cmp3(V const& a, V const& b) {
  return (a<b) ? -1 : (b<a);
}

//fast, but dangerous: may overflow if e.g. a-b>=INT_MAX
inline int cmp3_unsafe(int a, int b) {
  return a-b;
}

inline int cmp3(char const* a, char const* b) {
  return std::strcmp(a, b);
}

struct cstrhash {
  inline std::size_t operator()(char const*s) const {
    std::size_t hash = 1;
    for (; *s; ++s) hash = hash * 5 + *s;
    return hash;
  }
  inline bool operator()(char const* s1, char const* s2) const {
    return strcmp(s1, s2) == 0;
  }
};

template <class Less>
struct Cmp3FromLess : public Less {
  Cmp3FromLess(Less const& l=Less()) : Less(l) {}
  Cmp3FromLess(Cmp3FromLess const& o) : Less(o) {}
  template <class V>
  int operator()(V const& a, V const& b) const {
    return Less::operator()(a, b)
        ? -1
        : Less::operator()(b, a);
  }
};

struct LessFromCmp3 {
  template <class V>
  bool operator()(V const& a, V const& b) const { return cmp3(a, b)<0; }
};

struct GreaterFromCmp3 {
  template <class V>
  bool operator()(V const& a, V const& b) const { return cmp3(a, b)>0; }
};


template<class Pair>
struct Compare1st {
  bool operator()(const Pair& s1, const Pair& s2) const {
    return s1.first < s2.first;
  }
};

template<class Pair>
struct Compare1stByValue {
  bool operator()(const Pair& s1, const Pair& s2) const {
    return *s1.first < *s2.first;
  }
};

template<class Pair>
struct Compare2nd {

  bool operator()(const Pair& s1, const Pair& s2) const {
    return s1.second < s2.second;
  }
};

template<class Pair>
struct Compare1stReverse {
  bool operator()(const Pair& s1, const Pair& s2) const {
    return s1.first > s2.first;
  }
};

template<class Pair>
struct Compare1stByValueReverse {
  bool operator()(const Pair& s1, const Pair& s2) const {
    return !(*s1.first < *s2.first);
  }
};

template<class Pair>
struct Compare2ndReverse {
  bool operator()(const Pair& s1, const Pair& s2) const {
    return s1.second > s2.second;
  }
};

/**

 */
//TODO@MD: Template the operators, not the class
template<class T>
struct LessByValue {


    return *a < *b;
  }
  bool operator()(T const* a,
                  T const* b) const {
    return *a < *b;
  }
};

/**

 */
template<class T>
struct EqualByValue {


    return *a == *b;
  }
  bool operator()(T const* a,
                  T const* b) const {
    return *a == *b;
  }
};

/**


 */
struct PairLessByValue {
  template<class P>
  bool operator()(P const& x, P const& y) {
    return *x.first < *y.first
                      || (!(*y.first < *x.first) && *x.second < *y.second);
  }
};

/**

 */
template<typename T>
struct ReplaceIfLess {
  void operator()(T const& a, T& b) const {
    if (a < b) {
      b = a;
    }
  }
};



#endif
