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

    case-changing of parts of words using Icu locale.
*/

#ifndef ICUCASE_JG2012921_HPP
#define ICUCASE_JG2012921_HPP
#pragma once


#include <sdl/Config/Init.hpp>
#include <sdl/Util/Enum.hpp>
#include <sdl/Util/Icu.hpp>
#include <sdl/Util/IcuFirstWordBreakIterator.hpp>
#include <sdl/Util/LWSpecialToken.hpp>
#include <sdl/Sym.hpp>
#include <unicode/ucasemap.h>

namespace sdl {
namespace Util {

/**
   kFoldCase is locale independent for case-insensitive comparison - similar to
   default-locale FullLowerCase?

   from ICU docs:
   Case-folding is locale-independent and not context-sensitive,

   kFirstUpCase is for sentence start (over-under -> Over-under). kTitleCase
   does (over-under -> Over-Under), which is wrong for English sentence
   beginnings

   Title case is used to capitalize the first character of a word such as the
   Latin capital letter 'D' with small letter 'z' ( \u01F2 'Dz'). The term
   "title case" can also be used to refer to words whose first letter is an
   uppercase or title case letter and the rest are lowercase letters. However,
   not all words in the title of a document or first words in a sentence will be
   title case. The use of title case words is language dependent. For example,
   in English, "Taming of the Shrew" would be the appropriate capitalization and
   not "Taming Of The Shrew".

   Sentence start case is title-case on the first word in the token (ICU finds word breaks
   around words conjoined with punctuation, e.g. " - (

*/
SDL_ENUM(IcuCase, 8, (OriginalCase, TitleCase, FullLowerCase, FullUpperCase, SentenceStartCase, FoldCase,
                      FirstLowerCase, FirstUpperCase))

/** for backward semi-compatibility with Utf8Case based LmCapitalize configs, don't use enum names for option
 * names. TODO: change the names when we want to migrate configs*/
inline char const* name(IcuCase icuCase) {
  switch (icuCase) {
    case kOriginalCase: return "identity";
    case kTitleCase: return "title-case";
    case kSentenceStartCase: return "sentence-start-case";
    case kFullUpperCase: return "every-up";
    case kFullLowerCase: return "every-down";
    case kFirstLowerCase: return "first-down";
    case kFirstUpperCase: return "first-up";
    case kFoldCase: return "fold-case";
    default: SDL_THROW_LOG(IcuCase.name, ConfigException, "unsupported IcuCase type=" << (int)icuCase);
  }
}

/** for backward semi-compatibility with Utf8Case based LmCapitalize configs, don't use enum names for option
 * names. */
inline char const* usage(IcuCase icuCase) {
  switch (icuCase) {
    case kOriginalCase: return "original case - e.g. iPhone => iPhone";
    case kTitleCase:
      return "Title Case-first letter upper, rest lower - for each word subtoken e.g. alPHa-BEta => "
             "Alpha-Beta - good for unknown words which are likely proper names";
    case kSentenceStartCase:
      return "Sentence-initial case - like titlecase but only upcase the first letter, e.g. 'loW-Cost' => "
             "'LoW-cost' - intended sentence beginning only, even after e.g. sentence-initial quote or paren "
             "or spanish upside-down exclamation-mark";
    case kFullUpperCase: return "full uppercase - e.g. tHe => THE";
    case kFullLowerCase: return "full lowercase - e.g. USA => usa";
    case kFirstUpperCase: return "first uppercase - e.g. tHe => THe";
    case kFirstLowerCase: return "first lowercase - e.g. USA => sA";
    case kFoldCase: return "locale-insensitive case-folding (probably similar to full lowercase)";
    default: SDL_THROW_LOG(IcuCase.name, ConfigException, "unsupported IcuCase type=" << (int)icuCase);
  }
}

// TODO: hard to debug segfault due to only release mode ICU libs. this approach should be better/faster
struct SentenceStartCase {
  mutable IcuFirstWordBreakIterator breaker;
  explicit SentenceStartCase(Locale const& locale) : breaker(locale) {}
  void operator()(icu::UnicodeString& ustr) const {
    ustr.toTitle(&breaker, breaker.locale, U_TITLECASE_NO_LOWERCASE);
  }
};

/**
   for kSentenceStartCase. Title Case, but change only the first letter
*/

inline void sentenceStartCase(icu::UnicodeString& ustr, Locale const& locale) {
  // SentenceStartCase ssc((locale)); ssc(ustr); // this segfaults
  // ustr.toLower(locale);
  IcuErrorCode status;
  unique_ptr<BreakIterator> pBreaker((BreakIterator::createWordInstance(
      locale, status)));  // pointer, because ICU only supports Java-style OO
  BreakIterator& breaker = *pBreaker;

  int32_t from = 0;
  breaker.setText(ustr);
  for (;;) {
    int32_t to = breaker.next();
    if (to == BreakIterator::DONE) break;
    int32_t wordlen = to - from;
    UnicodeString word(ustr, from, wordlen);
    // SDL_TRACE(IcuCase.sentenceStartCase, "word within " << fromIcu(ustr) << " @UChar #" << to << " word="
    // << fromIcu(word) << " rest=" << fromIcu(ustr.tempSubString(to)));
    // U_TITLECASE_NO_LOWERCASE: Do not lowercase non-initial parts of words when titlecasing.
    word.toTitle(0, locale,
                 U_TITLECASE_NO_LOWERCASE);  // Do not lowercase non-initial parts of words when titlecasing.
    if (ustr.compareCodePointOrder(from, wordlen, word)) {
      ustr.replace(from, wordlen, word);
      break;
    }
  }
}

inline void firstLower(icu::UnicodeString& ustr, Locale const& locale) {
  using namespace icu;
  UErrorCode err = U_ZERO_ERROR;
  BreakIterator* iter = BreakIterator::createCharacterInstance(locale, err);
  assert(U_SUCCESS(err));
  iter->setText(ustr);
  if (iter->next() == BreakIterator::DONE) {
    return;
  }
  ustr.replace(0, iter->current(), ustr.tempSubString(0, iter->current()).toLower(locale));
  delete iter;
}

// TODO: Can share code with firstLower
inline void firstUpper(icu::UnicodeString& ustr, Locale const& locale) {
  using namespace icu;
  UErrorCode err = U_ZERO_ERROR;
  BreakIterator* iter = BreakIterator::createCharacterInstance(locale, err);
  assert(U_SUCCESS(err));
  iter->setText(ustr);
  if (iter->next() == BreakIterator::DONE) {
    return;
  }
  ustr.replace(0, iter->current(), ustr.tempSubString(0, iter->current()).toUpper(locale));
  delete iter;
}

inline void recaseAs(icu::UnicodeString& ustr, IcuCase icuCase, Locale const& loc) {
  switch (icuCase) {
    case kOriginalCase: break;
    case kTitleCase: ustr.toTitle(0, loc); break;
    case kSentenceStartCase: sentenceStartCase(ustr, loc); break;
    case kFullLowerCase: ustr.toLower(loc); break;
    case kFullUpperCase: ustr.toUpper(loc); break;
    case kFoldCase: ustr.foldCase(); break;
    case kFirstLowerCase: firstLower(ustr, loc); break;
    case kFirstUpperCase: firstUpper(ustr, loc); break;
    default: SDL_THROW_LOG(IcuCase.recase, ConfigException, "unsupported IcuCase type=" << (int)icuCase);
  }
}

// TODO: use icu ucasemap_open, ucasemap_utf8FoldCase instead of converting to temp UTF16 UnicodeString-see
// http://icu-project.org/apiref/icu4c/ucasemap_8h.html#a40df152b19d07dba4cc9d5b3015b27d1
inline std::string recasedAsUtf8(std::string const& str, IcuCase icuCase, Locale const& locale) {
  std::string r;
  icu::UnicodeString ustr = toIcu(str);
  recaseAs(ustr, icuCase, locale);
  ustr.toUTF8String(r);
  return r;
}

inline icu::UnicodeString recasedAs(icu::UnicodeString const& ustr, IcuCase icuCase, Locale const& loc) {
  icu::UnicodeString r = ustr;
  recaseAs(r, icuCase, loc);
  return r;
}

inline void lowercase(std::string& str, Locale const& loc) {
  icu::UnicodeString ustr = toIcu(str);
  ustr.toLower(loc);
  ustr.toUTF8String(str);
}

inline std::string lowercased(std::string const& str, Locale const& loc) {
  std::string r;
  icu::UnicodeString ustr = toIcu(str);
  ustr.toLower(loc);
  ustr.toUTF8String(r);
  return r;
}

/** \return ustr is equal to its recasing by icuCase already (under locale).

    unless you have converted your unicode string to a normal form (e.g. NFC or NFD), it may be possible
   (needs testing) that isCase will have false negatives - see
   http://www.unicode.org/reports/tr15/#Canonical_Equivalence-even though ICU's case maps probably respect
   canonical equivalence. this is a standard unicode-string-comparison concern.

    use classifyCase if you want to skip __LW_...__ parts
*/
inline bool isCase(icu::UnicodeString const& ustr, IcuCase icuCase, Locale const& locale) {
  icu::UnicodeString s(ustr);
  recaseAs(s, icuCase, locale);
  return s == ustr;
}

/** \return true iff TitleCase(ustr)!=ustr and FullUpperCase(ustr)!=ustr and FullLowerCase(ustr)!=ustr. */
inline bool isMixedCase(icu::UnicodeString const& ustr, Locale const& loc) {
  return !(isCase(ustr, kFullLowerCase, loc) || isCase(ustr, kFullUpperCase, loc)
           || isCase(ustr, kTitleCase, loc));
}

/** \return true iff ustr is both upper and lower case (e.g. it's completely nonalphabetic) */
inline bool isNoCase(icu::UnicodeString const& ustr, Locale const& loc) {
  return isCase(ustr, kFullLowerCase, loc) && isCase(ustr, kFullUpperCase, loc);
}

/** function object for choosing utf8->utf8 string recasings, with pass-LW and pass-mixed-cased options */
struct IcuCaseSome : IcuLocaleConfig {
  IcuCaseSome() { validate(); }

