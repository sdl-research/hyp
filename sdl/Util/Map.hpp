/** \file

    map-like access helpers:

    1. avoid cumbersome std::pair<typename Map::iterator, bool> i = map.insert(KeyValue(k, val));

    2. avoid cumbersome typename Map::iterator i = map.find(key); if (i != map.end()) ...

    3. support unsigned-integer-key vectors (good idea if they're not too
    sparse; otherwise wastes memory) through the same API
*/

#ifndef SDL_UTIL_MAP_HPP
#define SDL_UTIL_MAP_HPP
#pragma once

#include <utility>
#include <stdexcept>
#include <boost/utility/enable_if.hpp>

#include <sdl/Util/Contains.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Util/SortedMap.hpp>
#include <sdl/Util/Sorted.hpp>
#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Util {

/**
   set map[k] = val, returning true if any previous value existed.
*/
template <class Map>
bool replaceKeyValue(Map& map, typename Map::key_type const& k, typename Map::mapped_type const& val) {
  typedef typename Map::value_type KeyValue;
  std::pair<typename Map::iterator, bool> i = map.insert(KeyValue(k, val));
  bool const existed = !i.second;
  if (existed) i.first->second = val;
  return existed;
}

/**
   post: m[k]=initValue if m[k] didn't exist, else f(initValue, value_type &m[k]) (may modify m[k])

   \return true iff m[k] didn't exist before.
*/
template <class Map, class UpdateBy, class UpdateByInit>
inline bool updateByIsNew(UpdateBy const& updateByFn, Map& map, typename Map::key_type const& k,
                          UpdateByInit const& initValue) {
  typedef typename Map::value_type KeyValue;
  std::pair<typename Map::iterator, bool> i = map.insert(KeyValue(k, initValue));
  if (!i.second) updateByFn(initValue, i.first->second);
  return i.second;
}

/**
   post: m[k]=initValue if m[k] didn't exist, else f(initValue, value_type &m[k]) (may modify m[k])

   \return m[k]
*/
template <class Map, class UpdateBy, class UpdateByInit>
inline typename Map::mapped_type& updateBy(UpdateBy const& updateByFn, Map& map,
                                           typename Map::key_type const& k, UpdateByInit const& initValue) {
  typedef typename Map::value_type KeyValue;
  typedef typename Map::mapped_type Value;
  std::pair<typename Map::iterator, bool> i = map.insert(KeyValue(k, initValue));
  Value& rval = i.first->second;
  if (!i.second) updateByFn(initValue, rval);
  return rval;
}

/**
   post: m[k]=initValue if m[k] didn't exist, else f(updateValue, value_type &m[k]) (may modify m[k])

   \return m[k]
*/
template <class Map, class UpdateBy, class UpdateByInit>
inline typename Map::mapped_type& updateBy(UpdateBy const& updateByFn, Map& map,
                                           typename Map::key_type const& k, UpdateByInit const& initValue,
                                           UpdateByInit const& updateValue) {
  typedef typename Map::value_type KeyValue;
  typedef typename Map::mapped_type Value;
  std::pair<typename Map::iterator, bool> i = map.insert(KeyValue(k, initValue));
  Value& rval = i.first->second;
  if (!i.second) updateByFn(updateValue, rval);
  return rval;
}

/**
   same as updateBy(updateBy, m, keyInitValuePair.first, keyInitValuePair.second) but faster.
*/
template <class Map, class UpdateBy>
inline typename Map::mapped_type& updateBy(UpdateBy const& updateByFn, Map& map,
                                           typename Map::value_type const& keyInitValuePair) {
  std::pair<typename Map::iterator, bool> i = map.insert(keyInitValuePair);
  typename Map::mapped_type& rval = i.first->second;
  if (!i.second) updateByFn(keyInitValuePair.second, rval);
  return rval;
}


/** Add a default constructed m[k] if one didn't exist.

    \param val out: points to m[k]

    \return true iff m[k] didn't already exist */
template <class Map>
inline bool update(Map& map, typename Map::key_type const& k, typename Map::mapped_type*& val) {
  std::pair<typename Map::iterator, bool> i
      = map.insert(typename Map::value_type(k, typename Map::mapped_type()));
  val = &i.first->second;
  return i.second;
}

template <class Map, class CopyValueFrom>
inline bool update(Map& map, typename Map::key_type const& k, CopyValueFrom const& valueFrom,
                   typename Map::mapped_type*& val) {
  std::pair<typename Map::iterator, bool> i = map.insert(typename Map::value_type(k, valueFrom));
  val = &i.first->second;
  return i.second;
}

template <class Map, class CopyValueFrom>
inline bool updateValue(Map& m, typename Map::key_type const& k, CopyValueFrom const& valueFrom,
                        typename Map::mapped_type& val) {
  std::pair<typename Map::iterator, bool> i = m.insert(typename Map::value_type(k, valueFrom));
  val = i.first->second;
  return i.second;
}

