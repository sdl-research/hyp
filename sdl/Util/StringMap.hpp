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

    like unordered_map<std::string, Val> but different interface; allows efficient
    lookup of substring without creating temporary, and doesn't expect deletion of
    keys

    the empty key is reserved for "not found"

    see also Indexed.hpp (using stable_vector for fewer string copies)
*/

#ifndef STRINGMAP_JG_2014_01_03_HPP
#define STRINGMAP_JG_2014_01_03_HPP
#pragma once

#include <sdl/Util/Unordered.hpp>
#include <string>

namespace sdl {
namespace Util {

template <class Val>
struct StringMap {
  typedef farm_hash_map<std::string, Val> Vals;

  Vals vals;

  Val defaultVal;

  StringMap() : defaultVal() { Util::setEmptyKey(vals); }

  std::size_t size() const { return vals.size(); }

  typedef std::pair<char const*, char const*> Slice;

  template <class Val2>
  void set(std::string const& key, Val2&& val) {
    vals[key] = std::forward<Val2>(val);
  }

  void set(Slice const& key, Val const& val) { vals[std::string(key.first, key.second)] = val; }

  /// \return whether insert(key,val) succeeded (don't overwrite existing (key,val'))
  template <class Val2>
  bool add(std::string const& key, Val2&& val) {
    return vals.insert(Vals::value_type(key, std::forward<Val2>(val))).second;
  }

  /**
     \return &vals[key] if it exists, else NULL.
  */
  Val const* operator()(std::string const& key) const {
    typename Vals::const_iterator i = vals.find(key);
    return i == vals.end() ? NULL : &i->second;
  }

  /**
     \return vals[key] if it exists, else default.
  */
  Val const& operator[](std::string const& key) const {
    typename Vals::const_iterator i = vals.find(key);
    return i == vals.end() ? defaultVal : i->second;
  }
};


}}

#endif
