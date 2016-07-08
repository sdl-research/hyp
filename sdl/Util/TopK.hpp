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

    keep the k best items in a vector. TODO: optionally pass in visitor for the
    removed items before resize (e.g. delete)
*/

#ifndef TOPK_JG_2015_06_04_HPP
#define TOPK_JG_2015_06_04_HPP
#pragma once

#include <sdl/Util/PrintRange.hpp>
#include <algorithm>
#include <functional>

namespace sdl {
namespace Util {

template <class Vec, class Less = std::less<typename Vec::value_type>>
void topkSorted(Vec& vec, std::size_t k, Less less = Less()) {
  std::size_t n = vec.size();
  typedef typename Vec::value_type V;
  V *begin = vec.data(), *end = begin + n, *mid = begin + k;
  if (mid >= end) {
    std::sort(begin, end);
  } else {
    std::partial_sort(begin, mid, end);
    vec.resize(k);
  }
}

template <class Vec, class Less = std::less<typename Vec::value_type>>
void topkUnsorted(Vec& vec, std::size_t k, Less less = Less()) {
  if (!k)
    vec.clear();
  else {
    typedef typename Vec::value_type V;
    std::size_t n = vec.size();
    if (k < n) {
      V* b = vec.data();
      std::nth_element(b, b + k - 1, b + n);
      vec.resize(k);
    }
  }
}

template <class V, class Less = std::less<typename V::value_type>>
void topk(V& vec, std::size_t k, Less less = Less(), bool sorted = false) {
  if (sorted)
    topkSorted(vec, k, less);
  else
    topkUnsorted(vec, k, less);
}


}}

#endif
