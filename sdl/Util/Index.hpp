// Copyright 2014-2015 SDL plc
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

    canonical equal-by-content pointers (Registry) or pointer->index mapping (Index)
*/

#ifndef SDL_UTIL_INDEX_H_
#define SDL_UTIL_INDEX_H_
#pragma once


#include <vector>
#include <boost/functional/hash.hpp>
#include <sdl/Util/Unordered.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Util/Debug.hpp>
#include <sdl/Util/Map.hpp>
#include <sdl/Util/PointerWithFlag.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/NonNullPointee.hpp>
#include <sdl/IntTypes.hpp>

namespace sdl {
namespace Util {

struct DeleteAny {
  template <class Ptr>
  void operator()(Ptr p) const {
    delete p;
  }
};

template <class Ptr>
struct DeleteKey {
  void operator()(Ptr p) const {
    delete p;
  }
  template <class Val>
  void operator()(std::pair<Ptr const, Val> const& p) const {
    delete p.first;
  }
  template <class Val>
  void operator()(std::pair<Ptr, Val> const& p) const {
    delete p.first;
  }
};

template<class Ptr>
struct IndexVisitor {
  virtual void visit(Ptr) const = 0;
};

template <class Ptr>
struct DeleteIndexVisitor : public Util::IndexVisitor<Ptr> {
  virtual void visit(Ptr const& p) const {
    delete p;
  }
};

template <class Ptr>
struct PrintPtr {
  Ptr p;
  friend inline std::ostream& operator<<(std::ostream &out, PrintPtr const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream &out) const {
    if (p)
      out << *p;
    else
      out << "NULL";
  }
};

/**
   Maps objects to integer indices

   key arguments are pointers - passed by reference because they're updated to
   point at the copy in the hash after adding/getting (if there already is one,
   then the argument passed in gets deleted, and instead points at the existing
   copy)

   \author dreyer, graehl
*/
template<class Ptr,
         class Id = std::size_t,
         class Hash = boost::hash<Ptr>,
         class Pred = std::equal_to<Ptr>,
         bool UseFreeList = true
         >
class Index {

 private:
  typedef boost::unordered_map<Ptr, Id, Hash, Pred> Map;

 public:

  // types
  typedef Ptr PtrType;
  typedef Id IdType;
  typedef Hash Hasher;
  typedef Pred PtrEqual;


  Index()
      : freelist_((PointerOrFreeIndex)kNullFreelistIndex)
  {
    assert(!msb(freelist_));
    BOOST_STATIC_ASSERT((boost::is_pointer<Ptr>::value));
    BOOST_STATIC_ASSERT(sizeof(*(Ptr)0) > 1);
  }

  static inline void setPtr(Ptr &k, Ptr const& newk) {
    if (k != newk) {
      delete k;
      k = newk;
    }
  }

  IdType add(Ptr& k) {
    bool ignore;
    return addIsNew(k, ignore);
  }

  IdType addIsNew(Ptr& k, bool &isNew) {
    assert(!msb(freelist_));
    assert(!lsb(k));
    bool const noneFree = UseFreeList ? freelist_ == (PointerOrFreeIndex)kNullFreelistIndex : true;
    IdType const id = noneFree ? data_.size() : freelist_;
    std::pair<typename Map::iterator, bool> iNew = indexInData_.insert(typename Map::value_type(k, id));
    if ((isNew = iNew.second)) {
      if (noneFree) {
        assert(id == data_.size());
        data_.push_back(k);
        SDL_TRACE(PhraseBased.Index.addIsNew, "created new state "<<*k << " => new id=" << id);
        return id;
      } else {
        assert(UseFreeList);
        assert(id == freelist_);
        Ptr &kid = data_[id];
        freelist_ = integerFromPointer(kid); // link (next) of freelist is stored as shifted int w/ lsb added
        assert(!msb(freelist_));
        kid = k;
        SDL_TRACE(PhraseBased.Index.addIsNew, "created new state "<<*k << " => reused id=" << id);
        return id;
      }
    } else {
      SDL_TRACE(PhraseBased.Index.addIsNew,
                "rereached state "<<*k << " => id=" << iNew.first->second
                <<" (setting to previous state "<<*iNew.first->first);
      setPtr(k, iNew.first->first);
      return iNew.first->second;
    }
  }

  IdType nextId(Ptr k) const {
    bool const noneFree = UseFreeList ? freelist_ == (PointerOrFreeIndex)kNullFreelistIndex : true;
    return noneFree ? data_.size() : freelist_;
  }

  bool contains(Ptr k) const {
    return indexInData_.find(k) != indexInData_.end();
  }

  IdType get(Ptr& k) const {
    typename Map::const_iterator found = indexInData_.find(k);
    if (found == indexInData_.end())
      throw IndexException("Util::Index key not found");
    setPtr(k, found->first);
    return found->second;
  }

  IdType getConst(Ptr k) const {
    return get(k);
  }

  Ptr get(IdType id) const {
    assert(!lsb(data_[id]));
    assert(getConst(data_[id]) == id);
    return data_[id];
  }

  PrintPtr<Ptr> getPrint(IdType id) const {
    PrintPtr<Ptr> r;
    r.p = data_[id];
    if (lsb(r.p))
      r.p = 0;
    return r;
  }

  void accept(const IndexVisitor<Ptr>& visitor) {
    for (typename Map::iterator it = indexInData_.begin(), end = indexInData_.end(); it != end; ++it)
      visitor.visit(it->first);
  }

  // call after accept w/ deleter if you want to avoid multiple-delete confusion
  void clear() {
    indexInData_.clear();
    data_.clear();
    freelist_ = (PointerOrFreeIndex)kNullFreelistIndex;
    assert(!msb(freelist_));
  }

