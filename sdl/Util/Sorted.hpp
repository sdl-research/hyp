// Copyright 2014 SDL plc
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/** \file

    sorted vector of values from a set of values
*/


#ifndef SORTED_JG2012119_HPP
#define SORTED_JG2012119_HPP
#pragma once

#include <algorithm>
#include <vector>
#include <list>
#include <sdl/Util/ShrinkVector.hpp>
#include <sdl/Util/VoidIf.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>

namespace sdl {
namespace Util {

template <class Val, class Alloc, class Compare>
void sort(std::list<Val, Alloc>& l, Compare c) {
  l.sort(c);
}
template <class Val, class Alloc>
void sort(std::list<Val, Alloc>& l) {
  l.sort();
}

template <class Val, class Alloc, class Compare>
void sort(std::vector<Val, Alloc>& l, Compare c) {
  std::sort(l.begin(), l.end(), c);
}

template <class Cont>
void sort(Cont& cont) {
  std::sort(cont.begin(), cont.end());
}

template <class Cont, class Before>
void sort(Cont& cont, Before before) {
  std::sort(cont.begin(), cont.end(), before);
}

template <class Cont>
void unique(Cont& cont) {
  cont.erase(std::unique(cont.begin(), cont.end()), cont.end());
}

template <class Cont, class Before>
void unique(Cont& cont, Before before) {
  cont.erase(std::unique(cont.begin(), cont.end(), before), cont.end());
}

template <class Cont, class Before>
void sortUnique(Cont& cont, Before before) {
  sort(cont, before);
  unique(cont, before);
}

template <class Cont>
void sortUnique(Cont& cont) {
  if (cont.size() <= 1) return;
  sort(cont);
  unique(cont);
}

template <class Cont>
void mergeResize(Cont& merged, Cont const& a, Cont const& b) {
  merged.resize(a.size() + b.size());
  merged.erase(std::merge(a.begin(), a.end(), b.begin(), b.end(), merged.begin()), merged.end());
}

template <class Cont, class Before>
void mergeResize(Cont& merged, Cont const& a, Cont const& b, Before before) {
  merged.resize(a.size() + b.size());
  merged.erase(std::merge(a.begin(), a.end(), b.begin(), b.end(), merged.begin(), before), merged.end());
}

template <class Order>
struct Reverse : Order {
  Reverse(Order const& o = Order()) : Order(o) {}
  template <class X>
  bool operator()(X const& a, X const& b) const {
    return !Order::operator()(b, a);
  }
};

/**
   for std::sort ascending: a < b.
*/
struct Ascending {
  template <class X>
  bool operator()(X const& a, X const& b) const {
    return a < b;
  }
};

struct Descending {
  template <class X>
  bool operator()(X const& a, X const& b) const {
    return b < a;
  }
};

/// by first/second - useful for maps
struct FirstAscending {
  template <class X>
  bool operator()(X const& a, X const& b) const {
    return a.first < b.first;
  }
};

struct FirstDescending {
  template <class X>
  bool operator()(X const& a, X const& b) const {
    return b.first < a.first;
  }
};

struct SecondAscending {
  template <class X>
  bool operator()(X const& a, X const& b) const {
    return a.second < b.second;
  }
};

struct SecondDescending {
  template <class X>
  bool operator()(X const& a, X const& b) const {
    return b.second < a.second;
  }
};

/**
   for beforeings of iterators or (smart) pointers
*/
template <class Before>
struct ByPtr : Before {
  template <class X>
  bool operator()(X const& a, X const& b) const {
    return Before::operator()(*a, *b);
  }
};

template <class Value, class Before = Ascending>
struct Sorted : std::vector<Value> {
  Sorted() {}
  template <class Range>
  explicit Sorted(Range const& r) {
    set(r);
  }
  template <class Iter>
  Sorted(Iter i, Iter end) {
    set(i, end);
  }

