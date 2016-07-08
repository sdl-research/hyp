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

 .
*/

#ifndef ICUISCASE_GRAEHL_2016_06_20_HPP
#define ICUISCASE_GRAEHL_2016_06_20_HPP
#pragma once

#include <sdl/Util/Enum.hpp>
#include <sdl/Util/IcuCase.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Util {

SDL_ENUM(IcuIsCase, 5, (Upper, Lower, Mixed, No, Title));

inline IcuIsCase icuIsCase(std::string const& str, std::string const& lower, std::string const& upper, std::string const& title) {
  if (str == lower) return str == upper ? IcuIsCase::kNo : IcuIsCase::kLower;
  if (str == upper) return IcuIsCase::kUpper;
  return str == title ? IcuIsCase::kTitle : IcuIsCase::kMixed;
}

inline IcuIsCase icuIsCase(std::string const& str, std::string const& lower, std::string const& upper, Util::IcuCaseSome const& icuCaser) {
  if (str == lower) return str == upper ? IcuIsCase::kNo : IcuIsCase::kLower;
  if (str == upper) return IcuIsCase::kUpper;
  return str == icuCaser.recased(str, Util::kTitleCase) ? IcuIsCase::kTitle : IcuIsCase::kMixed;
}

inline IcuIsCase icuIsCase(std::string const& str, Util::IcuCaseSome const& icuCaser) {
  return icuIsCase(str, icuCaser.recased(str, Util::kFullLowerCase), icuCaser.recased(str, Util::kFullUpperCase), icuCaser);
}


}}

#endif
