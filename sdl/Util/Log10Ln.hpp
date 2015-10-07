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

 conversion between log10/ln. not recommended since we tune anyway. used to be
 applied to lm-weightN which was disastrous

*/

#ifndef LOG10LN_JG_2015_05_19_HPP
#define LOG10LN_JG_2015_05_19_HPP
#pragma once

#include <sdl/Types.hpp>
#include <graehl/shared/math_constants.hpp>

namespace sdl { namespace Util {

SdlFloat constexpr ln10 = (SdlFloat)M_LN10;
SdlFloat constexpr ln10inv = (SdlFloat)M_LOG10E;

inline SdlFloat logProbForCost(SdlFloat costVal) {
  return costVal * (SdlFloat)M_LN10;
}

inline SdlFloat costForLogProb(SdlFloat costVal) {
  return costVal * (SdlFloat)M_LOG10E;
}


}}

#endif
