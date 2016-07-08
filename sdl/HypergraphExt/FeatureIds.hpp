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

    Manage global feature-id space.

    TODO: move this to xmt/ (not really xmt/Hypergraph specific, though it does mostly relate to
   Hypergraph/FeatureHypergraph

    TODO: dynamically manage, once we have input feature hypergraphs using string keys.

    TODO: make id and name space depend on a configured mapping (e.g. on the fly, never dealing with raw ids
   in input/output, or having only some ids fixed - essentially similar to Vocabulary), modules registering
   their own fixed constant number of features vs open-class lexicalized features, etc
*/

#ifndef FEATUREIDS_JG201286_HPP
#define FEATUREIDS_JG201286_HPP
#pragma once

#include <sdl/Hypergraph/FeatureIdRange.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Hypergraph {

/** Stopgap: constant ids.

    we also have in TransformationRequest Hypergraph/NamedFeatureIds for
    kPhraseDecoderBegin... feature ids

    Eventually a configurable dictionary type resource (global adding
    strings->id might be bad in that it would require locking and might leak
    memory across many sentences if there are many dynamically computed
    indicators)

    ranges are half-open [begin, end) {x: begin <= x < end}
*/

struct FeatureIds {
  enum {
    kBillion = 1000 * 1000 * 1000,
    kAlignmentBegin = 0,
    kAlignmentEnd = kBillion / 2,
    knAlignment = kAlignmentEnd,
    kPhraseAlignmentBegin = kAlignmentEnd,
    knPhraseAlignment = kBillion / 4,
    kPhraseAlignmentEnd = kBillion - knPhraseAlignment,
    kTokenizedAlignmentBegin = kPhraseAlignmentEnd,
    kTokenizedAlignmentEnd = kBillion,

    kOCRCChannel = kBillion,
    kOCRCLM,
    kLmCapitalizeChannel,
    kLmCapitalizeLM  // 03

    // The first 5 disjoint cases are mutually exclusive:
    // see Hypergraph/IsCase.hpp
    ,
    kIsCaseBegin = kBillion + 100,
    kIsNoCase = kIsCaseBegin  // 101
    ,
    kIsFullUpperCase  // 102
    ,
    kIsFullLowerCase  // 103
    ,
    kIsTitleCase  // 104
    ,
    kIsMixedCase  // 105
    ,
    kDisjointCasesEnd

    ,
    kIsFirstUpperCase = kDisjointCasesEnd,
    kIsFirstLowerCase

    ,
    kIsSameCaseAsAlignedWord,
    kIsNotSameCaseAsAlignedWord

    ,
    kIsIdenticalToAlignedWord,
    kIsNotIdenticalToAlignedWord

    ,
    kIsFullUpperAndSrcIsFullUpper,
    kIsNotFullUpperAndSrcIsFullUpper

    ,
    kLogProbSrcWordGivenTrgWord,
    kLogProbTrgWordGivenSrcWord

    ,
    kIsCaseEnd

    // kIsCase features that fire only at sentence start:
    ,
    kIsCaseAtSentStart_begin = kIsCaseEnd,
    kIsCaseAtSentStart_end = kIsCaseAtSentStart_begin + (kIsCaseEnd - kIsCaseBegin)

    // kIsCase features that fire only when not at sentence start:
    ,
    kIsCaseNotAtSentStart_begin = kIsCaseAtSentStart_end,
    kIsCaseNotAtSentStart_end = kIsCaseNotAtSentStart_begin + (kIsCaseEnd - kIsCaseBegin)

    // TODO: assignable id (not 0...n) SDL_ENUM name<->id mapper?
    // or offset regular SDL_ENUM ids?

    // Reserve 100,000 feature IDs for TrainableCapitalizer:
    ,
    kTrainableCapitalizerBegin = kIsCaseEnd,
    kTrainableCapitalizerEnd = kTrainableCapitalizerBegin + 100000

    // Reserve 101 million feature IDs for PhraseDecoder:
    ,
    kPhraseDecoderBegin = kTrainableCapitalizerEnd,
    kPhraseDecoderEnd = kPhraseDecoderBegin + 101000000

    /////////////////////////////////////////////////////
    // Within the phrase decoder range: Reserve the last
    // 100,000,000 feature IDs for sentence-specific
    // features. These can be reassigned from sentence to
    // sentence. Use case: store the rules that were used to
    // create a sentence translation as features in a hypergraph. these don't have NamedFeatureIds
    ,
    kPhraseDecoderSentenceSpecificBegin = kPhraseDecoderEnd - 100000000,
    kPhraseDecoderSentenceSpecificEnd = kPhraseDecoderEnd
    /////////////////////////////////////////////////////
    // Features for unk tokens and transliterate tokens
    ,
    kIsUnk,
    kIsTransliterate,
    kOptionalConstraint,
    kReservedFeaturesEnd
  };

  /**
     feature name for id.
  */
  static std::string describeFeature(FeatureId id);

  static std::string describeAlignmentFeatures();
  static std::string describeIsCaseFeatures();

  static inline bool isCaseFeature(FeatureId id) { return id >= kIsCaseBegin && id < kIsCaseEnd; }

  static inline bool isAlignmentFeature(FeatureId id) { return id >= kAlignmentBegin && id < kAlignmentEnd; }

  static inline bool isTokenizedAlignmentFeature(FeatureId id) {
    return id >= kTokenizedAlignmentBegin && id < kTokenizedAlignmentEnd;
  }

  static inline bool isPhraseDecoderFeature(FeatureId id) {
    return id >= kPhraseDecoderBegin && id < kPhraseDecoderEnd;
  }

  static inline bool isPhraseDecoderSentenceSpecificFeature(FeatureId id) {
    return id >= kPhraseDecoderSentenceSpecificBegin && id < kPhraseDecoderSentenceSpecificEnd;
  }

  static inline bool isNonTunableFeature(FeatureId id) {
    return isAlignmentFeature(id) || id >= kPhraseDecoderSentenceSpecificBegin && id < kReservedFeaturesEnd;
  }

  static inline bool isTunableFeature(FeatureId id) { return !isNonTunableFeature(id); }

  /** feature id for OCRC Channel. */
  static inline FeatureId OCRCChannel() { return (FeatureId)kOCRCChannel; }

  /** feature id for OCRC LM. */
  static inline FeatureId OCRCLM() { return (FeatureId)kOCRCLM; }

  static inline FeatureId alignmentFeatureForPosition(FeatureId position) {
    FeatureId r = kAlignmentBegin + (FeatureId)position;
    assert(r < kAlignmentEnd);
    return r;
  }

  static inline FeatureId positionForAlignmentFeature(FeatureId id) {
    assert(isAlignmentFeature(id));
    FeatureId position = id - kAlignmentBegin;
    return (FeatureId)position;
  }

  static inline FeatureId tokenizedAlignmentFeatureForPosition(FeatureId position) {
    FeatureId r = kTokenizedAlignmentBegin + (FeatureId)position;
    assert(r < kTokenizedAlignmentEnd);
    return r;
  }

  static inline FeatureId positionForTokenizedAlignmentFeature(FeatureId id) {
    assert(isTokenizedAlignmentFeature(id));
    FeatureId position = id - kTokenizedAlignmentBegin;
    return (FeatureId)position;
  }

  static inline FeatureId phraseForFeature(FeatureId id) {
    assert(id >= kPhraseAlignmentBegin);
    assert(id < kPhraseAlignmentEnd);
    return id - kPhraseAlignmentBegin;
  }

  static inline FeatureId featureForPhrase(FeatureId phrase) {
    FeatureId id = phrase + kPhraseAlignmentBegin;
    assert(id < kPhraseAlignmentEnd);
    return id;
  }
};

inline FeatureIdRange tokenizedAlignmentIds() {
  return FeatureIdRange(FeatureIds::kTokenizedAlignmentBegin, FeatureIds::kTokenizedAlignmentEnd);
}

inline FeatureIdRange alignmentIds() {
  return FeatureIdRange(FeatureIds::kAlignmentBegin, FeatureIds::kAlignmentEnd);
}

inline FeatureIdRange isCaseIds() {
  return FeatureIdRange(FeatureIds::kIsCaseBegin, FeatureIds::kIsCaseEnd);
}

inline FeatureIdRange trainableCapitalizerIds() {
  return FeatureIdRange(FeatureIds::kTrainableCapitalizerBegin, FeatureIds::kTrainableCapitalizerEnd);
}

inline FeatureIdRange phraseDecoderIds() {
  return FeatureIdRange(FeatureIds::kPhraseDecoderBegin, FeatureIds::kPhraseDecoderEnd);
}

inline FeatureIdRange phraseDecoderSentenceSpecificIds() {
  return FeatureIdRange(FeatureIds::kPhraseDecoderSentenceSpecificBegin,
                        FeatureIds::kPhraseDecoderSentenceSpecificEnd);
}


}}

#endif
