// Copyright 2014 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef SDL_UTIL_INPUTSIZE_JG2012730_HPP
#define SDL_UTIL_INPUTSIZE_JG2012730_HPP
#pragma once

#include <iostream>
#include <string>
#include <sdl/Util/Plural.hpp>
#include <sdl/Util/ParseSize.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace sdl { namespace Util {

//TODO: make InputSizes which is a vector of InputSize, so you can have both (avg?) path length or # words, and states, arcs, etc. for now, just one metric.

typedef SizeMetric InputSizeAmount;
typedef std::string InputSizeUnits;

namespace Hypergraph {
template <class A> class IHypergraph;
template <class A>
InputSizeAmount sizeAmount(IHypergraph<A> const&);
template <class A>
InputSizeUnits sizeUnits(IHypergraph<A> const&);
}

inline InputSizeUnits sizeUnits(std::string const&)
{
  return "character";
}

inline InputSizeAmount sizeAmount(std::string const&s)
{
  return double(s.size());
}

/** for logging, a simple printable "input size" (scalar + description, e.g. states,
    characters, words, edges), stopping short of a full boost.units.

    note: prints as 20k or 2t instead of 2e5 or 2e12

    also note: a 0 (default) size is treated as "not specified" in that it's not printed.

    you may define double size(Val const&) and std::string sizeUnits(Val const&) to
    simplify initialization of an InputSize object from a Val.
*/


namespace adl {

template <class V>
inline InputSizeUnits callSizeUnits(V const& v)
{
  return sizeUnits(v);
}

template <class V>
inline InputSizeAmount callSizeAmount(V const& v)
{
  return sizeAmount(v);
}

}


struct InputSize
{
  struct InputSizeVisitor : boost::static_visitor<void>
  {
    InputSize *pInputSize;
    InputSizeVisitor(InputSize *pInputSize) : pInputSize(pInputSize) {}
    template <class C> void operator()(C const& c) const
    {
      pInputSize->amount = adl::callSizeAmount(c);
      pInputSize->units = adl::callSizeUnits(c);
    }
    template <class C> void operator()(shared_ptr<C> const& p) const
    {
      pInputSize->amount = adl::callSizeAmount(*p);
      pInputSize->units = adl::callSizeUnits(*p);
    }
  };

  InputSize(InputSizeAmount amount = 0., InputSizeUnits const& units = InputSizeUnits()) : amount(amount), units(units) {}
  InputSize(InputSize const& o) : amount(o.amount), units(o.units) {}
#ifndef _MSC_VER
  template <class Val>
  explicit InputSize(Val const& val) {
    boost::apply_visitor(InputSize::InputSizeVisitor(this), val);
  }
#endif
  InputSizeAmount amount;
  InputSizeUnits units;
  InputSize &operator +=(InputSize const& o) {
    if (empty()) units = o.units;
    assert(o.units==units);
    amount += o.amount;
    return *this;
  }
  InputSize &operator +=(double increment) {
    amount += increment;
    return *this;
  }
  InputSize &operator /=(double N) {
    amount /= N;
    return *this;
  }
#ifdef _MSC_VER
  //quiet a silly warning (4244)
  InputSize &operator +=(std::size_t increment) {
    amount += (double)increment;
    return *this;
  }
  InputSize &operator /=(std::size_t N) {
    amount /= (double)N;
    return *this;
  }
#endif
  template <class N>
  InputSize operator /(N const& n) const {
    InputSize per=*this;
    per /= n;
    return per;
  }
  InputSize &operator /=(InputSize const& o) {
    amount /= o.amount;
    if (o.units==units)
      units="";
    else if (!o.units.empty()) {
      units+="/";
      units += o.units;
    }
    return *this;
  }
  bool empty() const { return !amount; }
  std::string str() const
  {
    return quantity(amount, units);
  }
  template <class Out>
  void print(Out &o) const {
    o << str();
  }
  template <class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr> &o, InputSize const& self)
  { self.print(o); return o; }
};


template <class Variant>
InputSize getVariantInputSize(Variant const& variant)
{
  InputSize size;
  boost::apply_visitor(InputSize::InputSizeVisitor(&size), variant);
  return size;
}

}}

#endif
