// Copyright 2014-2015 SDL plc
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

    dynamic_bitset wrapper for iterating the set bits (like
    std::set::const_iterator), and boost hash_value. include this instead of
    directly including dynamic_bitset
*/

#ifndef SDL_UTIL__BITSET_HPP
#define SDL_UTIL__BITSET_HPP
#pragma once

// FIXME: someone is using ns std - I wanted to call setValue free fn 'set' but couldn't
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)
#endif
// please include this file before anyone else includes dynamic_bitset
#ifndef BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
// unfortunately needed to reasonably implement hash_value
#endif
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#include <iterator>
#include <memory>

#include <sdl/Util/Debug.hpp>
#include <sdl/Util/ShrinkVector.hpp>
#include <sdl/Util/Add.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/distance.hpp>
#include <boost/range/iterator.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple.hpp>

namespace sdl {
namespace Util {

#define bitset_grow(n) (2 * (n))

typedef Util::dynamic_bitset<std::size_t, std::allocator<std::size_t> > BitSet;

/// these macros allow us to use an enhanced dynamic_bitset instead of
/// boost::dynamic_bitset - see diffs vs. master in
/// https://github.com/graehl/boost/tree/dynamic_bitset
#define SDL_BITSET_OPEN_NAMESPACE \
  namespace sdl { \
  namespace Util {
#define SDL_BITSET_CLOSE_NAMESPACE \
  }                                \
  }
#define SDL_BITSET_NAMESPACE sdl::Util

template <class I, class A = std::allocator<I> >
struct SetBitsIter : std::iterator<std::forward_iterator_tag, bool> {
  typedef SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> B;
  B const* b;
  typedef typename B::size_type Sz;
  Sz i;
  //  SetBitsIter(SetBitsIter const& o) : b(o.b), i(o.i) {}
  SetBitsIter() : b(), i(B::npos) {}
  explicit SetBitsIter(B const& b) : b(&b), i(b.find_first()) {}
  typedef SetBitsIter Self;
  inline void advance() {
    UTIL_DBG_MSG(10, "advance " << i << " = ");
    i = b->find_next(i);
    CUTIL_DBG_MSG(10, b->find_next(i) << "\n");
  }
  Self& operator++() {
    advance();
    return *this;
  }
  Self operator++(int) {
    SetBitsIter r = *this;
    advance();
    return r;
  }
  Sz operator*() const { return i; }
  bool done() const { return i == B::npos; }

  bool operator==(SetBitsIter const& o) const {
    assert(o.b == 0 || b == 0 || o.b == b);
    return i == o.i;
  }
  bool operator!=(SetBitsIter const& o) const {
    assert(o.b == 0 || b == 0 || o.b == b);
    return i != o.i;
  }
};

template <class I, class A>
SetBitsIter<I, A> begin(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c) {
  return SetBitsIter<I, A>(c);
}

template <class I, class A>
SetBitsIter<I, A> end(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c) {
  return SetBitsIter<I, A>();
}

template <class I, class A>
std::pair<SetBitsIter<I, A>, SetBitsIter<I, A> > setBits(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c) {
  return std::pair<SetBitsIter<I, A>, SetBitsIter<I, A> >(SetBitsIter<I, A>(c), SetBitsIter<I, A>());
}

typedef SetBitsIter<std::size_t> BitSetOnesIter;

struct PrintSetBits {
  template <class I, class A>
  inline friend void print(std::ostream& o, SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& b,
                           PrintSetBits const& p) {
    typedef SetBitsIter<I, A> Set;
    printRange(o, Set(b), Set());
  }
};

// bit 0 goes at the end (right), big-endian style, just how it's represented in word(block)-space (lsb is bit
// 0)
struct PrintBitsRightToLeft {
  template <class I, class A>
  inline friend void print(std::ostream& o, SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& b,
                           PrintBitsRightToLeft const& p) {
    o << b;
  }
};

struct PrintBits {
  template <class I, class A>
  inline friend void print(std::ostream& o, SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& b,
                           PrintBits const& p) {
    o << "Bits:\n";
    for (typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i = 0, e = b.size(); i != e; ++i)
      o << ' ' << i << '=' << b.test(i) << '\n';
  }
};
}
}

SDL_BITSET_OPEN_NAMESPACE
// ADL for bitset means we need to be in boost ns

template <class I, class A>
inline std::size_t hash_value(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c) {
  return boost::hash_range(c.bits_.begin(), c.bits_.end());
}

SDL_BITSET_CLOSE_NAMESPACE

/// boost::range traits:
namespace boost {

template <class I, class A>
sdl::Util::SetBitsIter<I, A> range_begin(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c) {
  return sdl::Util::SetBitsIter<I, A>(c);
}

template <class I, class A>
sdl::Util::SetBitsIter<I, A> range_end(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c) {
  return sdl::Util::SetBitsIter<I, A>();
}

template <class I, class A>
sdl::Util::SetBitsIter<I, A> range_begin(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& c) {
  return sdl::Util::SetBitsIter<I, A>(c);
}

template <class I, class A>
sdl::Util::SetBitsIter<I, A> range_end(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& c) {
  return sdl::Util::SetBitsIter<I, A>();
}

template <class I, class A>
typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type range_calculate_size(
    SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c) {
  return c.count();
}

template <class I, class A>
struct range_const_iterator<SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> > {
  typedef sdl::Util::SetBitsIter<I, A> type;
};

template <class I, class A>
struct range_mutable_iterator<SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> > {
  typedef sdl::Util::SetBitsIter<I, A> type;
};
}

