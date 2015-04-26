// Copyright 2014 SDL plc
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

    save typing and reading shared_ptr and make_shared

    (note: make_shared puts the refcount object right next to the constructed
    thing which makes copying it later significantly faster, but might be
    slower in construction depending on compiler optimization). it saves memory
    and may be faster in that it calls the allocator once.

    no-delete shared pointer constructor for passing reference to value object

    note: if you 'using namespace std' in your scope, you may have to change
    'shared_ptr' to 'shared_ptr' later, if we do a C++11 compile and
    don't change it here to 'std::shared_ptr'

    (if you do both using namespace std and boost in your scope, you already
    have this problem)
*/

#ifndef SHAREDPTR_JG20121217_HPP
#define SHAREDPTR_JG20121217_HPP
#pragma once

#include <iostream>
#include <graehl/shared/warning_push.h>
#if HAVE_GCC_4_6
GCC_DIAG_OFF(delete-non-virtual-dtor)
#endif
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <graehl/shared/warning_pop.h>

namespace sdl {

using boost::shared_ptr;
using boost::make_shared;
using boost::static_pointer_cast;
using boost::dynamic_pointer_cast;
using boost::const_pointer_cast;

template <class Ptr>
struct PrintPtr {
  Ptr const& ptr;
  PrintPtr(Ptr const& ptr) : ptr(ptr) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintPtr const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    if (ptr)
      out << *ptr;
    else
      out << "NULL";
  }
};

template <class Ptr>
PrintPtr<Ptr> printPtr(Ptr const& ptr) {
  return PrintPtr<Ptr>(ptr);
}

template <class Val>
void makeNotNull(shared_ptr<Val>& p) {
  if (!p) p.reset(new Val);
}

template <class Val>
shared_ptr<Val> assertNotNull(shared_ptr<Val> const& p) {
  assert(p);
  return p;
}

template <class Val>
shared_ptr<Val> notNull(shared_ptr<Val> const& p) {
  if (!p)
    return make_shared<Val>();
  else
    return p;
}

struct DoNothing {
  template <class V>
  void operator()(V &) const {}
};

template <class Val>
Val* maybeNull(shared_ptr<Val> const& p) {
  return p ? p.get() : NULL;
}

template <class Val>
Val* maybeNull(shared_ptr<Val> const* p) {
  return p ? p->get() : NULL;
}

template <class Val>
shared_ptr<Val> ptrNoDelete(Val& val) {
  return shared_ptr<Val>(&val, DoNothing());
}

template <class Val>
shared_ptr<Val> shared(Val* pval) {
  return shared_ptr<Val>(pval);
}

template <class Val>
shared_ptr<Val> noDelete(Val* pval) {
  return shared_ptr<Val>(pval, DoNothing());
}

template <class Val>
void setNoDelete(shared_ptr<Val>& p, Val& val) {
  p.reset(&val, DoNothing());
}

template <class Val>
void setNoDelete(shared_ptr<Val>& p, Val* val) {
  p.reset(val, DoNothing());
}

template <class Val>
shared_ptr<Val> sharedCopy(Val const& val) {
  return make_shared<Val>(val);
}

/// helper for creating shared_ptr<Base> with nonvirtual Base dtor. same as shared_ptr<Base>(new Impl(val))
template <class Base, class Impl, class Val>
shared_ptr<Base> makeShared(Val const& val) {
  return make_shared<Impl>(val);
}

/**
   useful for map<key, shared_ptr<Val>, e.g.:

   constructed(map[key]).push_back(x);
*/
template <class Val>
Val& constructed(shared_ptr<Val>& ptrMayBeNull) {
  if (ptrMayBeNull) return *ptrMayBeNull;
  Val* r = new Val;
  ptrMayBeNull.reset(r);
  return *r;
}

/**
   useful for map<key, shared_ptr<Val>, e.g.:

   constructed(map[key], i1).push_back(x);
*/
template <class Val, class I1>
Val& constructed(shared_ptr<Val>& ptrMayBeNull, I1 const& i1) {
  if (ptrMayBeNull) return *ptrMayBeNull;
  Val* r = new Val(i1);
  ptrMayBeNull.reset(r);
  return *r;
}

/**
   accepts a reference to value (won't be deleted), pointer to value (will be
   deleted), shared_ptr (copied)
*/
template <class Val>
struct TakeSharedPtr : shared_ptr<Val> {
  typedef shared_ptr<Val> Base;
  TakeSharedPtr(Val* p) : Base(p) {}
  TakeSharedPtr(Base const& p) : Base(p) {}
  TakeSharedPtr(Val& val) : Base(val, DoNothing()) {}
};


}

#endif
