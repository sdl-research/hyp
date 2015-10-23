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

 \author mdreyer (originally called 'Registry').
*/

#ifndef OWNEDREGISTRY_GRAEHL_2015_10_22_HPP
#define OWNEDREGISTRY_GRAEHL_2015_10_22_HPP
#pragma once

#include <sdl/Util/Unordered.hpp>

namespace sdl { namespace Util {


/**
   canonical pointers to equivalent objects

   (used only by Hypergraph/Compose)

   \author Markus Dreyer
   */
template <class Ptr, class Hash, class Pred = NonNullPointeeEqualExpensive>
class Registry {
  typedef boost::unordered_set<Ptr const*, Hash, Pred> Set;
  Set ptrs_;

 public:
  typedef Ptr PtrType;
  typedef Hash HasherType;
  typedef Pred PtrEqualType;

  Registry() {}

  /**
     \param[inout] k: if equivalent object already exists, deletePtr(k) and set k to
     the preexisting object, else make k the canonical representative (which is
     not deleted normally, unless you do it yourself w/ clearDestroy)
  */
  void add(Ptr const*& k) {
    std::pair<typename Set::iterator, bool> iNew = ptrs_.insert(k);
    if (!iNew.second) {
      deletePtr(k);
      k = *iNew.first;
    }
  }

  Ptr* insert(Ptr const* k) {
    add(k);
    return const_cast<Ptr*>(k);
  }

  /// k was added previously so k is canonical
  void eraseDestroyAdded(Ptr const* k) {
    assert(ptrs_.find(k) != ptrs_.end() && *ptrs_.find(k) == k);
    ptrs_.erase(k);
    deletePtr(k);
  }

  void erase(Ptr const* k) { ptrs_.erase(k); }

  void clearDestroy() {
    destroy();
    ptrs_.clear();
  }

  ~Registry() { destroy(); }

 private:

  struct DeleteAny {
    template <class Ptr>
    void operator()(Ptr p) const {
      delete p;
    }
  };
  void destroy() { visitUnordered(ptrs_, DeleteAny()); }
};


}}

#endif
