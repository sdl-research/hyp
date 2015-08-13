// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    you can provide a void write(ostream, val), otherwise the default ostream <<
    val will be used. you can call it with impl::adl_write(ostream, val)

    also, you may provide functions print(val, state) which create a printer
    object that will call print(o, val, state) (e.g. state is a vocabulary used to
    print symbol ids). then you can o << Util::print(val, state) << "\n"; etc
*/

#ifndef SDL_PRINTRANGE_HPP
#define SDL_PRINTRANGE_HPP
#pragma once

#include <iostream>
#include <map>

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <sdl/Util/Sep.hpp>
#include <sdl/Util/Print.hpp>
#include <sdl/Util/FnReference.hpp>
#include <sdl/Util/VoidIf.hpp>
#include <algorithm>

namespace sdl {
namespace Util {

struct multiline {};
struct multilineNoBrace {};
struct singleline {};
struct commas {};

// pass as state to 3-arg print
struct RangeSep {
  typedef char const* S;
  S space, pre, post;
  bool spaceBefore;
  bool index;
  RangeSep(S space = " ", S pre = "[", S post = "]", bool spaceBefore = false)
      : space(space), pre(pre), post(post), spaceBefore(spaceBefore), index() {}
  RangeSep(multiline) : space("\n "), pre("{"), post("\n}"), spaceBefore(true), index() {}
  RangeSep(multilineNoBrace) : space("\n"), pre(""), post("\n"), spaceBefore(true), index() {}
  RangeSep(singleline) : space(" "), pre("["), post("]"), spaceBefore(), index() {}
  RangeSep(commas) : space(","), pre("<"), post(">"), spaceBefore(), index() {}
};

inline RangeSep multiLine() {
  return RangeSep(multiline());
}

inline RangeSep multiLineNoBrace() {
  return RangeSep(multilineNoBrace());
}

namespace adlimpl {
template <class O, class I>
void adl_write(O& o, I const& i);
}

template <class V, class Enable = void>
struct Write {
  template <class O>
  static void write(O& o, V const& v) {
    o << v;
  }
};

template <class O>
void write(O& o, std::string const& str) {
  o << str;
}

/**
   this default is overridable per ADL.
*/
template <class O, class V>
void write(O& o, V const& val) {
  Write<V>::write(o, val);
}

template <class O, class A, class B>
void writePair(O& o, std::pair<A, B> const& pair, char const* sep) {
  adlimpl::adl_write(o, pair.first);
  o << sep;
  adlimpl::adl_write(o, pair.second);
}

template <class O, class A, class B>
void writePair(O& o, std::pair<A, B> const& pair) {
  writePair(o, pair, "=");
}

template <class O, class A, class B>
void write(O& o, std::pair<A, B> const& pair) {
  writePair(o, pair);
}

namespace adlimpl {
template <class O, class I>
inline void adl_write(O& o, I const& i) {
  write(o, i);
}
}

template <class Out, class Iter>
Out& printRange(Out& out, Iter i, Iter end, RangeSep const& sep = RangeSep()) {
  out << sep.pre;
  unsigned index = 0;
  if (sep.spaceBefore)
    for (; i != end; ++i) {
      out << sep.space;
      if (sep.index)
        out << index << ':';
      adlimpl::adl_write(out, *i);
      ++index;
    }
  else {
    for (; i != end; ++i) {
      if (index) out << sep.space;
      if (sep.index)
        out << index << ':';
      adlimpl::adl_write(out, *i);
      ++index;
    }
  }
  out << sep.post;
  return out;
}

// TODO: switch to C++11 std::begin. can't just using namespace boost or ambig w/ begin in std (clang)
template <class O, class C>
O& printRange(O& o, C const& c, RangeSep const& r = RangeSep()) {
  //  return printRange(o, boost::begin(c), boost::end(c), r);
  return printRange(o, c.begin(), c.end(), r);
}

template <class Iter>
struct PrintableRange : RangeSep {
  Iter i;
  Iter end;
  PrintableRange(Iter i, Iter end, RangeSep const& sep = RangeSep()) : RangeSep(sep), i(i), end(end) {}
  template <class Out>
  friend inline Out& operator<<(Out& out, PrintableRange const& self) {
    self.print(out);
    return out;
  }
  template <class Out>
  void print(Out& out) const {
    printRange(out, i, end, *this);
  }
};

template <class Iter>
PrintableRange<Iter> printableRange(Iter i, Iter end, RangeSep const& sep = RangeSep()) {
  return PrintableRange<Iter>(i, end, sep);
}

template <class Out, class Iter>
Out& printPointerRange(Out& out, Iter i, Iter end, RangeSep const& sep = RangeSep()) {
  out << sep.pre;
  if (sep.spaceBefore)
    for (; i != end; ++i) {
      out << sep.space;
      adlimpl::adl_write(out, **i);
    }
  else {
    Sep s(sep.space);
    for (; i != end; ++i) {
      out << s;
      adlimpl::adl_write(out, **i);
    }
  }
  out << sep.post;
  return out;
}

template <class O, class C>
O& printPointerRange(O& o, C const& c, RangeSep const& r = RangeSep()) {
  //  return printPointerRange(o, boost::begin(c), boost::end(c), r);
  return printPointerRange(o, c.begin(), c.end(), r);
}

template <class Iter>
struct PrintablePointerRange : RangeSep {
  Iter i;
  Iter end;
  PrintablePointerRange(Iter i, Iter end, RangeSep const& sep = RangeSep()) : RangeSep(sep), i(i), end(end) {}
  template <class Out>
  friend inline Out& operator<<(Out& out, PrintablePointerRange const& self) {
    self.print(out);
    return out;
  }
  template <class Out>
  void print(Out& out) const {
    printPointerRange(out, i, end, *this);
  }
};

template <class Iter>
PrintablePointerRange<Iter> printablePointerRange(Iter i, Iter end, RangeSep const& sep = RangeSep()) {
  return PrintablePointerRange<Iter>(i, end, sep);
}

/**
   this will provide write(o, c) for collections, maps, etc.
*/
template <class Range>
struct Write<Range, typename VoidIf<typename Range::const_iterator>::type> {
  template <class O>
  static void write(O& o, Range const& r) {
    printRange(o, r);
  }
};

// uses 3-arg print()
template <class O, class PrintState, class Iter>
O& printRangeState(O& out, PrintState const& q, Iter it, Iter const& end, RangeSep const& rs = RangeSep()) {
  out << rs.pre;
  if (rs.spaceBefore)
    for (; it != end; ++it) {
      out << rs.space;
      print(out, *it, q);
    }
  else {
    Sep sep(rs.space);
    for (; it != end; ++it) {
      out << sep;
      print(out, *it, q);
    }
  }
  out << rs.post;
  return out;
}

template <class O, class PrintState, class I>
O& printRangeState(O& o, PrintState& q, I i, I const& end, RangeSep const& r = RangeSep()) {
  o << r.pre;
  if (r.spaceBefore)
    for (; i != end; ++i) print(o << r.space, *i, q);
  else {
    Sep s(r.space);
    for (; i != end; ++i) print(o << s, *i, q);
  }
  o << r.post;
  return o;
}

template <class O, class PrintState, class C>
O& printRangeState(O& o, PrintState const& q, C const& c, RangeSep const& r = RangeSep()) {
  using namespace boost;
  return printRangeState(o, q, boost::begin(c), boost::end(c), r);
}

template <class O, class PrintState, class C>
O& printRangeState(O& o, PrintState& q, C const& c, RangeSep const& r = RangeSep()) {
  using namespace boost;
  return printRangeState(o, q, boost::begin(c), boost::end(c), r);
}

// pass as state to 3-arg print
template <class X>
struct StateRangeSep {
  RangeSep r;
  X state;
  typedef char const* S;
  explicit StateRangeSep(X state = X(), RangeSep const& r = RangeSep(singleline())) : state(state), r(r) {}
};

template <class X>
StateRangeSep<typename boost::add_reference<typename boost::add_const<X>::type>::type> stateRange(
    X const& x, RangeSep s = RangeSep(singleline())) {
  return StateRangeSep<typename boost::add_reference<typename boost::add_const<X>::type>::type>(x, s);
}

template <class X>
StateRangeSep<typename boost::add_reference<X>::type> stateRange(X& x, RangeSep s = RangeSep(singleline())) {
  return StateRangeSep<typename boost::add_reference<X>::type>(x, s);
}

template <class V>
inline void print(std::ostream& o, V const& v, RangeSep const& r) {
  printRange(o, v, r);
}

template <class V, class X>
inline void print(std::ostream& o, V const& v, StateRangeSep<X> const& r) {
  printRangeState(o, r.state, v, r.r);
}

template <class X>
struct StateRangeRangeSep {
  RangeSep outer;
  StateRangeSep<X> inner;
  StateRangeRangeSep(X state, RangeSep outer = RangeSep(multiline()), RangeSep inner = RangeSep(singleline()))
      : outer(outer), inner(state, inner) {}
};

template <class X>
StateRangeRangeSep<typename boost::add_reference<typename boost::add_const<X>::type>::type> stateRangeRange(
    X const& x, RangeSep outer = RangeSep(multiline()), RangeSep inner = RangeSep(singleline())) {
  return StateRangeRangeSep<typename boost::add_reference<typename boost::add_const<X>::type>::type>(x, outer,
                                                                                                     inner);
}

template <class X>
StateRangeRangeSep<typename boost::add_reference<X>::type> stateRangeRange(
    X& x, RangeSep outer = RangeSep(multiline()), RangeSep inner = RangeSep(singleline())) {
  return StateRangeRangeSep<typename boost::add_reference<X>::type>(x, outer, inner);
}

template <class V, class X>
void print(std::ostream& o, V const& v, StateRangeRangeSep<X> const& rr) {
  printRangeState(o, rr.inner, v, rr.outer);
}

// Wraps range in PrintableRange, which has output operator.
// Usage: std::cerr << Util::makePrintable(myVector) << '\n';
//        std::cerr << Util::makePrintable(myMap)    << '\n';
//        ...
/// \return shorthand for Util::print(range, Util::RangeSep())
template <class T>
Printer<T, RangeSep> makePrintable(T const& range) {
  return Printer<T, RangeSep>(range, RangeSep());
}

template <class T>
Printer<T, RangeSep> makePrintable(T const& range, RangeSep const& sep) {
  return Printer<T, RangeSep>(range, sep);
}

template <class Pair>
struct PrintPair {
  Pair p;
  char const* sep;

