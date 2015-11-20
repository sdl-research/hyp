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

    pair of StateId-shared between phrase and syntax decoders, and DecoderOutput,
    DecodersShared.

*/

#ifndef HYPERGRAPH__SPAN_JG_2013_06_28_HPP
#define HYPERGRAPH__SPAN_JG_2013_06_28_HPP
#pragma once

#include <cassert>
#include <algorithm>
#include <sdl/Hypergraph/Types.hpp>
#include <boost/operators.hpp>
#include <sdl/Util/Hash.hpp>
#include <utility>
#include <sdl/Util/CastSubstring.hpp>
#include <sdl/Util/StringBuilder.hpp>

namespace sdl {
namespace Hypergraph {

/** span is a pair of state [left, right] in a hypergraph
 */
struct Span : boost::totally_ordered1<Span> {
  void set(std::string const& str, char hyphen = '-') {
    std::string::size_type hyphenPos = str.find(hyphen);
    if (hyphenPos == std::string::npos)
      setNull();
    else {
      std::string::const_iterator s = str.begin(), m = s + hyphenPos;
      Util::castSubstringTo(left, s, m);
      Util::castSubstringTo(right, ++m, str.end());
    }
  }

  void set(char const* s, char const* end, char hyphen = '-') {
    char const* m = std::find(s, end, hyphen);
    if (m == end)
      setNull();
    else {
      Util::castSubstringTo(left, s, m);
      Util::castSubstringTo(right, ++m, end);
    }
  }

  std::string str(char hyphen = '-') const {
    Util::StringBuilder b;
    write(b, hyphen);
    return std::string(b.begin(), b.end());
  }

  void write(Util::StringBuilder& b, char hyphen = '-') const { b(left)(hyphen)(right); }

  void setNull() { left = right = kNoState; }

  StateId left;
  StateId right;
  Span()
      : left()
      , right()
  // TODO: syntax needs to stop relying on default-init being 0, because
  // we'd rather default-init to null span (or uninitialized)
  {}
  Span(StateId l, StateId r) : left(l), right(r) {}
  void shrink(Span const& s) { shrink(s.left, s.right); }
  void shrink(StateId l, StateId r) {
    if (l > left) left = l;
    if (r < right) right = r;
  }
  void grow(Span const& s) { grow(s.left, s.right); }
  void grow(StateId l, StateId r) {
    if (l < left) left = l;
    if (r > right) right = r;
  }
  void growIfDefined(StateId l, StateId r) {
    if (l < left) left = l;
    if (r > right && r != kNoState) right = r;
  }
  void growIfDefined(Span const& s) { growIfDefined(s.left, s.right); }
  void setEmpty(StateId s) { left = right = s; }
  typedef std::pair<StateId, StateId> Pair;
  Pair pair() const {
    Pair r;
    assignTo(r);
    return r;
  }
  void assignTo(Pair& pair) const {
    pair.first = left;
    pair.second = right;
  }
  void assignTo(Span& to) const {
    to.left = left;
    to.right = right;
  }

  bool smaller(Span const& other) const {
    if (other.left == kNoState) return true;
    return size() < other.size();
  }

  StateId size() const { return right - left; }

  friend inline bool operator==(Span const& s1, Span const& s2) {
    return s1.left == s2.left && s1.right == s2.right;
  }
  friend inline bool operator<(Span const& s1, Span const& s2) {
    StateId sz1 = s1.size(), sz2 = s2.size();
    if (sz1 < sz2)
      return true;
    else if (sz1 > sz2)
      return false;
    else
      return s1.left < s2.left;
  }
  friend inline std::ostream& operator<<(std::ostream& os, Span const& span) {
    os << "[" << span.left << "," << span.right << "]";
    return os;
  }

  friend inline std::size_t hash_value(Span const& span) {
    std::size_t h = static_cast<std::size_t>(span.left);
    boost::hash_combine(h, span.right);
    return h;
    // return Util::mixedbits(*reinterpret_cast<uint64 const*>(&span));
  }
  // SDL_MOVE_COPY_DEFAULT(Span)
};

struct NullSpan : Span {
  NullSpan() : Span(kNoState, kNoState) {}
};

NullSpan const kNullSpan;

inline bool isNull(Span const& span) {
  return span.left == kNoState;
}

inline void setNull(Span& span) {
  span.left = kNoState;
}

/// for consistency with setSpan(TokenSpan, left, right)
inline void setSpan(Span& span, StateId left, StateId right) {
  span.left = left;
  span.right = right;
}

inline bool collides(Span const& x, Span const& y) {
  return y.right > x.left && x.right > y.left;
}


}}

#endif
