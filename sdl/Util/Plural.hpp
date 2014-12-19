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
