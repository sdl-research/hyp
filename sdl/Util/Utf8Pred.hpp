// Copyright 2014 SDL plc
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

    primitive unicode codepoint language id for heuristics joining e.g. chinese
    and non-chinese tokens.
*/

#ifndef SDL_LWUTIL__UTF8PRED_HPP
#define SDL_LWUTIL__UTF8PRED_HPP
#pragma once

#include <functional>
#include <cstdlib>
#include <string>
#include <sdl/Util/Icu.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Util {

namespace {

inline bool stringInCharSet(std::string const& str, icu::UnicodeSet const& charSet) {
  using namespace icu;
  bool in_char_set = true;
  UnicodeString u_str = UnicodeString::fromUTF8(str);
  StringCharacterIterator char_iter(u_str);
  while (char_iter.hasNext()) {
    UChar32 codepoint = char_iter.next32PostInc();
    if (!charSet.contains(codepoint)) {
      in_char_set = false;
      break;
    }
  }
  return in_char_set;
}
}

/**
   Chinese character predicate

   see http://www.khngai.com/chinese/charmap/tbluni.php?page=0
   for a reference of unicode codepoints for Chinese characters
   see also unicode codepoints map for punctuations
 */
struct ChineseCharPred : std::unary_function<std::string, bool> {
  ChineseCharPred(bool add_symbols = true) {
    using namespace icu;
    chi_char.addAll(UnicodeSet(0x4E00, 0x9FA5));  // CJK characters
    if (add_symbols) {
      chi_char.addAll(UnicodeSet(0xFF01, 0xFF1F));  // fullwidth Chinese punctuations
      chi_char.addAll(UnicodeSet(0x3000, 0x303F));  // other symbols , punctuations
      chi_char.addAll(UnicodeSet(0x2000, 0x27FF));  // symbols, including common punctuations
    }
  }
  bool operator()(const std::string& str) const { return stringInCharSet(str, chi_char); }

 private:
  icu::UnicodeSet chi_char;
};

/* utf8 ranges predictor for detokenization

   characters in those ranges should be put together without any spaces, example ranges,

   UnicodeSet(0x3200, 0x9FFF) // Enclosed CJK Letters and Months,  CJK Compatibility, CJK Unified idographs,
   and extension.
   UnicodeSet(0xF900, 0xFAFF) // CJK Compatibility ideographs
   UnicodeSet(0xFE30, 0xFE4F) // CJK Compatibility forms
   UnicodeSet(0xFF01, 0xFF1F) // fullwidth Chinese punctuations
   UnicodeSet(0x2E80, 0x2EFF) // CJK Radicals supplement
   UnicodeSet(0x3000, 0x303F) // other symbols , punctuations
   UnicodeSet(0x2000, 0x27FF) // symbols, including common punctuations
   UnicodeSet(0x0E00, 0x0E7F) // Thai language
   UnicodeSet(0x31C0, 0x31EF) // CJK strokes
   UnicodeSet(0x20000, 0x2FA1F) // CJK extensions
 */

struct Utf8RangePred : std::unary_function<std::string, bool> {
  typedef std::string UcharRange;
  Utf8RangePred(std::map<std::string, UcharRange> const& ofRanges) {
    using namespace icu;
    typedef std::pair<std::string, UcharRange> NameAndRange;
    forall (NameAndRange const& range, ofRanges) {
      UcharRange const& rangeStr = range.second;
      std::size_t sep = rangeStr.find('-');
      if (sep == std::string::npos) {
        SDL_THROW_LOG(Util.Utf8Pred, ConfigException,
                      "No space ranges must be in the format of {start-utf8-codepoint}-{end-utf8-codepoint}");
      }
      // use std::strtoul to allow hex as well as decimal
      noSpaceSet.addAll(UnicodeSet(std::strtoul(rangeStr.substr(0, sep).c_str(), 0, 0),
                                   std::strtoul(rangeStr.substr(sep + 1).c_str(), 0, 0)));
    }
  }
  bool operator()(std::string const& str) const { return stringInCharSet(str, noSpaceSet); }

 private:
  icu::UnicodeSet noSpaceSet;
};


}}

#endif
