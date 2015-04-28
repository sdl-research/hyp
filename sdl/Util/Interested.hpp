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

    Build subsets in O(|subset|) time (O(1) per element). Only one subset of interest may exist at a time.

    Assumes dense indices. A hash-based version would work for sparse indices.

    (used in e.g. MutableHypergraph deleteIn/OutArcs).
*/

#ifndef INTERESTED_JG_2014_06_24_HPP
#define INTERESTED_JG_2014_06_24_HPP
#pragma once

#include <sdl/Util/BitSet.hpp>

namespace sdl { namespace Util {

template <class Index>
struct Interested : std::vector<Index> {
  typedef std::vector<Index> Indices;
  Util::BitSet inInterested_;
  void clear() {
    for (typename Indices::const_iterator i = this->begin(), e = this->end(); i != e; ++i)
      inInterested_.reset(*i);
    Indices::clear();
  }
  void reserve(Index n) {
    assertEmpty();
    inInterested_.resize(n);
  }
  void add(Index s) {
    if (!inInterested_.test_set(s))
      this->push_back(s);
  }
  bool addIsNew(Index s) {
    if (inInterested_.test_set(s))
      return false;
    else
      this->push_back(s);
    return true;
  }
  bool addIsNewSparse(Index s) {
    if (!latchGrow(inInterested_, s))
      return false;
    else
      this->push_back(s);
    return true;
  }
  bool containsSparse(Index s) const {
    return testSparse(inInterested_, s);
  }
  bool contains(Index s) const {
    return inInterested_.test(s);
  }
  void assertEmpty() {
    assert(this->empty());
    assert(inInterested_.count() == 0);
  }
};


}}

#endif
