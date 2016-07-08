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

    for when you have x.print(out, context) - out << printer(x, context)

   // 3 arg print(state const&,val const&,ostream &) partial application to 1arg print_state_val(ostream). so
   you can do o << printer(val, state). calls free function print(ostream, val, state).

   note:

   ADL means you can define a free print in same NS as your type

   but if your type is just a typedef e.g. vector<X>, then you can put it in the LW ns and not e.g.
   sdl::Hypergraph, so that Util::print will find it anyway.

   e.g. // o << printer(SymbolId(s),*pVoc)
   // o << printer(StateId, hg)<<
 */

#ifndef SDL_UTIL_PRINT_HPP
#define SDL_UTIL_PRINT_HPP
#pragma once

#include <sdl/Util/StringBuilder.hpp>
#include <graehl/shared/adl_print.hpp>
#include <iostream>

namespace sdl {
using graehl::Printer;
using graehl::printer;


}

#endif
