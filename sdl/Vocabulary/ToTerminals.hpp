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

    create lexical (kTerminal) Syms from string(s) using IVocabulary.
*/

#ifndef TOTERMINALS_JG_2015_06_18_HPP
#define TOTERMINALS_JG_2015_06_18_HPP
#pragma once

#include <sdl/IVocabulary.hpp>
#include <sdl/Syms.hpp>

namespace sdl {
namespace Vocabulary {


/**
   append space separated sym id integers to syms.
*/
void splitIntegerSyms(std::string const& ids, Syms& syms);

/**
   Map space-separated terminal tokens to Symbols,
       append to the passed-in Syms vector,
       and as a side-effect add novel tokens to dictionary as kTerminal

   \param text space-separated terminal text tokens

   \param[out] syms Symbol vector to be appended to

   \param voc Vocabulary for lookup and insertion. if null then use splitIntegerSyms instead

 */
void splitToTerminals(std::string const& text, Syms& syms, IVocabulary& voc);

inline void splitToTerminals(std::string const& text, Syms& syms, IVocabulary* voc = 0) {
  if (!voc)
    splitIntegerSyms(text, syms);
  else
    splitToTerminals(text, syms, *voc);
}

inline void splitToTerminals(std::string const& text, Syms& syms, IVocabularyPtr const& voc) {
  splitToTerminals(text, syms, voc.get());
}

void mapToTerminals(std::vector<std::string> const& strs, Syms& syms, IVocabulary& vocab);


}}

#endif