/**
   exactly like update, but uses a find() first to save making a copy of key (e.g. key is a vector).
*/
template <class Map>
inline bool updateHeavyKey(Map& map, typename Map::key_type const& key, typename Map::mapped_type*& val) {
  typename Map::iterator i = map.find(key);
  if (i == map.end()) {
    val = &(map[key]);
    return true;
  } else {
    val = &i->second;
    return false;
  }
}

/** Set m[keyValPair.first]=keyValPair.second if k not in m, else set keyValPair=*map.find(keyValPair.first).

    \param keyValPair in/out

    \return true iff k wasn't in m (keyValPair was inserted) . */
template <class Map>
inline bool update(Map& map, typename Map::value_type& keyValPair) {
  std::pair<typename Map::iterator, bool> i = map.insert(keyValPair);
  if (!i.second) keyValPair = *i.first;
  return i.second;
}

/** Set m[key]=val if k not in m, else set val=*map.find(key).

    \param val in/out gets existing m[key] if return was false

    \return true iff k wasn't in m (val was inserted) . */
template <class Map>
inline bool update(Map& map, typename Map::key_type const& key, typename Map::mapped_type& val) {
  std::pair<typename Map::iterator, bool> i = map.insert(typename Map::value_type(key, val));
  if (!i.second) val = i.first->second;
  return i.second;
}

/**
   \return whether m[key] was created as val - if false, then m[key] already existed.
*/
template <class Map>
inline bool supplyMissing(Map& map, typename Map::key_type const& key, typename Map::mapped_type const& val) {
  return map.insert(typename Map::value_type(key, val)).second;
}

/**
   \return m[key] but throw DuplicateException if m[key] already existed
*/
template <class Map>
typename Map::mapped_type& addMissing(Map& map, typename Map::key_type const& key) {
  std::pair<typename Map::iterator, bool> i
      = map.insert(typename Map::value_type(key, typename Map::mapped_type()));
  if (!i.second) throw DuplicateException("addMissing: key already had value");
  return i.first->second;
}

template <class Map>
inline typename Map::mapped_type const& getOrElse(Map const& map, typename Map::key_type const& k,
                                                  typename Map::mapped_type const& valDefault) {
  typename Map::const_iterator i = map.find(k);
  if (i == map.end()) return valDefault;
  return i->second;
}

template <class Map, class AtLeast>
bool valueIsAtLeast(Map const& map, typename Map::key_type const& k, AtLeast const& valAtLeast) {
  typename Map::const_iterator i = map.find(k);
  if (i == map.end()) return false;
  return i->second >= valAtLeast;
}

template <class Map>
inline typename Map::mapped_type getOrDefault(Map const& map, typename Map::key_type const& k) {
  typename Map::const_iterator i = map.find(k);
  if (i == map.end()) return typename Map::mapped_type();
  return i->second;
}

template <class Map>
inline typename Map::mapped_type const& maybeGet(Map const& map, typename Map::key_type const& k,
                                                 typename Map::mapped_type& store) {
  typename Map::const_iterator i = map.find(k);
  if (i != map.end()) store = i->second;
  return store;
}

/**
   \return &m[k] if it exists, else NULL.
*/
template <class Map>
inline typename Map::mapped_type const* getPtr(Map const& map, typename Map::key_type const& k) {
  typename Map::const_iterator i = map.find(k);
  return i == map.end() ? (typename Map::mapped_type const*)0 : &i->second;
}

template <class Map>
inline typename Map::mapped_type* getPtr(Map& map, typename Map::key_type const& k) {
  typename Map::iterator i = map.find(k);
  return i == map.end() ? (typename Map::mapped_type*)0 : &i->second;
}

/**
   \throw IndexException if m[k] didn't exist

    \return m[k].
 */

template <class Map>
inline typename Map::mapped_type const& getOrThrow(Map const& map, typename Map::key_type const& k) {
  typename Map::const_iterator i = map.find(k);
  if (i == map.end()) throw IndexException("getOrThrow: key not found");
  return i->second;
}

template <class Vec>
inline typename Vec::value_type const& getOrElse(Vec const& v, typename Vec::size_type i,
                                                 typename Vec::value_type const& valDefault) {
  return i < v.size() ? v[i] : valDefault;
}

/**
   return (ref to) f(k), memoized in map map.
*/
template <class Map, class F>
inline typename Map::mapped_type& getLazy(F& f, Map& map, typename Map::key_type const& k) {
  std::pair<typename Map::iterator, bool> i
      = map.insert(typename Map::value_type(k, typename Map::mapped_type()));
  if (i.second) return (i.first->second = f(k));
  return i.first->second;
}

template <class Set, class K>
inline typename Set::value_type const& value(Set& s, K const& k) {
  return *s.find(k);
}

template <class Set, class K, class V>
inline typename Set::mapped_type const& setValue(Set& s, K const& k, V const& v) {
  return s[k] = v;  // instead of insert, because this is faster than value_type(k, v) for expensive k
}