namespace sdl {
namespace Util {

typedef SetBitsIter<std::size_t> BitSetIter;

/**
   key (an index) is templated otherwise the generic contains will be used in preference to this
*/
template <class I, class A, class K>
bool contains(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c, K key) {
  return c.test((typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type)key);
}

template <class I, class A, class K>
bool containsGrow(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& c, K key) {
  typedef typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type Size;
  return (Size)key < c.size() && c.test((Size)key);
}

// if you use vector<bool> etc as bitset, watch for conflict with Contains.hpp
template <class I, class A>
bool latch(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& c,
           typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i) {
  return !c.test_set(i);
}

template <class I, class A>
bool resizeFor(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& v,
               typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i) {
  typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type n = v.size();
  if (i >= n) {
    v.resize(std::max(bitset_grow(n), i + 1));
    return true;
  }
  return false;
}

template <class I, class A>
bool resizeFor(std::vector<I, A>& v, std::size_t i) {
  std::size_t n = v.size();
  if (i < n)
    return false;
  else {
    v.resize(i + 1);
    return true;
  }
}

template <class B>
void addSetFromMap(B& v, std::size_t i) {
  v[i] = 1;
}

// see also setGrow - requires preallocation
// NOTE: for vector, add = push_back
template <class I, class A, class Val>
void add(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& b, Val i) {
  b.set((typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type)i);
}

template <class I, class A>
void erase(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& b,
           typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i) {
  b.reset(i);
}

template <class I, class A, class C>
void setBits(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& b, C i, C const& end) {
  for (; i != end; ++i) b.set(*i);
}

template <class I, class A, class C>
void setAll(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& b, C i, C const& end) {
  reinit(b, end - i);
  setBits(b, i, end);
}

template <class I, class A, class C>
void setBits(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& b, C const& c) {
  setBits(b, c.begin(), c.end());
}

template <class I, class A, class C>
void copyBitsOut(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& b, C oit) {
  typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i = b.find_first();
  for (; i != b.npos; ++oit, i = b.find_next(i)) *oit = i;
}

template <class I, class A, class C>
void copyBits(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& b, C& c) {
  copyBitsOut(b, adder(c));
}

template <class I, class A, class V>
void copyBits(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& b, std::vector<V>& c) {
  c.resize(b.count());
  typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i = b.find_first();
  copyBitsOut(b, c.begin());
}

template <class Bool>
bool test(std::vector<Bool> const& v, std::size_t i) {
  return v[i];
}

// TODO: couldn't use name 'set' because of improper 'using namespace std' (std::set) - setValue instead
template <class Vec>
void setValue(Vec& v, std::size_t i) {
  v[i] = true;
}

template <class Vec>
void setValue(Vec& v, std::size_t i, bool bit) {
  v[i] = bit;
}

template <class I, class A>
void setValue(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& v,
              typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i, bool bit) {
  v.set(i, bit);
}

template <class I, class A>
void setValue(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& v,
              typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i) {
  v.set(i);
}

template <class Bool>
void reset(std::vector<Bool>& v, std::size_t i) {
  v[i] = 0;
}

static const std::size_t npos = (std::size_t) - 1;

template <class Bool>
std::size_t find_first(std::vector<Bool> const& v) {
  typename std::vector<Bool>::const_iterator p = std::find(v.begin(), v.end(), 1);
  return p == v.end() ? npos : p - v.begin();
}

template <class Bool>
std::size_t find_next(std::vector<Bool> const& v, std::size_t i) {
  typename std::vector<Bool>::const_iterator p = std::find(v.begin() + i + 1, v.end(), 1);
  return p == v.end() ? npos : p - v.begin();
}

template <class I, class A>
bool test(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& v,
          typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i) {
  return v.test(i);
}

template <class I, class A>
void reset(SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>& v,
           typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i) {
  v.reset(i);
}

template <class I, class A>
typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type find_first(
    SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& v) {
  return v.find_first();
}

template <class I, class A>
typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type find_next(
    SDL_BITSET_NAMESPACE::dynamic_bitset<I, A> const& v,
    typename SDL_BITSET_NAMESPACE::dynamic_bitset<I, A>::size_type i) {
  return v.find_next(i);
}

template <class Vec>
void setGrow(Vec& v, std::size_t i) {
  resizeFor(v, i);
  v.set(i);
}

// for vector or bitset
template <class Vec>
bool latchGrow(Vec& v, std::size_t i) {
  if (!resizeFor(v, i)) return !v.test_set(i);
  v.set(i);
  return true;
}

template <class Vec>
void unlatch(Vec& v, std::size_t i) {
  v.reset(i);
}

template <class Vec>
void resetSparse(Vec& v, std::size_t i) {
  if (i < v.size()) reset(v, i);
}

template <class Vec>
bool testSparse(Vec& v, std::size_t i) {
  return i < v.size() ? v.test(i) : false;
}

template <class Vec>
void setSparse(Vec& v, std::size_t i, bool bit) {
  if (bit)
    setGrow(v, i);
  else
    resetSparse(v, i);
}

template <class Vec>
bool testSparse(Vec const& v, std::size_t i) {
  return i < v.size() && v.test(i);
}

// TODO: right-infinite all-false sparse dynamic_bitset (adapt from boost source for same efficiencies in |=


}}

#endif
