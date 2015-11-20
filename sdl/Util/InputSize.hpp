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
#ifndef SDL_UTIL_INPUTSIZE_JG2012730_HPP
#define SDL_UTIL_INPUTSIZE_JG2012730_HPP
#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <sdl/Util/Plural.hpp>
#include <sdl/Util/ParseSize.hpp>
#include <graehl/shared/size_mega.hpp>

namespace sdl {
typedef graehl::size_metric InputSizeAmount;
}

namespace adl_default {

inline std::string sizeUnits(std::string const&) {
  return "character";
}

inline sdl::InputSizeAmount sizeAmount(std::string const& s) {
  return double(s.size());
}

template <class Val>
std::string sizeUnits(std::shared_ptr<Val> const& pval) {
  return sizeUnits(*pval);
}

template <class Val>
sdl::InputSizeAmount sizeAmount(std::shared_ptr<Val> const& pval) {
  return sizeAmount(*pval);
}

template <class Val>
std::string sizeUnits(Val const* pval) {
  return sizeUnits(*pval);
}

template <class Val>
sdl::InputSizeAmount sizeAmount(Val const* pval) {
  return sizeAmount(*pval);
}
}

namespace adl {

template <class V>
inline std::string adl_sizeUnits(V const& v) {
  using namespace adl_default;
  return sizeUnits(v);
}

template <class V>
inline sdl::InputSizeAmount adl_sizeAmount(V const& v) {
  using namespace adl_default;
  return sizeAmount(v);
}
}

namespace sdl {

namespace Hypergraph {
template <class A>
class IHypergraph;
template <class A>
InputSizeAmount sizeAmount(IHypergraph<A> const&);
template <class A>
std::string sizeUnits(IHypergraph<A> const&);
}

namespace Util {

// TODO: make InputSizes which is a vector of InputSize, so you can have both (avg?) path length or # words,
// and states, arcs, etc. for now, just one metric.

/** for logging, a simple printable "input size" (scalar + description, e.g. states,
    characters, words, edges), stopping short of a full boost.units.

    note: prints as 20k or 2t instead of 2e5 or 2e12

    also note: a 0 (default) size is treated as "not specified" in that it's not printed.

    you may define double size(Val const&) and std::string sizeUnits(Val const&) to
    simplify initialization of an InputSize object from a Val.
*/
struct InputSize {
  InputSize(InputSizeAmount amount = 0., std::string const& units = std::string())
      : amount(amount), units(units) {}
  InputSize(InputSize const& o) : amount(o.amount), units(o.units) {}
  InputSizeAmount amount;
  std::string units;
  InputSize& operator+=(InputSize const& o) {
    if (empty()) units = o.units;
    assert(o.units == units);
    amount += o.amount;
    return *this;
  }
  InputSize& operator+=(double increment) {
    amount += increment;
    return *this;
  }
  InputSize& operator/=(double N) {
    amount /= N;
    return *this;
  }
  template <class N>
  InputSize operator/(N const& n) const {
    InputSize per = *this;
    per /= n;
    return per;
  }
  InputSize& operator/=(InputSize const& o) {
    amount /= o.amount;
    if (o.units == units)
      units = "";
    else if (!o.units.empty()) {
      units += "/";
      units += o.units;
    }
    return *this;
  }
  bool empty() const { return !amount; }
  std::string str() const { return quantity(amount, units); }
  template <class Out>
  void print(Out& o) const {
    o << str();
  }
  template <class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& o, InputSize const& self) {
    self.print(o);
    return o;
  }
};
}


}

#endif
