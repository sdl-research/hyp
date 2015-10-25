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

    set of pointer-sized ints: only do things once (cache pointers you already
    did things to).
*/

#ifndef SDL_ONCE_HPP
#define SDL_ONCE_HPP
#pragma once

#include <sdl/Util/PointerSet.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Util {


// wrap pointer-visitors so each pointer gets visited once - mark the pointer seen the first time (via IntSet)

struct Once : public PointerSet {
  template <class V>
  bool first(V const* p) {
    return this->insert((intptr_t)p).second;
  }
};

// NOTE: default char * equality may be wrong in some impls for unordered_set, but std says it should just be
// regular pointer equality. maybe specifying hash avoids this problem only.
template <class V>
struct OnceTypedP : public unordered_set<V const*, PtrDiffHash<V> > {
  // careful: don't pass by val:
  bool first(V const& p) { return first(&p); }
  bool first(V const* p) { return this->insert(p).second; }
};

// visit by pointer. once per pointer. visitor F is copied.
template <class F>
struct VisitOnce : public F {
  VisitOnce(F const& f, PointerSet* p) : F(f), p(p) {}
  PointerSet* p;
  // mutating via pointer p - const helps compiler out?
  template <class V>
  bool first(V const* v) const {
    return p->insert((intptr_t)v).second;
  }
  template <class V>
  void operator()(V* v) const {
    if (first(v)) F::operator()(v);
  }
  template <class V>
  void operator()(V const* v) const {
    if (first(v)) F::operator()(v);
  }
};

template <class F>
VisitOnce<F> makeVisitOnce(F const& f, PointerSet* once) {
  return VisitOnce<F>(f, once);
}

// visitor held by ref. otherwise same as VisitOnce
template <class F>
struct VisitOnceRef {
  F const& f;
  PointerSet* p;
  VisitOnceRef(F const& f, PointerSet* p) : f(f), p(p) {}
  // mutating via pointer p - const helps compiler out?
  template <class V>
  bool first(V const* v) const {
    return p->insert((intptr_t)v).second;
  }
  template <class V>
  void operator()(V* v) const {
    if (first(v)) f(v);
  }
  template <class V>
  void operator()(V const* v) const {
    if (first(v)) f(v);
  }
};

template <class F>
VisitOnceRef<F> makeVisitOnceRef(F const& f, PointerSet* once) {
  return VisitOnceRef<F>(f, once);
}

struct CheckOnce {
  mutable Once once_;
  template <class V>
  void operator()(V const* p) const {
    if (!once_.first(p))
      SDL_THROW_LOG(Once.CheckOnce, ProgrammerMistakeException, "pointer " << (void*)p
                                                                           << " was seen more than once");
  }
};


}}

#endif