  PrintPair(Pair p_, char const* sep_ = ",") : p(p_), sep(sep_) {}

  template <class Out>
  void print(Out& out) const {
    writePair(out, p, sep);
  }

  template <class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& o, PrintPair const& self) {
    self.print(o);
    return o;
  }
};

template <class T1, class T2>
PrintPair<std::pair<T1, T2> > makePrintable(std::pair<T1, T2> p) {
  return PrintPair<std::pair<T1, T2> >(p);
}

template <class Iter>
struct PrintRange : RangeSep {
  Iter begin;
  Iter end;
  PrintRange(Iter begin, Iter end, bool printIndex = false) : begin(begin), end(end) { index = printIndex; }
  PrintRange(Iter begin, Iter end, RangeSep const& sep) : RangeSep(sep), begin(begin), end(end) {}
  template <class Out>
  void print(Out& o) const {
    printRange(o, begin, end, *this);
  }
  template <class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& o, PrintRange const& self) {
    self.print(o);
    return o;
  }
};

template <class Iter>
PrintRange<Iter> makePrintable(Iter begin, Iter end, bool printIndex = false) {
  return PrintRange<Iter>(begin, end, printIndex);
}

template <class Range>
PrintRange<typename Range::const_iterator> rangePrintable(Range const& range, bool printIndex = false) {
  return PrintRange<typename Range::const_iterator>(range.begin(), range.end(), printIndex);
}

