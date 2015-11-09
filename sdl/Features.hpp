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

    feature (name or id) -> (float) value

*/

#ifndef FEATURES_JG_2014_10_09_HPP
#define FEATURES_JG_2014_10_09_HPP
#pragma once

#define SDL_EXTERNAL_FEATURES_SWAP_ONLY 1

#include <sdl/Types.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/ZeroInitializedArray.hpp>
#include <sdl/Util/Fields.hpp>
#include <sdl/IntTypes.hpp>
#include <sdl/Util/Log10Ln.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {

typedef uint32 FeatureId;
// formerly in FeatureBot/IFeature - could move to Features.hpp instead
typedef std::string FeatureName;
typedef SdlFloat FeatureValue;
typedef SdlFloat FeatureValueWeight;  // Hypergraph::FeatureWeight conflict
typedef std::pair<FeatureName, FeatureValue> FeatureEntry;
typedef std::pair<FeatureName const, FeatureValue> FeaturePair;

typedef std::map<FeatureName, FeatureValue> Features;
typedef std::vector<FeatureEntry> FeaturesVec;
typedef Features::value_type NamedFeature;
typedef Features NamedFeatureWeights; //TODO: should definitely be (compiled to) farm_hash_map for speed - see FeatureWeightsByCase
typedef NamedFeatureWeights NamedFeatureValues;

/// rule feature encodings (for boost serialization grammar db format)
typedef uint32 RuleFeatureId;
FeatureId const kNullFeatureId = (FeatureId)-1;

// don't change to 64-bit featureid; serialized grammar rules depend on 32-bit.
typedef std::map<RuleFeatureId, FeatureValue> SparseFeatures;
typedef SparseFeatures::value_type SparseFeatureEntry;

template <class Map, class Key>
inline FeatureValue featureValue(Map const& map, Key const& key) {
  typename Map::const_iterator i = map.find(key);
  return i == map.end() ? (FeatureValue)0 : i->second;
}

template <class Map, class Key>
inline FeatureValue featureWarn(Map const& map, Key const& key) {
  typename Map::const_iterator i = map.find(key);
  if (i == map.end()) {
    SDL_WARN(FeatureValue, "missing feature '"<<key<<"'");
    return (FeatureValue)0;
  } else
    return i->second;
}

template <class Map, class Key>
inline FeatureValue featureRequire(Map const& map, Key const& key) {
  typename Map::const_iterator i = map.find(key);
  if (i == map.end())
    SDL_THROW_LOG(FeatureValue, ConfigException, "missing feature '"<<key<<"'");
  return i->second;
}

template <class Map, class Key>
inline FeatureValue featureValueRequire(Map const& map, Key const& key) {
  typename Map::const_iterator i = map.find(key);
  return i == map.end() ? (FeatureValue)0 : i->second;
}

inline void printNamedFeatures(std::ostream& out, Features const& features) {
  Util::Sep featSep;
  for (Features::const_iterator i = features.begin(), e = features.end(); i != e; ++i)
    out << featSep << i->first << "=" << i->second;
}

struct PrintNamedFeatures {
  Features const& features;
  PrintNamedFeatures(Features const& features) : features(features) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintNamedFeatures const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { printNamedFeatures(out, features); }
};

struct AddToFeatures {
  Features& features;
  AddToFeatures(Features& features) : features(features) {}
  void operator()(std::string const& name, FeatureValue val) const { features[name] += val; }
};

/// overwrite to with vals from from. return true if the keys were disjoint (no overwrites)
inline bool disjointUnion(Features& to, Features const& from) {
  std::size_t expect = to.size() + from.size();
  for (Features::const_iterator i = from.begin(), e = from.end(); i != e; ++i) to[i->first] = i->second;
  return to.size() == expect;
}

// move but not copy
typedef Util::UnsizedArray<FeatureValue> DenseFeatures;

FeatureValue constexpr kLnToFeatureValue = (SdlFloat)-M_LOG10E; // multiply ln(prob) by this for neglog10 cost (e^x)

/// if str has e.g. "myfeat=4.3 ...", sets to["myfeat"]=4.3 (to may be filled
/// with features already; only the ones named in str are overwritten)
inline void addFeatures(Features& to, Slice const& str) {
  // TODO .cpp
  using namespace Util;
  // TODO .cpp
  FieldGenerator fields(str, ' ');
  for (; fields; fields.got()) {
    Field f = fields.get();
    if (!f.empty()) {
      Pchar eq = f.find_first('=');
      if (!eq)
        SDL_THROW_LOG(Features.addFeatures, RuleFormatException, "addFeatures - bad name=val format for '"
                                                                     << f << "' in " << Util::Field(str));
      FeatureValue& val = to[std::string(f.first, eq)];
      f.first = eq + 1;
      if (f.first + 2 < f.second && f.first[0] == 'e' && f.first[1] == '^') {
        f.first += 2;
        val = kLnToFeatureValue * f.toReal<FeatureValue>();
      } else
        val = f.toReal<FeatureValue>();
    }
  }
}

inline void addFeatures(Features& to, std::string const& str) {
  addFeatures(to, toSlice(str));
}

inline void readNamedFeatures(std::istream& in, Features& f) {
  // TODO: test
  // TODO .cpp (as for everything in this root dir, in what lib? Util?)
  FeatureEntry x;
  while (in >> x.first) {
    if (in >> x.second) {
      if (!f.insert(x).second)
        SDL_WARN(Features.readNamedFeatures, "duplicate feature weight for '"
                                                 << x.first << " " << x.second
                                                 << "' in 'name val' file - keeping previous weight");
    } else
      SDL_THROW_LOG(Features.readNamedFeatures, FileFormatException, "couldn't read feature weight for '"
                                                                         << x.first << "'");
  }
}


}

#endif
