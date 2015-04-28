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

    uniform access to "return true if previous value was false, and force new
    value to true" ('latch')
*/

#ifndef SDL_UTIL__LATCH_HPP
#define SDL_UTIL__LATCH_HPP
#pragma once

#include <sdl/Util/Add.hpp>
#include <sdl/Util/Contains.hpp>
#include <sdl/Util/BitSet.hpp> // for contains(bitset, index)

namespace sdl {
namespace Util {

/** \return true iff falseToTrue was false

    \param falseToTrue becomes true after the call
*/
inline bool latch(bool &falseToTrue)
{
  if (falseToTrue) return false;
  falseToTrue=true;
  return true;
}

//TODO: specialization for std::set, unordered_set, etc?
template <class Set, class K>
bool latch(Set &s, K const& k) {
  if (!Util::contains(s, k)) {
    add(s, k);
    return true;
  }
  return false;
}

template <class Set, class I>
void latchAny(bool &anyadded, Set &s, I i, I const& end) {
  for (;i!=end;++i)
    if (latch(s,*i))
      anyadded=true;
}

template <class Set, class I>
void latchAny(bool &anyadded, Set &s, I const& i) {
  latchAny(anyadded, s, i.begin(), i.end());
}

template <class Set>
struct DupCount {
  Set s;
  std::size_t ndup;
  DupCount() : ndup() {}
  std::size_t get() const { return ndup; }
  template <class K>
  bool test(K const& k) const {
    return Util::contains(s, k); //test(s, k);
  }
  template <class K>
  void operator()(K const& k) {
    if (!latch(s, k)) ++ndup;
  }
};

template <class Set>
struct removeDupF {
  Set &s;
  removeDupF(Set &s) : s(s) {}
  removeDupF(removeDupF const& o) : s(o.s) {}
  template <class K>
  bool operator()(K const& k) const {
    return !latch(s, k);
  }
};

template <class Set>
removeDupF<Set> removeDup(Set &s) { return removeDupF<Set>(s); }


}}

#endif
