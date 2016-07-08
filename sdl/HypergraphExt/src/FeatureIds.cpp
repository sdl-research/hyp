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
#include <sdl/HypergraphExt/FeatureIds.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/StringBuilder.hpp>

namespace sdl {
namespace Hypergraph {

std::string FeatureIds::describeIsCaseFeatures() {
  using namespace graehl;
  Util::StringBuilder out;
  for (FeatureId i = (FeatureId)kIsCaseBegin, e = (FeatureId)kIsCaseEnd; i < e; ++i)
    out(describeFeature(i))("[")(i)(']').append_space();
  return out.str();
}

static std::string const kSourceAlignmentEq = "SourceAlignment=";
static std::string const kFeature_ = "Feature_";
std::string FeatureIds::describeFeature(FeatureId id) {
  using namespace graehl;
  if (isAlignmentFeature(id))
    return kSourceAlignmentEq + graehl::to_string(id);
  else if (isCaseFeature(id))
    switch (id) {
      case kIsNoCase: return "NoCase";
      case kIsFullUpperCase: return "FullUpperCase";
      case kIsFullLowerCase: return "FullLowerCase";
      case kIsTitleCase: return "TitleCase";
      case kIsMixedCase: return "MixedCase";

      case kIsFirstUpperCase: return "IsFirstUpperCase";
      case kIsFirstLowerCase: return "IsFirstLowerCase";

      case kIsSameCaseAsAlignedWord: return "IsSameCaseAsAlignedWord";
      case kIsNotSameCaseAsAlignedWord: return "IsNotSameCaseAsAlignedWord";

      case kIsIdenticalToAlignedWord: return "IsIdenticalToAlignedWord";
      case kIsNotIdenticalToAlignedWord: return "IsNotIdenticalToAlignedWord";

      case kIsFullUpperAndSrcIsFullUpper: return "IsFullUpperAndSrcIsFullUpper";
      case kIsNotFullUpperAndSrcIsFullUpper: return "IsNotFullUpperAndSrcIsFullUpper";

      case kLogProbSrcWordGivenTrgWord: return "LogProbSrcWordGivenTrgWord";
      case kLogProbTrgWordGivenSrcWord: return "LogProbTrgWordGivenSrcWord";
    }
  else
    switch (id) {
      case kOCRCChannel: return "OCRCChannel";
      case kOCRCLM: return "OCRCLM";
      case kLmCapitalizeChannel: return "LmCapitalizeChannel";
      case kLmCapitalizeLM: return "LmCapitalizeLm";
    }
  SDL_DEBUG(FeatureIds, "Feature id " << id << " is unknown");
  return kFeature_ + graehl::to_string(id);
}

std::string FeatureIds::describeAlignmentFeatures() {
  return featureIdRangeDescription(kAlignmentBegin, kAlignmentEnd);
}


}}
