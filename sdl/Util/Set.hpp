/** \file

    see also Map.hpp.
*/

#ifndef SDL_UTIL_SET_HPP
#define SDL_UTIL_SET_HPP
#pragma once

#include <algorithm>

namespace sdl {
namespace Util {

/**
   Inserts key into the set.

   \return true if key is new (i.e., was not in the set before)
*/
template <class Set>
bool insertIsNew(Set &set, typename Set::key_type const& key) {
  return set.insert(key).second;
}


}}

#endif
