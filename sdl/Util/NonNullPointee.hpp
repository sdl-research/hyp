/** \file

    equality/hash for never-null pointers (by their pointed-to contents).
*/

#ifndef NONNULLPOINTEE_JG_2013_03_06_HPP
#define NONNULLPOINTEE_JG_2013_03_06_HPP
#pragma once

#include <boost/functional/hash.hpp>

namespace sdl { namespace Util {

/**
   compare pointer-likes by their pointed-to contents.
*/
struct NonNullPointeeEqual {
  typedef bool result_type;

  template <class Ptr1, class Ptr2>
  bool operator()(Ptr1 p1, Ptr2 p2) const {
    return *p1 == *p2;
  }
};

/**
   if value == is expensive, check pointers first.
*/
struct NonNullPointeeEqualExpensive {
  typedef bool result_type;

  template <class Ptr1, class Ptr2>
  bool operator()(Ptr1 p1, Ptr2 p2) const {
    return p1 == p2 || *p1 == *p2;
  }
};

/**
   Hash pointer-likes by their pointed-to contents.
*/

template <class T>
struct NonNullPointeeHash : boost::hash<T> {
  typedef std::size_t result_type;

  result_type operator()(T *p) const {
    return boost::hash<T>()(*p);
  }
  result_type operator()(T const* p) const {
    return boost::hash<T>()(*p);
  }
  template <class Ptr>
  result_type operator()(Ptr const& p) const {
    return boost::hash<T>()(*p);
  }
};


}}

#endif
