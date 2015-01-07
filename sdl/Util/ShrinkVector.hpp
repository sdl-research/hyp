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

    calling vector::clear doesn't usually free any memory. these methods ensure
    you aren't left with too much wasted space (std::swap is necessary in some cases)
*/

#ifndef SDL_SHRINKVECTOR_HPP
#define SDL_SHRINKVECTOR_HPP
#pragma once


#include <vector>
#include <algorithm>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/distance.hpp>
#include <sdl/Util/SmallVector.hpp>

namespace sdl {
namespace Util {

template <class Vec>
void clearVector(Vec &vec) {
  Vec().swap(vec);
}

template <class Vec>
void compactVector(Vec &vec) {
  Vec(vec).swap(vec);
}

template <class Vec>
void resizeCompactVector(Vec &vec, std::size_t newSz) {
  if (!newSz)
    clearVector(vec);
  else if (newSz<vec.size())
    Vec(vec.begin(), vec.begin()+newSz).swap(vec);
  else
    vec.resize(newSz);
}

template <class T, unsigned M, class S>
void clearVector(small_vector<T, M, S> &vec) {
  vec.clear();
}

template <class T, unsigned M, class S>
void compactVector(small_vector<T, M, S> &vec) {
  vec.compact();
}

template <class T, unsigned M, class S>
void resizeCompactVector(small_vector<T, M, S> &vec, std::size_t newSz) {
  S sz = (S)newSz;
  if (sz<=vec.size())
    vec.compact(sz);
  else
    vec.resize(sz);
}


/**
   instead of v=Vec(sz, defaultVal), in-place resize (not leaving too much spare capacity).
*/
template <class Vec>
void reinit(Vec &vec, std::size_t sz, double capacityPerSizkMaxSymbolType = 1.5) {
  if (vec.capacity()<=sz*capacityPerSizkMaxSymbolType) {
    vec.clear();
    vec.resize(sz);
  } else
    Vec(sz).swap(vec);
}

template <class Vec>
void reinit(Vec &vec, std::size_t sz, typename Vec::value_type const& value, double capacityPerSizkMaxSymbolType = 1.5) {
  if (vec.capacity()<=sz*capacityPerSizkMaxSymbolType) {
    vec.clear();
    vec.resize(sz, value);
  } else
    Vec(sz, value).swap(vec);
}

template <class Vec, class Iter>
void reinit(Vec &vec, std::size_t sz, Iter begin, Iter end, double capacityPerSizkMaxSymbolType = 1.5) {
  if (vec.capacity()<=sz*capacityPerSizkMaxSymbolType) {
    vec.clear();
    vec.reserve(sz);
    vec.insert(vec.end(), begin, end);
  } else
    Vec(begin, end).swap(vec);
}

template <class Vec, class Iter>
void reinit(Vec &vec, Iter begin, Iter end, double capacityPerSizkMaxSymbolType = 1.5) {
  using namespace std;
  reinit(vec, distance(begin, end), begin, end, capacityPerSizkMaxSymbolType);
}

template <class Vec, class Range>
void reinitRange(Vec &vec, Range const& range, double capacityPerSizkMaxSymbolType = 1.5) {
  reinit(vec, range.size(), boost::begin(range), boost::end(range), capacityPerSizkMaxSymbolType);
}

template <class Vec, class RemoveIf>
void removeShrink(Vec &vec, RemoveIf const& r) {
  typename Vec::iterator i = vec.begin(), b = i, end = vec.end(), o;
  for (; i != end; ++i) {
    if (r(*i)) {
      o = i;
      for (++i; i != end; ++i) {
        if (!r(*i)) {
          *o = *i;
          ++o;
        }
      }
      vec.resize(o - b);
      break;
    }
  }
}

template <class Vec, class RemoveIf>
void removeShrinkTail(Vec &vec, RemoveIf const& remove) {
  typename Vec::iterator b = vec.begin(), r = vec.end();
  if (r != b)
    for (;;) {
      if (!remove(*--r)) {
        vec.resize(++r - b);
        return;
      }
      if (r == b) {
        vec.clear();
        return;
      }
    }
}

template <class Vec, class RemoveIf>
void removeShrinkCopySameSize(Vec const& in, Vec &vec, RemoveIf const& r) {
  vec.resize((typename Vec::size_type)
             (std::remove_copy_if (in.begin(), in.end(), vec.begin(), r) - vec.begin()));
}

template <class Vec, class RemoveIf>
void removeShrinkCopy(Vec const& in, Vec &vec, RemoveIf const& r) {
  vec.resize(in.size());
  vec.resize((typename Vec::size_type)
             (std::remove_copy_if (in.begin(), in.end(), vec.begin(), r) - vec.begin()));
}


template <class Vec>
void shrinkToNewEnd(Vec &vec, typename Vec::iterator newEnd) {
  assert(newEnd <= vec.end());
  vec.erase(newEnd, vec.end());
}

struct Empty {
  typedef bool result_type;
  template <class C>
  bool operator()(C const& c) const {
    return c.empty();
  }
};

template <class Vec>
void removeEmpty(Vec &vec) {
  removeShrink(vec, Empty());
}


}}

#endif