template <class Iter>
PrintRange<Iter> arrayPrintable(Iter begin, unsigned n, bool printIndex = false) {
  return PrintRange<Iter>(begin, begin + n, printIndex);
}

template <class Iter>
PrintRange<Iter> printPrefix(Iter begin, Iter end, std::size_t maxPrefixSize) {
  return PrintRange<Iter>(begin, std::min(end, begin + maxPrefixSize));  // std::advance?
}

template <class Container>
PrintRange<typename Container::const_iterator> printPrefix(Container const& range, std::size_t maxPrefixSize) {
  return PrintRange<typename Container::const_iterator>(range.begin(),
                                                        range.begin() + std::min(range.size(), maxPrefixSize));
}

template <class Val>
PrintRange<Val const*> printPrefix(Val const* begin, std::size_t size, std::size_t maxPrefixSize) {
  return PrintRange<Val const*>(begin, begin + std::min(size, maxPrefixSize));
}

std::string const elipsis("...");
unsigned const kColumnsDefault = 132;

struct Elide : std::string {
  Elide(std::string const& in, std::size_t columns = kColumnsDefault) : std::string(in, 0, columns) {
    std::size_t const lenElipsis = elipsis.size();
    std::size_t const len = in.size();
    if (len >= columns && len > lenElipsis)
      std::copy(elipsis.begin(), elipsis.end(), begin() + columns - lenElipsis);
  }
};

template <class Val, class State>
std::string printed(Val const& val, State const& state) {
  std::ostringstream out;
  print(out, val, state);
  return out.str();
}


}}

#endif
