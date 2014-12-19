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

    hash functions.
*/

#ifndef HASH_JG_2013_05_31_HPP
#define HASH_JG_2013_05_31_HPP
#pragma once

#include <sdl/IntTypes.hpp>
#include <graehl/shared/hash_murmur.hpp>

namespace sdl { namespace Util {

using graehl::MurmurHash; // std::size_t MurmurHash( const void * key, int len, uint64_t seed = 0)
using graehl::MurmurHash64;

/**
   Xorshift(http://www.jstatsoft.org/v08/i14/paper) and multiplication are two
   main methods that mix the bits inside an integer. Often they are interlaced
   in the mix function as to achieve better non-linearity. It should be noted
   that both Xorshift and multiplication (multiplied by an odd integer) are
   invertible in the residue field of power-of-two.
*/
inline void mixbits(uint64 &h) {
  h ^= h >> 23;
  h *= 0x2127599bf4325c37ULL;
  h ^= h >> 47;
}

/**
   return h well-mixed (every input bit affects every output bit) without
   ruining evenness-of-distribution properties.
*/
inline void mixbitsPerfect(uint64 &h) {
  h = (~h) + (h << 21);
  h = h ^ (h >> 24);
  h = (h + (h << 3)) + (h << 8);
  h = h ^ (h >> 14);
  h = (h + (h << 2)) + (h << 4);
  h = h ^ (h >> 28);
  h += (h << 31);
}


inline uint64 mixedbits(uint64 h) {
  mixbits(h);
  return h;
}

inline uint64 mixedbits(uint32 h) {
  uint64 k = h * 357913941;
  k ^= k << 24;
  k += ~357913941;
  k ^= k >> 31;
  k ^= k << 31;
  return k;
}

inline uint64 mixbitsQuick(uint64 h) {
  return h + (h >> 13);
}

/**
   since this only gives 32 output bits, it's not an overload of mixbitsFull.
*/
inline uint32 mixbits32(uint32 h) {
  h = (h ^ 61) ^ (h >> 16);
  h = h + (h << 3);
  h = h ^ (h >> 4);
  h = h * 0x27d4eb2d;
  return h ^ (h >> 15);
}

/**
   \return p/m where m is the least power of 2 smaller than sizeof(P), suitable
   for hash tables. the lowest bits of pointers to large P are all 0. better
   would be p/sizeof(P) - integer division is slow if e.g. sizeof(P)=12.
*/
template <class P>
inline uint64 pointerSignificantBits(P const* p) {
  return ((uint64)p) >>
      (sizeof(P) < 2 ? 0 :
       sizeof(P) < 4 ? 1 :
       sizeof(P) < 8 ? 2 :
       sizeof(P) < 16 ? 3 :
       sizeof(P) < 32 ? 4 :
       sizeof(P) < 64 ? 5 :
       sizeof(P) < 128 ? 6 :
       sizeof(P) < 256 ? 7 : 8);
  // no harm done if larger: you might have 1 or more trailing 0s, but 8 fewer than before
  // could use boost integer log2.
}

inline uint64 hashPointer(void const* p) {
  return mixbitsQuick((uint64)p);
}

struct HashPointer {
  inline uint64 operator()(void const* p) const { return mixbitsQuick((uint64)p); }
  // for TBB: HashCompare
  static inline bool equal(void const* a, void const* b) { return a==b; }
  static inline uint64 hash(void const* p) { return mixbitsQuick((uint64)p); }
};


template <class P>
inline uint64 hashPointer(P const* p) {
  return mixbitsQuick(pointerSignificantBits(p));
}

inline uint64 rotateRight(uint64 h, int shift) {
  assert(shift && shift<64 && shift>-64);
  return (h >> shift) | (h << (64 - shift));
}

inline uint64 hashFloat(double v) {
  return *reinterpret_cast<uint64 const*>(&v);
}

inline uint64 hashFloat(float v) {
  return *reinterpret_cast<uint32 const*>(&v);
}

inline uint64 mixedbitsMedium(uint64 h)
{
  return h + 0x9e3779b9 + (h << 6) + (h >> 2);
}

inline uint64 combinedHash(uint64 seed, uint64 hashed) {
  return seed ^ (hashed + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

inline uint64 combinedHash(uint64 seed, uint32 hashed) {
  return seed ^ ((uint64)hashed + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

}}

#endif