  bool passMixedCase = false, passLW = true;
  friend void validate(IcuCaseSome& x) { x.validate(); }

  template <class Config>
  void configure(Config& c, bool passMixedCaseDefault = false) {
    IcuLocaleConfig::configure(c);
    c.is("ICU recasing options");
    c("pass-mixed-case", &passMixedCase)(
        "pass unmodified any mixed-case words like 'iPhone', that are neither Titlecase nor ALLCAPS nor "
        "lowercase. if passLW, then the case of the __LW...__ is ignored, and each word chunk is treated "
        "separately")
        .init(passMixedCaseDefault);
    c("pass-LW", &passLW)(
        "pass __LW_...__ portions of words unmodified (the parts between are recased as if individual words "
        "- e.g. __LW_NL__ for newlines may occur mid-token!)")
        .defaulted();
  }

  /**
     like recasedAsUtf8, but respects the pass-LW and pass-mixed-case options
  */
  std::string recased(std::string const& str, IcuCase icuCase) const {
    if (icuCase == kOriginalCase) return str;
    return passLW ? recasedPassLW(str, icuCase) : recasedChunk(str, icuCase);
  }

  /**
     effectively splits on __LW_.__ special regions
     and recases what's between. this can't be in-place because #bytes may
     change on recasing. TODO: use C api utf8->utf8 icu methods for recasing. in that
  */
  std::string recasedPassLW(std::string const& str, IcuCase icuCase) const {
    std::string result;
    typedef std::string::size_type Pos;
    Pos specialStarts = 0, specialEnds, len = str.size(), lastEnds = 0;
    result.reserve(
        len + 2);  // +2 in case recasing alters number of utf8 bytes a little. little harm done either way
    while ((specialEnds = nextLWends(str, specialStarts)) != std::string::npos) {
      if (specialStarts > lastEnds) {
        appendRecasedChunk(str, lastEnds, specialStarts - lastEnds, icuCase, result);
        if (icuCase == kSentenceStartCase)
          icuCase = kFullLowerCase;  // because we already sentence cased the first word.
      }
      result.append(str, specialStarts, specialEnds - specialStarts);
      specialStarts = lastEnds = specialEnds;
    }
    if (lastEnds == 0) return recasedChunk(str, icuCase);
    if (lastEnds != len) appendRecasedChunk(str, lastEnds, len - lastEnds, icuCase, result);
    return result;
  }

