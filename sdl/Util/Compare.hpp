/** \file

    common compare/less/equal function objects.
*/

#ifndef SDL_UTIL_COMPARE_H
#define SDL_UTIL_COMPARE_H
#pragma once

#include <cstring>
#include <sdl/SharedPtr.hpp>

namespace sdl {
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
  typedef bool result_type;
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
   Compares two shared_ptr objs by value
 */
//TODO@MD: Template the operators, not the class
template<class T>
struct LessByValue {
  bool operator()(shared_ptr<T> const& a,
                  shared_ptr<T> const& b) const {
    return *a < *b;
  }
  bool operator()(T const* a,
                  T const* b) const {
    return *a < *b;
  }
};

/**
   Compares two shared_ptr objs by value
 */
template<class T>
struct EqualByValue {
  bool operator()(shared_ptr<T> a,
                  shared_ptr<T> b) const {
    return *a == *b;
  }
  bool operator()(T const* a,
                  T const* b) const {
    return *a == *b;
  }
};

/**
   A 'less' functor comparing two pairs; each pair element is
   taken by value (i.e., dereferenced).
 */
struct PairLessByValue {
  template<class P>
  bool operator()(P const& x, P const& y) {
    return *x.first < *y.first
                      || (!(*y.first < *x.first) && *x.second < *y.second);
  }
};

/**
   Replaces b with a if a is smaller than b.
 */
template<typename T>
struct ReplaceIfLess {
  void operator()(T const& a, T& b) const {
    if (a < b) {
      b = a;
    }
  }
};

}}

#endif
