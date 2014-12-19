


















#include <boost/unordered_map.hpp>




#include <boost/unordered_set.hpp>



























// non-commutative
inline void hashvalCombine(std::size_t& s, std::size_t v) {
  s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
}

template <class B>
inline std::size_t hashPairInt(std::size_t s, B b) {
  hashvalCombine(s, b);
  return s;
}

template <class I>
inline std::size_t hashRange(I i, I end, std::size_t s = 0) {


  return s;
}

template <class C>
inline std::size_t hashCont(C const& c) {
  return hashRange(c.begin(), c.end(), c.size());
}

struct HashCont {
  template <class C>
  std::size_t operator()(C const& c) const {
    return hashCont(c);
  }
};

struct PointedHashCont {
  template <class C>
  std::size_t operator()(C const* c) const {
    return hashCont(*c);
  }
};

struct PointedEqual {
  template <class A, class B>
  bool operator()(A const* a, B const* b) const {
    return *a == *b;
  }
  typedef bool return_type;
};

struct PointedLess {
  template <class A, class B>
  bool operator()(A const* a, B const* b) const {
    return *a < *b;
  }
  typedef bool return_type;
};





















































































}









}












#endif