  void clearDestroy() {
    if (indexInData_.empty()) {
      data_.clear();
      freelist_ = (PointerOrFreeIndex)kNullFreelistIndex;
      assert(!msb(freelist_));
      return;
    }
#define SDL_INDEX_CAREFUL_DELETE 0
    // shouldn't be necessary; debugging CM-434 crash in release
#if SDL_INDEX_CAREFUL_DELETE
    std::vector<Ptr> toDelete(indexInData_.size());
    Ptr *ob = &*toDelete.begin();
    Ptr *o = ob;
#endif
    // this care is taken because you may have a bunch of same-hash items (e.g. CM-434)
    for (std::size_t b = 0, be = indexInData_.bucket_count(); b < be; ++b) {
      for (typename Map::const_local_iterator i = indexInData_.cbegin(b), end = indexInData_.cend(b); i != end;) {
        assert(!lsb(i->first));
#if SDL_INDEX_CAREFUL_DELETE
        *o++ = i->first;
        ++i;
#else
        Ptr d = i->first;
        ++i;
        delete d;
#endif
      }
    }
    clear();
#if SDL_INDEX_CAREFUL_DELETE
    assert(o - ob == toDelete.size());
    for (; o != ob; ++o)
      delete *o;
#endif
  }

  /// make sure that things inserted after this use ids greater than id
  void reserveIdsUpTo(IdType id) {
    if (id > data_.size())
      data_.resize(id);
  }

  bool empty() {
    return indexInData_.empty();
  }

  /// if !UseFreeList, this would be the next id for a ptr not already in
  /// indexInData_; in any case the next id is no more than this.
  std::size_t size() const {
    return data_.size();
  }

  /**
     idempotent on id (but not erase(id, true), insert(...), erase(id, true) - that may erase the new item!)
  */
  void eraseAndDeletePtrAtId(IdType id, bool reuseId = true) {
    std::size_t const nkeys = data_.size();
    if (id < nkeys) {
      Ptr &key = data_[id];
      bool const present = UseFreeList ? (bool)key && !lsb(key) : (bool)key;
      if (present) {
        assert(Util::contains(indexInData_, key));
        indexInData_.erase(key);
        delete key; // *after* we erase, not before
        if (!reuseId)
          key = 0;
        else if (id == nkeys - 1)
          data_.pop_back();
        else if (UseFreeList) {
          key = (Ptr)pointerForInteger(freelist_);
          assert(!msb(freelist_));
          assert(lsb(key));
          assert(integerFromPointer(key) == freelist_);
          freelist_ = id;
          assert(!msb(freelist_));
        } else
          key = 0;
      }
    }
  }

  ~Index() {
    clearDestroy();
  }
 private:

  void destroy() {
  }

  Map indexInData_;               // Ptr -> Id
  std::vector<Ptr> data_; // Id -> Ptr (if lsb is 0)

  /// this is almost like an object pool, except we're explicitly index-based
  /// rather than pointer (so we can key external data off that index, instead
  /// of having members pointing to that data.

  //TODO: try object-pool/pointer approach +  compare?

  typedef uint64 PointerOrFreeIndex;

# ifdef _WIN32
  enum : PointerOrFreeIndex
# else
  enum
#endif
  {
    kMaxPointerOrFreeIndex = (PointerOrFreeIndex)-1, // we need to keep this value to force uint64 type (if you have just 0x7ff.... then it's no good - your enum might be signed)
    kNullFreelistIndex = (PointerOrFreeIndex)kMaxPointerOrFreeIndex >> 1
  };
  PointerOrFreeIndex freelist_; // first free index, and next is index(ptrOrFreeList_[freelist_]), or else kNullFreelistIndex
};

///

/**
   canonical pointers to equivalent objects

   \author Markus Dreyer

   //TODO: defaults don't compare contents of pointer, so what's the point? if this is unused, delete it
   */
template<class Ptr,
         class Hash,
         class Pred = NonNullPointeeEqualExpensive
         >
class Registry {
  typedef boost::unordered_set<Ptr const*, Hash, Pred> Set;
  Set ptrs_;
 public:
  typedef Ptr PtrType;
  typedef Hash Hasher;
  typedef Pred PtrEqual;

  Registry() {}

  /**
     \param[inout] k: if equivalent object already exists, delete k and set k to
     the preexisting object, else make k the canonical representative (which is
     not deleted normally, unless you do it yourself w/ clearDestroy)
  */
  void add(Ptr const* & k) {
    std::pair<typename Set::iterator, bool> iNew = ptrs_.insert(k);
    if (!iNew.second) {
      delete k;
      k = *iNew.first;
    }
  }

  Ptr * insert(Ptr const* k) {
    add(k);
    return const_cast<Ptr*>(k);
  }

  /// k was added previously so k is canonical
  void eraseDestroyAdded(Ptr const* k) {
    assert(ptrs_.find(k) != ptrs_.end() && *ptrs_.find(k) == k);
    ptrs_.erase(k);
    delete k;
  }

  void erase(Ptr const* k) {
    ptrs_.erase(k);
  }

  void accept(const IndexVisitor<Ptr const*>& visitor) {
    for (typename Set::iterator it = ptrs_.begin(), end = ptrs_.end(); it != end; ++it) {
      visitor.visit(*it);
    }
  }

  void clearDestroy() {
    destroy();
    ptrs_.clear();
  }

  ~Registry() {
    destroy();
  }

 private:
  void destroy() {
    visitUnordered(ptrs_, DeleteAny());
  }

};


}}

#endif
