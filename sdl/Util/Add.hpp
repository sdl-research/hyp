/** \file

    uniform helper fns for adding to collections.
*/

#ifndef SDL_UTIL__ADD_HPP
#define SDL_UTIL__ADD_HPP
#pragma once

#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/distance.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <sdl/Util/SmallVector.hpp>
#include <sdl/Array.hpp>
#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Util {

template <class SharedPtrCollection, class PushBackVal>
inline void pushBackShared(SharedPtrCollection& collection, PushBackVal const& val) {
  if (collection)
    collection->push_back(val);
  else
    collection = make_shared<typename SharedPtrCollection::element_type>(1, val);
}

/**
   std::copy(range1 -> O) then std::copy(range2 -> that) (return ending O
   iter). this is simply cumbersome to type.

   also, i tend to make the following error: begin(r1), end(r2) (at least, i made
   it twice, so now this function exists)
*/
template <class Range1, class Range2, class O>
inline O copyConcatRanges(Range1 const& r1, Range2 const& r2, O const& outIter) {
  return std::copy(boost::begin(r2), boost::end(r2), std::copy(boost::begin(r1), boost::end(r1), outIter));
}

// as above, but resize OutVec before calling copy. OutVec could be a list as well
template <class Range1, class Range2, class OutVec>
inline OutVec& concatRangesToVec(Range1 const& r1, Range2 const& r2, OutVec& outvec) {
  using namespace boost;
  outvec.resize(size(r1) + size(r2));
  std::copy(begin(r2), end(r2), std::copy(begin(r1), end(r1), outvec.begin()));
  return outvec;
}

// as above but insert at end of existing vector
template <class Range1, class Range2, class AppendSequence>
inline AppendSequence& concatRangesAppend(Range1 const& r1, Range2 const& r2, AppendSequence& outvec) {
  using namespace boost;
  outvec.insert(outvec.insert(outvec.end(), begin(r1), end(r1)), begin(r2), end(r2));
  return outvec;
}

template <class C, class Val>
inline void add(C& collection, Val const& v) {
  collection.insert(v);
}

template <class V, class Alloc, class Val>
inline void add(std::vector<V, Alloc>& c, Val const& v) {
  c.push_back(v);
}

template <class V, unsigned MaxInline, class Size, class Val>
inline void add(small_vector<V, MaxInline, Size>& c, Val const& v) {
  c.push_back(v);
}

template <class V, class Alloc, class Val>
inline void add(std::deque<V, Alloc>& c, Val const& v) {
  c.push_back(v);
}

template <class V, class Alloc, class Val>
inline void add(std::list<V, Alloc>& c, Val const& v) {
  c.push_back(v);
}

template <class Vec>
inline typename Vec::value_type const& lastAdded(Vec const& vec) {
  return vec.back();
}

template <class Vec>
inline typename Vec::value_type const& removeLastAdded(Vec const& vec) {
  return vec.pop_back();
}

/**
   an iterator and a function object. *this=val or (*this)(val) call add(a, val);
*/
template <class AddTo>
struct Adder {
  typedef std::output_iterator_tag iterator_category;
  typedef typename AddTo::value_type value_type;
  typedef void difference_type;
  typedef value_type const* pointer_type;
  typedef value_type const& reference_type;
  AddTo& addTo;
  Adder(AddTo& addTo) : addTo(addTo) {}
  typedef Adder Self;

  Self const& operator*() const { return *this; }
  Self const& operator++() const { return *this; }
  Self const& operator++(int) const { return *this; }
  typedef typename AddTo::value_type argument_type;
  template <class Val>
  void operator()(Val const& val) const {
    add(addTo, val);
  }
  template <class Val>
  void operator=(Val const& val) const {
    add(addTo, val);
  }
};

template <class AddTo>
Adder<AddTo> adder(AddTo& a) {
  return Adder<AddTo>(a);
}

template <class AddTo>
Adder<AddTo> adder(AddTo* a) {
  return Adder<AddTo>(&a);
}

