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
/** \file

    for when you have x.print(out, context) - out << Util::print(x, context)

   // 3 arg print(state const&,val const&,ostream &) partial application to 1arg print_state_val(ostream). so
   you can do o << print(val, state). calls free function print(ostream, val, state).

   note:

   ADL means you can define a free print in same NS as your type

   but if your type is just a typedef e.g. vector<X>, then you can put it in the LW ns and not e.g.
   sdl::Hypergraph, so that Util::print will find it anyway.

   e.g. // o << print(SymbolId(s),*pVoc)
   // o << print(StateId, hg)<<
 */

#ifndef SDL_UTIL_PRINT_HPP
#define SDL_UTIL_PRINT_HPP
#pragma once

#include <iostream>
#include <sdl/Util/StringBuilder.hpp>

namespace sdl {
namespace Util {

namespace adl_impl {
template <class Val, class State>
void adl_print(std::ostream& out, Val const& val, State const& state) {
  print(out, val, state);
}
}

// instantiate w/ ref types!
template <class Val, class State>
struct Printer {
  Val v;
  State s;
  Printer(Val v, State s) : v(v), s(s) {}
  friend inline std::ostream& operator<<(std::ostream& out, Printer<Val, State> const& x) {
    // must be found by ADL - note: typedefs won't help.
    // that is, if you have a typedef and a shared_ptr, you have to put your print in either ns LW or boost
    print(out, x.v, x.s);
    return out;
  }
  friend inline Util::StringBuilder& operator<<(Util::StringBuilder& out, Printer<Val, State> const& x) {
    print(out, x.v, x.s);
    return out;
  }
};

template <class Val, class State>
Printer<Val const&, State const&> print(Val const& v, State const& s) {
  return Printer<Val const&, State const&>(v, s);
}

template <class Val, class State>
Printer<Val const&, State&> print(Val const& v, State& s) {
  return Printer<Val const&, State&>(v, s);
}


}}

#endif
