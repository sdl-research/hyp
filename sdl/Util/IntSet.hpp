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

    hash/equal on unordered vector of unique ints (sparse set of ints - not bit vector)

   we want a set of ints to serve as a key for a lazily expanded determinization
   (subset of states) -> expanded state id hash.

   options for representing a set:

   unordered_set<int> or set<int>

   big-endian particia tree

   32-ary binary trie

   open hash

   //TODO: choose/benchmark an appropriate data structure

   sequence with mutable 0-ed out bit vector or byte vector equality comparison (single-threaded use only)
*/

#ifndef SDL_UTIL__INTSET_HPP
#define SDL_UTIL__INTSET_HPP
#pragma once

#include <vector>
#include <set>
#include <cassert>
#include <sdl/Util/BitSet.hpp>

namespace sdl {
namespace Util {


/* using an arbitrary sequence as an int set representation - we require only no duplicates; arbitrary order
 * is allowed. //TODO: test */

// no duplicates allowed. order independent hash of int stateids (so this
// *intentionally* is a worse hash fn than you're used to). for set<int> you can
// use the default boost::hash, which shoul ddo slightly better.
struct IntSetHash {
  template <class S>
  std::size_t operator()(S const* set) const {
    std::size_t hash = 0;
    for (typename S::const_iterator i = set->begin(), e = set->end(); i != e; ++i) hash ^= *i;
    return hash;
  }
};

/**
   single threaded use only!

   BitArray e.g. vector<bool>, boost::dynamic_bitset<>, BitSet, vector<char>, char **, etc.

   recommend using a similar approach (even the same BitArray) when constructing subsets to ensure no
   duplicates
*/
template <class BitArray>
struct IntSetEqual {
  BitArray* z;  // should be all zeros and allow z[M] where M is the largest int in any set. will be
  // temporarily mutated to 1 and then back to 0 in the course of checking equality
  IntSetEqual(BitArray& z) : z(&z) {}
  IntSetEqual(BitArray* z) : z(z) {}
  IntSetEqual(IntSetEqual const& o) : z(o.z) {}
  // pre: *z==0 post: *z==0
  template <class Sc, class Tc>
  bool operator()(Sc const* S, Tc const* T) const {
    typedef typename Sc::const_iterator Si;
    // pre: z is all 0
    for (Si s = S->begin(), se = S->end(); s != se; ++s) {
      assert(!test(*z, *s));
      setValue(*z, *s);
    }
    // 1': now s \in S <=> z[s]
    // 2: verify that forall t \in T, t \in S, i.e. z[t]
    for (Si t = T->begin(), te = T->end(); t != te; ++t) {
      if (!test(*z, *t)) {
        for (Si s = S->begin(), se = S->end(); s != se; ++s) reset(*z, *s);
        // post: z is all 0 again
        return false;
      }
      reset(*z, *t);
    }
    // 2' now: S contains T, and s \in (S-T) <=> z[s]
    // 3: check that S-T is empty: no s \in S with z[s]
    bool eq = true;
    for (Si s = S->begin(), se = S->end(); s != se; ++s)
      if (test(*z, *s)) {
        reset(*z, *s);
        eq = false;  // must continue to get post
      }
    // post: z is all 0 again
    return eq;
  }
};


}}

#endif
