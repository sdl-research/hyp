/** \file

    feature (name or id) -> (float) value

*/

#ifndef FEATURES_JG_2014_10_09_HPP
#define FEATURES_JG_2014_10_09_HPP
#pragma once

#include <sdl/Types.hpp>
#include <sdl/Util/PrintRange.hpp>

namespace sdl {

typedef unsigned FeatureId;
// formerly in FeatureBot/IFeature - could move to Features.hpp instead
typedef std::string FeatureName;
typedef SdlFloat FeatureValue;
typedef SdlFloat FeatureValueWeight;  // Hypergraph::FeatureWeight conflict
typedef std::pair<FeatureName, FeatureValue> FeatureEntry;
typedef std::pair<FeatureName const, FeatureValue> ConstFeatureEntry;

typedef std::map<FeatureName, FeatureValue> Features;
typedef Features::value_type NamedFeature;
typedef Features NamedFeatureWeights;
typedef NamedFeatureWeights NamedFeatureValues;

/// rule feature encodings (for boost serialization grammar db format)
typedef FeatureId RuleFeatureId;

// in case we switch to wider feature id, remember that in old dbs we use this type (but probably we never
// will)
typedef std::map<RuleFeatureId, FeatureValue> SparseFeatures;
typedef std::vector<FeatureValue> DenseFeatures;  // TODO: FixedArray? we know number in advance.

template <class Map, class Key>
inline FeatureValue featureValue(Map const& map, Key const& key) {
  typename Map::const_iterator i = map.find(key);
  return i == map.end() ? (FeatureValue)0 : i->second;
}

inline void printNamedFeatures(std::ostream& out, Features const& features) {
  Util::Sep featSep;
  for (Features::const_iterator i = features.begin(), e = features.end(); i != e; ++i)
    out << featSep << i->first << "=" << i->second;
}


}

#endif