/**
   Adder but add .first (e.g. key_type of a map)
*/
template <class AddTo>
struct FirstAdder : Adder<AddTo> {
  typedef Adder<AddTo> Base;
  FirstAdder(AddTo& addTo) : Base(addTo) {}
  template <class Val>
  void operator()(Val const& val) const {
    Base::operator()(val.first);
  }
  template <class Val>
  void operator=(Val const& val) const {
    Base::operator()(val.first);
  }
};

template <class AddTo>
FirstAdder<AddTo> firstAdder(AddTo& a) {
  return FirstAdder<AddTo>(a);
}

template <class AddTo>
FirstAdder<AddTo> firstAdder(AddTo* a) {
  return FirstAdder<AddTo>(&a);
}

/**
   Adder but add .second (e.g. mapped_type of a map)
*/
template <class AddTo>
struct SecondAdder : Adder<AddTo> {
  typedef Adder<AddTo> Base;
  SecondAdder(AddTo& addTo) : Base(addTo) {}
  template <class Val>
  void operator()(Val const& val) const {
    Base::operator()(val.second);
  }
  template <class Val>
  void operator=(Val const& val) const {
    Base::operator()(val.second);
  }
};

template <class AddTo>
SecondAdder<AddTo> secondAdder(AddTo& a) {
  return SecondAdder<AddTo>(a);
}

template <class AddTo>
SecondAdder<AddTo> secondAdder(AddTo* a) {
  return SecondAdder<AddTo>(&a);
}

template <class Collection, class ForwardIter>
inline void addAll(Collection& collection, ForwardIter const& begin, ForwardIter const& end) {
  collection.insert(begin, end);
}

template <class Value, class Cmp, class Alloc, class ForwardIter>
inline void addAll(std::set<Value, Cmp, Alloc>& collection, ForwardIter const& begin, ForwardIter const& end) {
#if _MSC_VER
  for (ForwardIter i = begin; i != end; ++i) collection.insert(*i);
#else
  collection.insert(begin, end);
#endif
}

template <class Collection, class ForwardIter>
inline void append(Collection& collection, ForwardIter const& begin, ForwardIter const& end) {
  collection.insert(collection.end(), begin, end);
}

template <class V, unsigned MaxInline, class Size, class ForwardIter>
inline void addAll(small_vector<V, MaxInline, Size>& sequence, ForwardIter const& begin,
                   ForwardIter const& end) {
  sequence.append(begin, end);
}

template <class V, unsigned MaxInline, class Size, class ForwardIter>
inline void append(small_vector<V, MaxInline, Size>& sequence, ForwardIter const& begin,
                   ForwardIter const& end) {
  sequence.append(begin, end);
}

template <class V, unsigned MaxInline, class Size, class Range>
inline void append(small_vector<V, MaxInline, Size>& sequence, Range const& r) {
  sequence.append(r);
}

template <class V, unsigned MaxInline, class Size>
inline void addFront(small_vector<V, MaxInline, Size>& sequence, V const& v) {
  sequence.push_front(v);
}

template <class Sequence, class Size>
inline void eraseSubstr(Sequence& sequence, typename Sequence::iterator const& begin, Size len) {
  sequence.erase(begin, begin + len);
}

template <class Sequence, class Size>
inline void eraseFront(Sequence& sequence, Size erasePrefixLen) {
  typename Sequence::iterator begin = sequence.begin();
  eraseSubstr(sequence, sequence.begin(), erasePrefixLen);
}

template <class Sequence, class Size>
inline void shortenToSuffix(Sequence& sequence, Size keepSuffixlen) {
  Size len = sequence.size();
  if (len > keepSuffixlen) eraseFront(sequence, len - keepSuffixlen);
}

/**
   requires iterators don't alias into sequence (won't be invalidated by resizing/clearing it)
*/
template <class Sequence, class Iterator>
inline void assignNonAliasingRange(Sequence& sequence, Iterator i, Iterator end) {
  sequence.clear();
  sequence.resize(end - i);
  std::copy(i, end, sequence.begin());
}

template <class V, unsigned MaxInline, class Size>
inline void assignRangeNonAliasing(small_vector<V, MaxInline, Size>& sequence, V const* i, V const* end) {
  sequence.assign(i, end);
}

