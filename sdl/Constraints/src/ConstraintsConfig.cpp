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

    .
*/

#include <sdl/Constraints/ConstraintsConfig.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/AsciiCase.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Util/PrintRange.hpp>

namespace sdl {

std::string const kNoSwapLc("noswap");

void EntityConstraintConfig::validate() {
  if (!valid())
    SDL_WARN(Constraint, "span [" << span.first << ", " << span.second
                                  << ") is empty - expect unicode codepoint span: half-open [start, end), "
                                     "i.e. codepoints at 0-based index start <= i < end are kept together "
                                     "and optionally replaced by decoder-output and final-output");
  if (isFormat && hasOutput()) {
    SDL_WARN(Constraint, "can't is-format an output constraint; clearing is-format");
    isFormat = false;
  }
  if (protect) protectFinal = protectDecoder = true;
}

void ConstraintsConfig::clear() {
  EntityConstraintConfigs::clear();
  forceDecode.clear();
  prefixForceDecode.clear();
  lmPrefix.clear();
  walls.clear();
}

void ConstraintsConfig::validate() {
  bool force = !forceDecode.empty(), prefix = !prefixForceDecode.empty(), lmprefix = !lmPrefix.empty();
  unsigned noptions = force + prefix + lmprefix;
  if (noptions > 1)
    SDL_THROW_LOG(
        Constraint, ConfigException,
        "don't specify more than one of force-decode, prefix-force-decode, and lm-prefix - you specified "
            << noptions << " of these.");
  for (EntityConstraintConfigs::iterator i = begin(), e = end(); i != e; ++i)
    i->validate();  // maybe config lib did this already
}

void ConstraintsConfig::print(std::ostream& out) const {
  out << "{constraints config:";
  char const sp = ' ';
  if (!walls.empty()) out << sp << "walls: " << Util::makePrintable(walls);
  if (!forceDecode.empty()) out << sp << "force-decode: " << forceDecode;
  if (!prefixForceDecode.empty()) out << sp << "prefix-force-decode: " << prefixForceDecode;
  if (!lmPrefix.empty()) out << sp << "lm-prefix: " << lmPrefix;
  if (!empty()) out << sp << Util::print(*this, Util::multiline());
  out << "}";
}
}
