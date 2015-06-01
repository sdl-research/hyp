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

#ifndef TOPK_JG_2015_06_04_HPP
#define TOPK_JG_2015_06_04_HPP
#pragma once

#include <algorithm>
#include <sdl/Util/PrintRange.hpp>

namespace sdl { namespace Util {

template <class V>
struct TopK {
  TopK(std::size_t k)
      : k(k)
  {}
  std::vector<V> topk;
  std::size_t k;
  void add(V const& v) {
    if (k)
      topk.push_back(v);
  }
  void filter(bool sortedAlways = true) {
    std::size_t n = topk.size();
    if (n < k) {
      if (sortedAlways)
        std::sort(topk.begin(), topk.end());
    } else {
      V *b = &topk[0];
      std::partial_sort(b, b + k, b + n);
      topk.resize(k);
    }
    SDL_DEBUG(TopK.filter, Util::arrayPrintable(&topk[0], n));
  }
};


}}

#endif
