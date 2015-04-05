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

    __LW_AT__ (Vocabulary::kGlueAffix) appearing at beginning or end of tokens means the
    detokenizer joins them together with no space (and with no __LW_AT__
*/

#ifndef GLUE_JG_2014_07_08_HPP
#define GLUE_JG_2014_07_08_HPP
#pragma once

#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/String.hpp>

namespace sdl { namespace Vocabulary {

std::string const kGlueAffix("__LW_AT__");

inline void removeGlueAffix(std::string &token) {
  Util::stripSuffix(token, kGlueAffix);
  Util::stripPrefix(token, kGlueAffix);
}

inline std::string removeGlueAffixCopy(std::string token) {
  removeGlueAffix(token);
  return token;
}

//TODO (for MaltDependencyParser): in-place removeglueaffix on space-split word
//substrings (alternative: make string from hg with a token-by-token transform
//fn)


}}

#endif
