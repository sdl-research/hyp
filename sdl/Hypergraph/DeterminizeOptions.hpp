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

    options for Determinize.
*/

#ifndef HYP__DETERMINIZEOPTIONS_HPP
#define HYP__DETERMINIZEOPTIONS_HPP
#pragma once

//could dump DeterminizeFlags and use this instead.

#include <sdl/graehl/shared/bit_arithmetic.hpp>
#include <sdl/Hypergraph/Determinize.hpp>

namespace sdl {
namespace Hypergraph {

struct DeterminizeOptions {
  DeterminizeFlags default_flags;
  bool EPSILON_NORMAL;
  bool RHO_NORMAL;
  bool PHI_NORMAL;
  bool SIGMA_NORMAL;
  static char const* caption() { return "Determinize"; }
  DeterminizeOptions(DeterminizeFlags default_flags = DETERMINIZE_INPUT) : default_flags(default_flags), EPSILON_NORMAL(), RHO_NORMAL(), PHI_NORMAL(), SIGMA_NORMAL() {}
  DeterminizeFlags getFlags() const {
    DeterminizeFlags f = default_flags;
#define HYP__DETOPTSETMASK(n) graehl::set_mask(f, DETERMINIZE_##n, n)
    HYP__DETOPTSETMASK(EPSILON_NORMAL);
    HYP__DETOPTSETMASK(RHO_NORMAL);
    HYP__DETOPTSETMASK(PHI_NORMAL);
    HYP__DETOPTSETMASK(SIGMA_NORMAL);
#undef HYP__DETOPTSETMASK
    return f;
  }
  template <class Config>
  void configure(Config &c)
  {
    c.is("Determinization");
#define HYP__DETOPT(s, nl, nu) c(s)(#nl"-ordinary", &nu##_NORMAL)("treat "#nl" as regular symbols in determinization")
    HYP__DETOPT('E', epsilon, EPSILON);
    HYP__DETOPT('R', rho, RHO);
    HYP__DETOPT('P', phi, PHI);
    HYP__DETOPT('S', sigma, SIGMA);
#undef HYP__DETOPT
  }
};

}}

#endif