/**
   C++11 will allow this optimization with basic_string, too.
*/
template <class V, unsigned MaxInline, class Size>
inline void assignRangeNonAliasing(small_vector<V, MaxInline, Size>& sequence,
                                   typename std::vector<V>::const_iterator i,
                                   typename std::vector<V>::const_iterator end) {
#if SDL_WIN32_SECURE_SCL_WORKAROUND
  if (i != end) sequence.assign(&*i, 1 + &*(end - 1));
#else
  sequence.assign(&*i, &*end);
#endif
}

template <class Sequence, class Sequence2>
inline void assignSequenceNonAliasing(Sequence& sequence, Sequence2 const& newSequence) {
  assignRangeNonAliasing(sequence, newSequence.begin(), newSequence.end());
}

/**
   sequence = newSequence (of different types, same value) - potentially faster than std::copy
*/
template <class Sequence, class Sequence2>
inline void assignSequence(Sequence& sequence, Sequence2 const& newSequence) {
  if ((void*)&sequence == (void*)&newSequence) return;
  assignRangeNonAliasing(sequence, newSequence.begin(), newSequence.end());
}

/**
   optimization of above for same type.
*/
template <class Sequence>
inline void assignSequence(Sequence& sequence, Sequence const& newSequence) {
  // waste of time to check for identity equality since user's = is practically required to already
  sequence = newSequence;
}

template <class Collection, class Set>
inline void append(Collection& c, Set const& set) {
  append(c, set.begin(), set.end());
}

template <class Collection, class RandomAccessIter>
inline void appendLen(Collection& c, RandomAccessIter begin, std::size_t len) {
  append(c, begin, begin + len);
}

template <class Collection, class Val>
inline void addFront(Collection& c, Val const& v) {
  c.insert(c.begin(), v);
}

template <class Collection>
inline void popFront(Collection& c) {
  c.erase(c.begin());
}

template <class V, class A, class V2>
inline void addFront(std::deque<V, A>& c, V2 const& v) {
  c.push_front(v);
}

template <class V, class A, class V2>
inline void addFront(std::list<V, A>& c, V2 const& v) {
  c.push_front(v);
}

template <class V, class A, class V2>
inline void addFront(std::vector<V, A>& c, V2 const& v) {
  c.insert(c.begin(), v);
}


template <class Collection, class ForwardIter>
inline void prepend(Collection& c, ForwardIter const& begin, ForwardIter const& end) {
  c.insert(c.begin(), begin, end);
}

template <class Collection, class Set>
inline void prepend(Collection& c, Set const& set) {
  c.insert(c.begin(), set.begin(), set.end());
}

template <class Cont, class Iter>
inline void prependPointers(Cont& cont, Iter begin, Iter end) {
  // bidirectional iter only
  for (;;) {
    if (end == begin) return;
    --end;
    addFront(cont, get_pointer(*end));
  }
}

template <class Cont, class Range>
inline void prependPointers(Cont& cont, Range const& r) {
  prependPointers(cont, r.begin(), r.end());
}


template <class V, class Alloc, class ForwardIter>
inline void addAll(std::vector<V, Alloc>& c, ForwardIter const& begin, ForwardIter const& end) {
  Util::append(c, begin, end);
}

template <class C, class Set>
inline void addAll(C& c, Set const& s) {
  addAll(c, s.begin(), s.end());
}

template <class V, class Alloc, class Set>
inline void addAll(std::vector<V, Alloc>& c, Set const& s) {
  append(c, s);
}

struct first {
  template <class P>
  typename P::first_type const& operator()(P const& p) const {
    return p.first;
  }
};

struct second {
  template <class P>
  typename P::second_type const& operator()(P const& p) const {
    return p.second;
  }
};

template <class Map, class O>
void keysAdd(Map const& m, O& o) {
  std::copy(m.begin(), m.end(), firstAdder(o));
}

template <class Map, class O>
void valsAdd(Map const& m, O& o) {
  std::copy(m.begin(), m.end(), secondAdder(o));
}