/**
   return whether any values changed.
*/
template <class Map, class MapNew>
bool overrideMap(Map& old, MapNew const& makenew) {
  bool anyChanged = false;
  for (typename MapNew::const_iterator i = makenew.begin(), e = makenew.end(); i != e; ++i) {
    typename Map::mapped_type& val = old[i->first];
    if (val != i->second) {
      anyChanged = true;
      val = i->second;
    }
  }
  return anyChanged;
}

/**
   return whether any values changed.
*/
template <class Map, class MapNew>
void overrideMapSaveChanged(Map& old, MapNew const& makenew, MapNew& changed) {
  for (typename MapNew::const_iterator i = makenew.begin(), e = makenew.end(); i != e; ++i) {
    typename MapNew::key_type const& key = i->first;
    typename MapNew::mapped_type const& newVal = i->second;
    typename Map::mapped_type& val = old[key];
    if (val != newVal) {
      changed[key] = newVal;
      val = newVal;
    }
  }
}


template <class US>
void intersectInto(US& out, US const& in1, US const& in2) {
  out.clear();
  if (in2.size() < in1.size()) {
    intersectInto(out, in2, in1);
    return;
  }
  for (typename US::const_iterator i = in1.begin(); i != in1.end(); i++) {
    if (Util::contains(in2, *i)) out.insert(*i);
  }
}

template <class US>
void unionEq(US& out, US const& in) {
  out.insert(in.begin(), in.end());
}

template <class US>
void unionInto(US& out, US const& in1, US const& in2) {
  out.clear();
  out.insert(in1.begin(), in1.end());
  out.insert(in2.begin(), in2.end());
}

// disable_if map has hasher member type (for coordination with Unordered.hpp) - unordered_map etc have
// slightly different erase api.
/**
   eraseIf: erase if EraseIfFn(Map/Set::value_type const&) (doesn't just get the key for maps)
*/
// enable for SortedMap trait only so no ambiguity with unordered_map version
template <class EraseIfFn, class Ordered>
typename SortedMap<Ordered>::type  // void
    eraseIf(Ordered& orSet, EraseIfFn const& f) {
  for (typename Ordered::iterator i = orSet.begin(), e = orSet.end(); i != e;)  // no ++i - intentional
    if (f(*i))
      orSet.erase(i++);  // postincrement is required
    else
      ++i;
}

/**
   eraseIfVal: erase all umap[k] such that EraseIfFn(umap[k]). might be slower than a filtered copy/swap.

   (also works for set<pair<a, b> > or set<anythign with i->second>
*/
template <class EraseIfFn, class Ordered>
typename SortedMap<Ordered>::type  // void
    eraseIfVal(Ordered& map, EraseIfFn const& f) {
  for (typename Ordered::iterator i = map.begin(), e = map.end(); i != e;)  // no ++i - intentional
    if (f(i->second))
      map.erase(i++);  // postincrement is required
    else
      ++i;
}


// TODO: perhaps these remaining are redundant with above update etc. (old contents of Map.hpp):

// adds default val to table if key wasn't found, returns ref to val
template <class H, class K>
typename H::mapped_type& get_default(H& ht, K const& k, typename H::mapped_type const& v) {
  return const_cast<typename H::mapped_type&>(ht.insert(typename H::value_type(k, v)).first->second);
}

// like ht[k]=v, but you want to check your belief that ht[k] didn't exist before.  also may be faster
template <class HT, class K, class V>
void addNew(HT& ht, K const& key, V const& value = V()) {
  if (!ht.insert(typename HT::value_type(key, value)).second)
    throw DuplicateException("Key already existed in add_new(ht, key, val)");
}

// NB: value is in/out param
template <class HT, class K, class V>
bool addIsNew(HT& ht, K const& key, V& value) {
  std::pair<typename HT::iterator, bool> r = ht.insert(typename HT::value_type(key, value));
  if (!r.second) value = r.first->second;
  return r.second;
}

// for std::map and similar
template <class OrderedMap, class T>
typename OrderedMap::const_iterator lowerBoundMap(OrderedMap const& map, T const& value,
                                                  typename HasKeyType<OrderedMap>::type* = 0) {
  return map.lower_bound(value);
}

// sorted random accessible sequence e.g. sorted vector
template <class Ordered, class T>
typename Ordered::const_iterator lowerBound(Ordered const& sequence, T const& value) {
  return std::lower_bound(sequence.begin(), sequence.end(), value);
}

template <class Key>
struct FirstAscendingVs {
  template <class X>
  bool operator()(X const& a, Key const& key) const {
    return a.first < key;
  }
};

template <class Key>
FirstAscendingVs<Key> firstAscendingVs(Key const& key) {
  return FirstAscendingVs<Key>(key);
}

// sorted random accessible sequence e.g. sorted vector
template <class Ordered, class T>
typename Ordered::const_iterator lowerBoundFirst(Ordered const& sequence, T const& value) {
  return std::lower_bound(sequence.begin(), sequence.end(), value, FirstAscendingVs<T>());
}


}}

#endif