  template <class Range>
  void set(Range const& r) {
    Util::reinitRange(*this, r);
    sort();
  }
  template <class Iter>
  void set(Iter i, Iter end) {
    Util::reinit(*this, i, end);
    sort();
  }
  void sort() { std::sort(this->begin(), this->end(), Before()); }
};

template <class Map, class Enable = void>
struct ValueCopyable {
  typedef typename Map::value_type type;
};

template <class Map>
struct ValueCopyable<Map, typename Util::VoidIf<typename Map::key_type>::type> {
  typedef std::pair<typename Map::key_type, typename Map::mapped_type> type;
};

/// shortcut for a constructor call e.g. in a forall (...). we're hoping return
/// value optimization saves us from an actual copy
template <class Before, class Range>
inline Sorted<typename ValueCopyable<Range>::type, Before> sorted(Range const& r) {
  return Sorted<typename ValueCopyable<Range>::type, Before>(r);
}

template <class Before, class Range>
inline Sorted<typename ValueCopyable<Range>::type, Before> sorted(Range const& r, Before const&) {
  return Sorted<typename ValueCopyable<Range>::type, Before>(r);
}

template <class Map>
inline Sorted<std::pair<typename Map::key_type, typename Map::mapped_type> > sortedByKey(Map const& r) {
  return Sorted<typename ValueCopyable<Map>::type, FirstAscending>(r);
}

template <class Vec, class Index = std::size_t>
struct IndexedLess {
  Vec const& vec;
  explicit IndexedLess(Vec const& vec) : vec(vec) {}
  bool operator()(Index i, Index j) const { return vec[i] < vec[j]; }
};


template <class Indices, class Vec>
void permute(Indices const& indices, Vec& vec) {
  typedef typename Indices::value_type Index;
  Index N = indices.size();
  assert(vec.size() == indices.size());
  Vec original(vec);
  for (Index i = 0; i < N; ++i) vec[i] = original[indices[i]];
}


/**
   when Vec1[i] and Vec2[i] are conceptually pairs - sort according to Vec1
   value only (not the pair). we do this by sorting an array of indexes. note
   that it *might* be faster to create a reinterpret_cast-able type with a
   modified assignment operator that also assigns in vec2. but this would rely
   on details of std::sort, which might use move, std::swap ... we don't know

   TODO: implement our own sort and save the copies, which will certainly be
   faster
*/
template <class Vec1, class Vec2>
void sortParallelArrays(Vec1& vec1, Vec2& vec2) {
  typedef typename Vec1::size_type Index;
  typedef std::vector<Index> VecIndex;
  Index N = vec1.size();
  assert(N == vec2.size());
  VecIndex sorti(N);
  for (Index i = 0; i < N; ++i) sorti[i] = i;
  std::sort(sorti.begin(), sorti.end(), IndexedLess<Vec1>(vec1));
  permute(sorti, vec1);
  permute(sorti, vec2);
}


template <class Iter>
inline bool isRangeSorted(Iter i, Iter const& end) {
  if (i == end) return true;
  for (;;) {
    Iter last((i));
    ++i;
    if (i == end) return true;
    if (*i < *last) return false;
  }
}

template <class Iter, class Before>
inline bool isRangeSorted(Iter i, Iter const& end, Before before) {
  if (i == end) return true;
  for (;;) {
    Iter last((i));
    ++i;
    if (i == end) return true;
    if (before(*i, *last)) return false;
  }
}

template <class Cont>
inline bool isSorted(Cont const& cont) {
  return isRangeSorted(cont.begin(), cont.end());
}

template <class Cont, class Before>
inline bool isSorted(Cont const& cont, Before before) {
  return isRangeSorted(cont.begin(), cont.end(), before);
}


template <class Iter>
inline bool isRangeSortedAndUnique(Iter i, Iter const& end) {
  if (i == end) return true;
  for (;;) {
    Iter last((i));
    ++i;
    if (i == end) return true;
    if (!(*i < *last)) return false;
  }
}

template <class Cont>
inline bool isSortedAndUnique(Cont const& cont) {
  return isRangeSortedAndUnique(cont.begin(), cont.end());
}

// TODO: polyphase merge, d_ary_heap (d=4) merge


}}

#endif
