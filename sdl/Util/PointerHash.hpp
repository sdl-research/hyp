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

#ifndef POINTERHASH_GRAEHL_2015_10_29_HPP
#define POINTERHASH_GRAEHL_2015_10_29_HPP
#pragma once

#include <graehl/shared/int_types.hpp>

namespace sdl {
namespace Util {

/// use exact value size info to get more-significant bits
template <class P>
struct PtrDiffHash {
  std::size_t operator()(P const* p) const { return p - (P const*)0; }
  std::size_t operator()(std::intptr_t p) const { return (P const*)p - (P const*)0; }
  std::size_t operator()(std::size_t p) const { return (P const*)p - (P const*)0; }
};

template <std::size_t sz = 3>
struct HashDiscardLowBits {
  static constexpr std::size_t discard = sz;
  std::size_t operator()(std::size_t p) const { return p >> discard; }
  std::size_t operator()(void const* p) const { return operator()((std::size_t)p); }
  std::size_t operator()(std::intptr_t p) const { return operator()((std::size_t)p); }
};


template <class T>
using PointerHash = HashDiscardLowBits<graehl::ceil_log2_const(sizeof(T))>;
;

struct EqualPointer {
  bool operator()(void const* a, void const* b) const { return a == b; }
  bool operator()(std::size_t a, std::size_t b) const { return a == b; }
};


}}

#endif