/**
   v.resize(sz) if not already at least that large
*/
template <class Vector, class Index>
void increaseSize(Vector& v, Index sz) {
  if (sz > v.size()) v.resize(sz);
}

/**
   v.resize(sz, zero) if not already at least that large
*/
template <class Vector, class Index, class Zero>
void increaseSize(Vector& v, Index sz, Zero const& zero) {
  if (sz > v.size()) v.resize(sz, zero);
}

// doesn't work for vector<bool>
template <class Vector, class Index>
typename Vector::reference atExpand(Vector& v, Index i) {
  if (i >= v.size()) v.resize(i + 1);
  return v[i];
}

/**
   return reference to v[i], after v.resize(i+1, zero) if needed.
*/
template <class Vector, class Index, class Zero>
typename Vector::reference atExpand(Vector& v, Index i, Zero const& zero) {
  if (i >= v.size()) v.resize(i + 1, zero);
  return v[i];
}

/**
   as with above atExpand, compensating for boost::ptr_vector funny syntax
   requiring you give a pointer argument to resize.
*/
template <class Val, class Index, class Zero>
Val& atExpandPtr(boost::ptr_vector<Val>& ptrVec, Index i, Zero const& zeroValue) {
  if (i < ptrVec.size()) return ptrVec[i];
  ptrVec.resize(i + 1, const_cast<Zero*>(&zeroValue));
  return ptrVec[i];
}

template <class Val, class Index, class Zero>
Val& atExpandPtr(boost::ptr_vector<Val>& ptrVec, Index i) {
  if (i < ptrVec.size()) return ptrVec[i];
  ptrVec.resize(i + 1);
  return ptrVec[i];
}

/// Val should be nullable<T> - then returns T&
template <class Nullable, class Index, class Zero>
typename Nullable::type& atExpandPtrNullable(boost::ptr_vector<Nullable>& ptrVec, Index i) {
  if (i < ptrVec.size()) return ptrVec[i];
  ptrVec.resize(i + 1, 0);
  typedef typename Nullable::type Val;
  Val* r = new Val();
  ptrVec.replace(i, r);
  return r;
}

/**
   FIFO stack w/ vector or deque (push = append or add)
*/
template <class Collection>
typename Collection::value_type const& top(Collection const& c) {
  return c.back();
}

template <class Collection>
typename Collection::value_type& top(Collection& c) {
  return c.back();
}

template <class Collection>
void pop(Collection& c) {
  c.pop_back();
}

/**
   in case you have an expensive copy constructor, you can retrieve top via swap
   rather than copy ctor or assignment. you'd use this since pop invalidates the
   top value
*/
template <class Collection>
void popSwap(Collection& c, typename Collection::value_type& saveTop) {
  saveTop.swap(top(c));
  pop(c);
}

template <class Collection>
inline void pushBackDefault(Collection& collection) {
  collection.push_back(typename Collection::value_type());
}

/// avoids some boost::assign C++11 ambiguity problems
template <class Map>
struct MapAdd {
  Map& to;

  MapAdd(Map& to) : to(to) {}

  template <class Key, class Val>
  MapAdd const& operator()(Key const& key, Val const& val) const {
    to[key] = val;
    return *this;
  }

  template <class Pair>
  MapAdd const& operator()(Pair const& pair) const {
    to[pair.first] = pair.second;
    return *this;
  }
  template <class Val>
  friend inline MapAdd const& operator<<(MapAdd const& out, Val const& val) {
    return out(val);
  }
};

template <class Seq>
struct SeqAdd {
  Seq& to;

  SeqAdd(Seq& to) : to(to) {}

  template <class Val>
  SeqAdd const& operator()(Val const& val) const {
    add(to, val);
    return *this;
  }
  template <class Val>
  friend inline SeqAdd const& operator<<(SeqAdd const& out, Val const& val) {
    return out(val);
  }
};

template <class Map>
MapAdd<Map> mapAdd(Map& to) {
  return MapAdd<Map>(to);
}

template <class Seq>
SeqAdd<Seq> seqAdd(Seq& to) {
  return SeqAdd<Seq>(to);
}


}}

#endif
