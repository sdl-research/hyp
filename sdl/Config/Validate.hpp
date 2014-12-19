#ifndef VALIDATE_JG2012725_HPP
#define VALIDATE_JG2012725_HPP
#pragma once

/** \file

    Usage: config("option", &val).validate(Config::boundedRange(0,10)) means only 0...9 are allowed.
*/

#include <sdl/graehl/shared/validate.hpp>

namespace sdl {
namespace Config {

template <class Val>
void call_validate(Val& val) {
  ::adl::adl_validate(val);
}

// for strings or paths: .validate(Config::Exists())
typedef configure::exists Exists;
typedef configure::file_exists FileExists;
typedef configure::dir_exists DirExists;

using configure::one_of;

// for numerics:
template <class I>
configure::bounded_range_validate<I> boundedRange(I const& begin, I const& end,
                                                  std::string const& desc = "value out of bounds") {
  return configure::bounded_range_validate<I>(begin, end, desc);
}

template <class I>
configure::bounded_range_inclusive_validate<I> boundedRangeInclusive(I const& begin, I const& end,
                                                                     std::string const& desc
                                                                     = "value out of bounds") {
  return configure::bounded_range_inclusive_validate<I>(begin, end, desc);
}


}}

#endif
