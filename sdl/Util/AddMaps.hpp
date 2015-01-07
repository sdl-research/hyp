// Copyright 2014 SDL plc
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

    add or subtract a scalar from map values.

    AddFct examples are in LogMath.hpp.

*/

#ifndef SDL_UTIL_ADD_MAPS_H_
#define SDL_UTIL_ADD_MAPS_H_
#pragma once

#include <cassert>
#include <utility>
#include <sdl/Util/Map.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Util {

/**
   for F plus with F::result_type plus(F::result_type, F::result_type); with an
   optional (required when Commutative is false) F::zero(), to include a
   zeroPlus(F::result_type) and (plusBy) void F(F::result_type, F::result_type &).
*/
template <class AddFct, bool Commutative = AddFct::kIsCommutative>
struct PlusByFromAddFct {
  typedef typename AddFct::result_type result_type;
  enum { kIsCommutative = Commutative };
  inline result_type zeroPlus(result_type const& b) const {
    return AddFct::operator()(AddFct::zero(), b);
  }
  inline void operator()(result_type const& b, result_type &a) const {
    a = AddFct::operator()(a, b);
  }
  inline result_type operator()(result_type const& a, result_type const& b) const {
    return AddFct::operator()(a, b);
  }
};

template <class AddFct>
struct PlusByFromAddFct<AddFct, true> {
  typedef typename AddFct::result_type result_type;
  enum { kIsCommutative = true };
  inline result_type zeroPlus(result_type const& b) const {
    return b;
  }
  inline void operator()(result_type const& b, result_type &a) const {
    a = AddFct::operator()(a, b);
  }
  inline result_type operator()(result_type const& a, result_type const& b) const {
    return AddFct::operator()(a, b);
  }
};


template <class Map, class PlusBy>
inline void plusByMapElement(PlusBy const& plusBy, Map &m, typename Map::value_type const& kv) {
  std::pair<typename Map::iterator, bool> iNew = m.insert(kv);
  if (!iNew.second)
    plusBy(kv.second, iNew.first->second);
  else if (!PlusBy::kIsCommutative)
    iNew.first->second = plusBy.zeroPlus(kv.second);
}

/**
   add to existing map (sorted or unordered) *pmap the (key, val) pairs in [i, iEnd) with plusBy:

   map[key]=plusBy.zeroPlus(v) if key not in map (optimization: if PlusBy::kIsCommutative, this is just v

   else (key was in map already)

   plusBy(v, map[key]) (which is like map[key]=plusBy(map[key], v) - that is, v is
   added/subtracted/whatever from the existing value map[key]
*/
template<typename InputIterator,
         typename PlusBy,
         typename OutputMap>
void plusByMap(InputIterator i, InputIterator iEnd, PlusBy plusBy, OutputMap *pmap)
{
  OutputMap &map=*pmap;
  for (; i != iEnd; ++i)
    plusByMapElement(plusBy, map, *i);
}

//TODO: does the sorted-map-specific version even outperform the general map?
//this is considerably more effort to understand
template<typename InputIterator,
         typename PlusBy,
         typename OutputMap>
void plusByMapSorted(InputIterator i, InputIterator iEnd, PlusBy plusBy, OutputMap *pmap,
                     typename OutputMap::iterator o, typename OutputMap::iterator oEnd)
{
  OutputMap &map=*pmap;
  typedef typename OutputMap::key_type Key;
  typedef typename OutputMap::value_type Pair;
  typename OutputMap::iterator hint = o;  // hint is first key in o that is not < i (before where you'd normally insert).
  if (o!=oEnd) {
    // for speeding map::insert. supposed to be element *before*, but initially, there's no "before" begin()
    for (;;) {
      Key const& iKey = i->first; // constant for the coming loop:
      if (o->first < iKey) { // thing to be added has key >= than o. so advance o until o is == or >
        o = map.lower_bound(iKey); if (o==oEnd) break;
        hint = o;
      }
      //SDL_TRACE(AddMaps, "i: " << iKey << "=" << i->second << " add to or insert before existing o: " << o->first << "=" << o->second);
      //now o points at the first element not less than i
      Key const& oKey = o->first;
      if (iKey == oKey) { // existing. o advances since next i will be past it
        plusBy(i->second, o->second);
        hint = o;
        ++o; if (o==oEnd) break;
        ++i; if (i==iEnd) return;
      } else
        // since not ==, now we have i->first < o->first. so add all the 0+i until this is no longer true.
        for (;;) {
          Key const& iKey = i->first;
          if (oKey<iKey) break;
          if (PlusBy::kIsCommutative)
            hint = map.insert(hint, *i);
          else
            hint = map.insert(hint, Pair(iKey, plusBy.zeroPlus(i->second)));
          ++i; if (i==iEnd) return;
          // o doesn't advance; we're inserting all the things w/ keys that go before o
        }
    }
  }
  assert (o==oEnd); // o has no more elements to add with i's - just add zeroPlus([i, iend))
  if (PlusBy::kIsCommutative)
    map.insert(i, iEnd);
  else
    for (; i!=iEnd; ++i)
      hint = map.insert(hint, Pair(i->first, plusBy.zeroPlus(i->second)));

}

/**
   plusByMap but for optimized for sorted map (not multimap) and input range (with same sort) (e.g. std::map) only.

*/
template<typename InputIterator,
         typename PlusBy,
         typename OutputMap>
void plusByMapSorted(InputIterator i, InputIterator iEnd, PlusBy plusBy, OutputMap *pmap)
{
  return plusByMapSorted(i, iEnd, plusBy, pmap, pmap->begin(), pmap->end());
}


/**
   automatically choose the sorted optimization if maps are sorted. assumes
   sorted maps have same sort.
*/
template<typename InputMap,
         typename PlusBy,
         typename OutputMap>
typename SortedMap<InputMap>::type //void
plusByMap(InputMap const& inMap, PlusBy const& plusBy, OutputMap *pmap)
{
  plusByMapSorted(inMap.begin(), inMap.end(), plusBy, pmap);
}

template<typename InputMap,
         typename PlusBy,
         typename OutputMap>
typename HashedMap<InputMap>::type //void
plusByMap(InputMap const& inMap, PlusBy const& plusBy, OutputMap *pmap)
{
  plusByMap(inMap.begin(), inMap.end(), plusBy, pmap);
}

/**
   for sorted maps only.

   result[k] = add(map1[k], map2[k]), provided add(z,0) for all z. if map2[k] but no map1[k] exists, uses add(map1[k]). note that add may thus be a subtract with sub(z)=-z (unary minus) and sub(a, b)=a-b. actually, for optimization, AddFct::kIsCommutative is tested (should be true if we can just use map2[k] instead of add(map2[k]))

   \param begin1 The begin of first map.

   \param end1 The end of first map.

   \param begin2 The begin of second map.

   \param end2 The end of second map.

   \param Resulting map
*/
template<typename InputIterator1, typename InputIterator2,
         typename PlusBy,
         typename OutputMap>
void addMaps(InputIterator1 begin1, InputIterator1 end1,
             InputIterator2 begin2, InputIterator2 end2,
             PlusBy plusBy,
             OutputMap* result) {
  assert(result->empty());
  result->insert(begin1, end1);
  plusByMap(begin2, end2, plusBy, result);
}

template<typename Map1, typename Map2,
         typename PlusBy,
         typename OutputMap>
void addMaps(Map1 const& map1, Map2 const& map2,
             PlusBy const& plusBy,
             OutputMap* result)
{
  addMaps(map1.begin(), map1.end(),
          map2.begin(), map2.end(),
          plusBy,
          result);
}

}}

#endif
