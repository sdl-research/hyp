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

    used by SDL.hpp to add alignment feature ids per target word.
*/

#ifndef ALIGNMENTFEATURES_JG20121116_HPP
#define ALIGNMENTFEATURES_JG20121116_HPP
#pragma once

#include <sdl/HypergraphExt/FeatureIds.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Positions.hpp>

namespace sdl {
namespace Hypergraph {


/**
   visitAlignmentFeatures calls visitFeature(id, val) for all alignment ids
   addAlignmentFeatures calls add(alignmentIds, id) (val is presumed to be 1)
*/
template <class Weight, class Enable = void>
struct AlignmentFeaturesForWeight {
  enum { features = false };
  template <class F>
  static inline void visitAlignmentFeatures(Weight const&, F const&, bool) {}
  static inline void copyAlignmentFeatures(Weight const& w, Weight& wAlignOnly, bool) {}
  static inline void setAlignmentFeatures(Weight const& w, Weight& wAlignOnly, bool) {}
};

inline FeatureId endAlignmentIds(bool alsoTokenizedAlignments) {
  return alsoTokenizedAlignments ? FeatureIds::kTokenizedAlignmentEnd : FeatureIds::kAlignmentEnd;
}

template <class Weight>
struct AlignmentFeaturesForWeight<Weight, typename Weight::IsFeatureWeight> {
  enum { features = true };
  template <class F>
  static inline void visitAlignmentFeatures(Weight const& w, F const& visitFeature,
                                            bool alsoTokenizedAlignments = false) {
    w.visitFeatureRange(FeatureIds::kAlignmentBegin, endAlignmentIds(alsoTokenizedAlignments), visitFeature);
  }
  static inline void copyAlignmentFeatures(Weight const& w, Weight& wAlignOnly,
                                           bool alsoTokenizedAlignments = false) {
    w.visitFeatureRange(FeatureIds::kAlignmentBegin, endAlignmentIds(alsoTokenizedAlignments),
                        typename Weight::AddFeature(wAlignOnly));
  }
  static inline void setAlignmentFeatures(Weight const& w, Weight& wAlignOnly,
                                          bool alsoTokenizedAlignments = false) {
    wAlignOnly.removeFeatures();
    copyAlignmentFeatures(w, wAlignOnly, alsoTokenizedAlignments);
  }
};

template <class Weight, class AlignmentIds>
void addAlignmentFeatures(Weight const& w, AlignmentIds& alignmentIds, bool alsoTokenizedAlignments = false) {
  AlignmentFeaturesForWeight<Weight>::visitAlignmentFeatures(w, firstAdder(alignmentIds), alsoTokenizedAlignments);
}

template <class Weight>
void copyAlignmentFeatures(Weight const& w, Weight& wAlignOnly, bool alsoTokenizedAlignments = false) {
  AlignmentFeaturesForWeight<Weight>::copyAlignmentFeatures(w, wAlignOnly, alsoTokenizedAlignments);
}

template <class Weight>
void setAlignmentFeatures(Weight const& w, Weight& wAlignOnly, bool alsoTokenizedAlignments = false) {
  AlignmentFeaturesForWeight<Weight>::setAlignmentFeatures(w, wAlignOnly, alsoTokenizedAlignments);
}

template <class Weight, class IdValPairVisitor>
void visitAlignmentFeatures(Weight const& w, IdValPairVisitor const& idValPairVisitor,
                            bool alsoTokenizedAlignments = false) {
  AlignmentFeaturesForWeight<Weight>::visitAlignmentFeatures(w, idValPairVisitor, alsoTokenizedAlignments);
}

template <class IdValPairVisitor>
void visitAlignmentFeatures(Features const& f, IdValPairVisitor const& idValPairVisitor,
                            bool alsoTokenizedAlignments = false) {
  FeatureId endid = endAlignmentIds(alsoTokenizedAlignments);
  Features::const_iterator i = f.lower_bound(FeatureIds::kAlignmentBegin), e = f.end();
  while (i != e)
    if (i->first < endid) {
      idValPairVisitor(*i);
      ++i;
    } else
      break;
}


}}

#endif