  /** in/out are utf8 without any __LW_.__. respects pass-mixed-case. */
  void appendRecasedChunk(std::string const& str, std::string::size_type start, std::string::size_type len,
                          IcuCase icuCase, std::string& result) const {
    if (icuCase == kOriginalCase)
      result.append(str, start, len);
    else {
      icu::UnicodeString ustr;
      toIcu(ustr, str.begin() + start, str.begin() + start + len);
      if (maybeRecase(ustr, icuCase))
        appendFromIcu(ustr, result);
      else
        result.append(str);
    }
  }

  /** in/out are utf8. respects pass-mixed-case */
  std::string recasedChunk(std::string const& str, IcuCase icuCase) const {
    if (icuCase == kOriginalCase) return str;
    icu::UnicodeString ustr = toIcu(str);
    return maybeRecase(ustr, icuCase) ? fromIcu(ustr) : str;
  }

  void lowercaseFull(std::string& str) { lowercase(str, locale()); }

  std::string lowercasedFull(std::string const& str) { return lowercased(str, locale()); }

  /**
     respects pass-mixed-case.
  */
  bool maybeRecase(icu::UnicodeString& ustr, IcuCase icuCase) const {
    if (passMixedCase && isMixedCase(ustr, locale())) return false;
    recaseAs(ustr, icuCase, locale());
    return true;
  }
};

/** function object for a particular utf8->utf8 string recasing, with pass-LW
    and pass-mixed-cased options.

    Leaves __LW_AT__ and similar markers untouched.
*/
struct IcuCaseAs : IcuCaseSome {
  typedef std::string result_type;
  IcuCase icuCase = kOriginalCase;
  bool equalToRecasedSecond(std::string const& alreadyLowerCase, std::string const& stringToLowercase) const {
    return alreadyLowerCase == recased(stringToLowercase, icuCase);
  }
  bool equalWhenRecased(std::string const& a, std::string const& b) const {
    return recased(a, icuCase) == recased(b, icuCase);
  }
  void setLowercase() { icuCase = kFullLowerCase; }
  explicit IcuCaseAs(IcuCase icuCase = kOriginalCase) : icuCase(icuCase) { validate(); }
  friend void validate(IcuCaseAs& x) { x.validate(); }
  template <class Config>
  void configure(Config& c, bool passMixedCaseDefault = false, IcuCase defaultCase = kOriginalCase) {
    IcuCaseSome::configure(c, passMixedCaseDefault);
    c.is("ICU recasing method");
    c("icu-case", &icuCase)("which ICU recasing method (behavior may depend on locale)").init(defaultCase);
  }
  /** in/out are utf8. */
  std::string operator()(std::string const& str) const { return recased(str, icuCase); }

  bool modify(std::string& str) const {
    str = (*this)(str);
    return true;
  }

  /**
     adds recased symbol as terminal.
  */
  template <class Vocab>
  Sym operator()(std::string const& str, Vocab& vocab) const {
    return vocab.add(recased(str, icuCase), kTerminal);
  }

  /**
     adds recased symbol as terminal.
  */
  template <class Vocab>
  Sym operator()(Sym sym, Vocab& vocab) const {
    assert(sym.isTerminal());
    return operator()(vocab.str(sym), vocab);
  }

  bool enabled() const { return icuCase != kOriginalCase; }
};

/** function object for utf8->utf8 string recasing. */
struct IcuCaseAlways {
  typedef std::string result_type;
  IcuCase icuCase;
  Locale const& locale;
  IcuCaseAlways(IcuCase icuCase, Locale const& locale) : icuCase(icuCase), locale(locale) {}
  std::string operator()(std::string const& str) const { return recasedAsUtf8(str, icuCase, locale); }
};


}}

#endif
