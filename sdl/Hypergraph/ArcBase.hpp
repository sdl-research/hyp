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

    hyperedge (ArcBase) connectivity (but no semiring - see Arc.hpp ArcTpl for that).
*/

#ifndef ARCBASE_GRAEHL_2015_08_13_HPP
#define ARCBASE_GRAEHL_2015_08_13_HPP
#pragma once

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/ObjectCount.hpp>
#include <boost/functional/hash.hpp>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>

namespace sdl { namespace Hypergraph {

struct PresizeTails {};

//SDL_OBJECT_TRACK_BASE(ArcBase)
struct ArcBase {
  StateId head_;
  StateIdContainer tails_;
  ArcBase() {}
  /**
     for making a copy with a tail id substitution. this could be a free fn but
     this is more efficient (no extra copying).
  */
  ArcBase(StateIdContainer const& tails, StateId fromTail, StateId replacementTail) : tails_(tails.size())
  {
    std::replace_copy(tails.begin(), tails.end(), tails_.begin(), fromTail, replacementTail);
  }
  explicit ArcBase(StateId srcState) : tails_(1, srcState) {}
  explicit ArcBase(StateIdContainer const& tails) : tails_(tails) {}
  ArcBase(PresizeTails, StateId ntails) : tails_(ntails) {}
  ArcBase(StateId srcState, StateId lexState) : tails_(2) {
    tails_[0] = srcState;
    tails_[1] = lexState;
  }
#if __cplusplus >= 201103L
  /// move, copy. in case object tracking interferes w/ defaults
  ArcBase(ArcBase && o) = default;
  ArcBase& operator=(ArcBase && o) = default;
  ArcBase(ArcBase const& o) = default;
  ArcBase& operator=(ArcBase const& o) = default;
#endif
  friend inline std::ostream& operator<<(std::ostream &out, ArcBase const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream &out) const {
    out << head_ << " <-";
    for(StateIdContainer::const_iterator i = tails_.begin(), e = tails_.end(); i != e; ++i)
      out << ' ' << *i;
  }
};


// reference to arc's tails valid only as long as arc isn't modified. equal/hash of an Arc ignoring weight
struct ArcConnections {
  StateId const* tails;
  StateId head;
  TailId nTailBytes;
  StateId const* end() const { return (StateId const*)((char const*)tails + nTailBytes); }

  /**
     for unordered_map.
  */
  bool operator==(ArcConnections const& o) const {
    return head == o.head && nTailBytes == o.nTailBytes && !std::memcmp(tails, o.tails, nTailBytes);
  }
  friend inline std::size_t hash_value(ArcConnections const& c) {
    std::size_t h = c.head;
    boost::hash_range(h, c.tails, c.end());
    return h;
  }

  /**
     for std::map or sorting key.
  */
  bool operator<(ArcConnections const& o) const {
    if (head < o.head) return true;
    if (o.head < head) return false;
    if (nTailBytes < o.nTailBytes) return true;
    if (o.nTailBytes < nTailBytes) return false;
    return std::memcmp(tails, o.tails, nTailBytes) < 0;
  }

  void set(ArcBase const& a) {
    head = a.head_;
    StateIdContainer const& t = a.tails_;
    tails = arrayBegin(t);
    nTailBytes = (TailId)(sizeof(StateId) * t.size());
  }

  ArcConnections() {}

  explicit ArcConnections(ArcBase const* a) {
    set(*a);
  }
};

}}

#endif
