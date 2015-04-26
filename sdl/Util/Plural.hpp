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

    for english logging messages, pluralize line->lines, boss->bosses etc
*/


#ifndef PLURAL_JG201281_HPP
#define PLURAL_JG201281_HPP
#pragma once

#include <graehl/shared/string_to.hpp>

namespace sdl { namespace Util {

inline
std::string plural(std::string const& singular)
{
  std::string::size_type len = singular.size();
  if (len && singular[len-1]=='s')
    return singular+"es";
  return singular+"s";
}

template <class Quantity>
std::string unitForQuantity(Quantity const& n, std::string const& singularUnit)
{
  return n==1 ? singularUnit : plural(singularUnit);
}

template <class Quantity>
std::string quantity(Quantity const& n, std::string const& singularUnit)
{
  if (singularUnit.empty()) return graehl::to_string(n);
  return graehl::to_string(n)+" "+unitForQuantity(n, singularUnit);
}

}}

#endif
