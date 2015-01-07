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

    ICU (unicode) helpers.
*/

#ifndef ICU_JG201276_HPP
#define ICU_JG201276_HPP
#pragma once

#include <sdl/Util/IcuHeaders.hpp>
#include <sdl/Util/StringBuilder.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/String32.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Exception.hpp>

#include <vector>
#include <boost/optional.hpp>
#include <locale>
#include <locale.h>

namespace sdl {

using icu::UnicodeString;
using icu::Locale;
//UChar32 is in global ns, in machine.h. is int32_t
typedef shared_ptr<Locale const> LocalePtr;

namespace Util {

/** \param icuLanguageName empty string = default, else two-letter or three-letter ISO-639 code. This parameter can instead be an ICU style C locale (e.g. "en_US"), but the other parameters must not be used.

    \param icuCountryCode 2-letter country code (e.g. US). \param icuLocaleName if empty, return default

*/

/**
   e.g. Utf8Chars = vector<char> or string. appends utf8 to vec.
*/


template <class Utf8Chars>
inline void appendFromIcu(UnicodeString const& ustr, Utf8Chars &vec)
{
  UChar const* begin = ustr.getBuffer();
  appendUtf8From16(vec, begin, begin+ustr.length());
}

/**
   e.g. Utf8Chars = vector<char> or string. sets ustr to vec utf8->UnicodeString

   (Illegal input is replaced with U+FFFD)
*/
template <class Utf8Buf>
inline void toIcu(UnicodeString & ustr, Utf8Buf const& vec)
{
  UChar *out = ustr.getBuffer((int)vec.size()); // ascii upper bound on # of UTF16 code points needed
  ustr.releaseBuffer(utf8::unchecked::utf8to16(vec.begin(), vec.end(), out)-out);
}

template <class Utf8Iter>
inline void toIcu(UnicodeString & ustr, Utf8Iter begin, Utf8Iter end)
{
  int maxSize = (int)(end-begin); // ascii upper bound on # of UTF16 code points needed
  UChar *out = ustr.getBuffer(maxSize);
  int actualSize = (int)(utf8::unchecked::utf8to16(begin, end, out)-out);
  ustr.releaseBuffer(actualSize);
}

namespace {
unsigned const kIcuErrorInReplacing(-1);
}

/// like toIcu, set ustr to utf8 but return the number of U+FFEE replacements performed. \return kIcuErrorInReplacing (max unsigned)
unsigned toIcuReplacing(UnicodeString & ustr, icu::StringPiece utf8);

///(Illegal input is replaced with U+FFFD)
inline icu::UnicodeString toIcu(std::string const& s) {
  return UnicodeString::fromUTF8(s);
}

inline icu::UnicodeString toIcu(std::string const& s, std::string::size_type start, int32_t len) {
  return UnicodeString::fromUTF8(icu::StringPiece(&s[0]+start, len));
}

inline std::size_t getNumGraphemes(const UnicodeString& ustr) {
  using namespace icu;

  UErrorCode err = U_ZERO_ERROR;
  char* locale = ::setlocale(LC_ALL, NULL);
  BreakIterator* iter =
      BreakIterator::createCharacterInstance(locale, err);

  assert(U_SUCCESS(err));
  iter->setText(ustr);

  std::size_t count = 0;
  while (iter->next() != BreakIterator::DONE) {
    ++count;
  }
  delete iter;
  return count;
}

inline std::size_t getNumGraphemes(std::string const& s) {
  return getNumGraphemes(toIcu(s));
}

namespace {
/**
   From ICU docs:
   // When converting from UTF-16 to UTF-8, the result will have at most 3 times
   // as many bytes as the source has UChars.
   // The "worst cases" are writing systems like Indic, Thai and CJK with
   // 3:1 bytes:UChars.

   */
const int32_t maxUtf8PerUtf16 = 3;
}

/**
   \return # of bytes copied to bytesOut (<= bytesCapacity) from str[startUChar]....
*/
inline int32_t fromIcu(icu::UnicodeString const& str
                       , char *bytesOut, int32_t bytesCapacity
                       , int32_t startUChar, int32_t lenUChar, UChar32 invalidCharReplacement = 0xFFFD)
{
  using namespace icu;
  UErrorCode errorCode = U_ZERO_ERROR;
  int32_t bytesCopied;
  u_strToUTF8WithSub(bytesOut, bytesCapacity, &bytesCopied,
                     str.getBuffer() + startUChar, lenUChar, invalidCharReplacement, 0, &errorCode);
  return bytesCopied;
}

/**
   quickly convert a utf-16 substring from str[startUChar] ... str[startUChar+lenUChar] to a char vector or std::string

   \param out a ByteVector = string or vector<char> - something with resize and contiguous array storage starting at &out[0]
*/
template <class ByteVector>
inline void fromIcu(icu::UnicodeString const& str, ByteVector &out, int32_t startUChar, int32_t lenUChar) {
  int32_t bytes = maxUtf8PerUtf16*lenUChar;
  out.resize(bytes);
  out.resize(fromIcu(str, &out[0], bytes, startUChar, lenUChar));
}

template <class ByteVector>
inline void fromIcu(icu::UnicodeString const& str, ByteVector &out) {
  fromIcu(str, out, 0, str.length());
}

inline std::string fromIcu(icu::UnicodeString const& str, int32_t startUChar, int32_t lenUChar) {
  std::string r;
  fromIcu(str, r, startUChar, lenUChar);
  return r;
}

struct FromIcu : std::string {
  explicit FromIcu(icu::UnicodeString const& str) {
    fromIcu(str, *this);
  }
};

inline std::string fromIcu(icu::UnicodeString const& str) {
  //s.toUTF8String(r); // same result as this but faster
  std::string r;
  fromIcu(str, r);
  return r;
}

inline std::string fromIcu(UChar *cstr) {
  return fromIcu(icu::UnicodeString(cstr));
}


/// Nfc : Unicode normal form composed - "Normalization Form C (NFC) Canonical
/// Decomposition, followed by Canonical Composition"
bool isNfc(const UnicodeString& s);

inline void warnNotNfc(const UnicodeString& us) {
  SDL_WARN(Icu.Nfc, fromIcu(us) << " (UnicodeString) is not normal-form-composed (NFC)");
}

inline void warnNotNfc(std::string const& utf8) {
  SDL_WARN(Icu.Nfc, "'" << utf8 << "' (utf8) is not normal-form-composed (NFC)");
}

inline void warnNotNfc(std::string const& utf8, unsigned replaced) {
  if (replaced)
    SDL_WARN(Icu.Nfc,
             "'" << utf8 << "' (utf8 with " << replaced << " invalid codepoints) is not normal-form-composed (NFC)");
  else
    warnNotNfc(utf8);
}

/// \param[out] buf <- (s -> nfc) and wasNfc <- false, returning buf;
/// or else wasNfc <- true, returning s (Illegal input is replaced with U+FFFD)
UnicodeString const& maybeNormalizeToNfc(UnicodeString const& s,
                                         UnicodeString &buf,
                                         bool &wasNfc,
                                         bool warnIfNotNfc = false);

inline UnicodeString maybeNormalizeToNfc(const UnicodeString& s, bool &wasNfc, bool warnIfNotNfc = false) {
  UnicodeString buf;
  return maybeNormalizeToNfc(s, buf, wasNfc, warnIfNotNfc);
}

inline UnicodeString normalizeToNfc(const UnicodeString& s, bool warnIfNotNfc = false) {
  bool wasNfc;
  return maybeNormalizeToNfc(s, wasNfc, warnIfNotNfc);
}

inline bool isNfcWarn(UnicodeString const& us) {
  bool const r = isNfc(us);
  if (!r)
    warnNotNfc(us);
  return r;
}

std::string parseErrorString(UParseError const& e); /// ICU lacks a strerror-like for parse errors

VERBOSE_EXCEPTION_DECLARE(IcuException)

std::string icuLanguagesList(const char* sep=", ");
std::string icuCountriesList(const char* sep=", ");

inline std::string localeName(Locale const& locale)
{
  UnicodeString r;
  return fromIcu(locale.getDisplayName(r));
}

inline LocalePtr getLocalePtr(std::string const& icuLanguageName="en", std::string const& icuCountryCode="")
{
  if (icuLanguageName.empty())
    return LocalePtr(&Locale::getDefault(), DoNothing());
  char const* country = icuCountryCode.empty() ? 0 : icuCountryCode.c_str();
  if (icuLanguageName=="en_US")
    return LocalePtr(&Locale::getUS(), DoNothing());
  if (icuLanguageName=="en_GB") // bad ISO. should be UK
    return LocalePtr(&Locale::getUK(), DoNothing());
  LocalePtr r((new Locale(icuLanguageName.c_str(), country)));
  std::string describeArgs="locale config: language(ISO-639)="+icuLanguageName;
  if (country)
    describeArgs+=" country(ISO-3166)="+icuCountryCode;
  if (r->isBogus())
    SDL_THROW_LOG(ICU.Locale, ConfigException, "got bogus ICU Locale for " << describeArgs);
  std::string isoLanguage = r->getISO3Language();
  if (isoLanguage.empty())
    SDL_THROW_LOG(ICU.Locale, ConfigException, "ICU Locale doesn't know language: " << describeArgs);
  SDL_INFO(ICU.Locale, "Found ICU Locale " << localeName(*r) << " for specification " << describeArgs);
  return r;
}

struct IcuLocaleConfig
{
  LocalePtr pLocale;
  Locale const& locale() const { return *pLocale; }
  std::string language, country;
  friend inline std::string to_string_impl(IcuLocaleConfig const& x) { return x.to_string_impl(); }
  std::string to_string_impl() const {
    return "language='"+language+"' country='"+country+"'";
  }
  friend void validate(IcuLocaleConfig &x) { x.validate(); }
  void validate()
  {
    if (language.empty() && !country.empty())
      SDL_THROW_LOG(IcuLocaleConfig, ConfigException, "don't specify country without specifying language: " << to_string_impl());
    if (language.size()>3 && !country.empty())
      SDL_THROW_LOG(IcuLocaleConfig, ConfigException, "don't specify a language_country (e.g. en_US) --language while also specifying --country: " << to_string_impl());
    pLocale = getLocalePtr(language, country);
  }
  template <class Config>
  void configure(Config &c)
  {
    c.is("ICU locale");
    c("language", &language)("ICU language 2-3 letter ISO code, or language_country e.g. en_US. empty means icu (OS?) default. valid choices: "+icuLanguagesList());
    c("country", &country)("ICU country 2 letter ISO code. may be empty. valid choices: "+icuCountriesList());
    //TODO: region? script? probably unnecessary for our LPs
  }
};

struct IcuErrorCode : icu::ErrorCode
{
  boost::optional<UParseError> optParse;
  std::string detail, prefix;
  bool assertSuccessOnDestroy;
  IcuErrorCode(std::string const& detail="ERROR", std::string const& prefix="sdl.IcuErrorCode", bool assertSuccessOnDestroy = true)
      : detail(detail), prefix(prefix)
      , assertSuccessOnDestroy(assertSuccessOnDestroy)
  {}
  ~IcuErrorCode() {
    if (assertSuccessOnDestroy)
      this->assertSuccess();
  }
  UParseError &storeParseError(); // pass this to ICU methods that write a parse error
  std::string parseError() const;
  void fail()
  {
    handleFailure();
  }

 protected:
  virtual void handleFailure() const
  {
    THROW_LOG_NAMESTR(prefix, IcuException, detail << ": " << this->errorName() << parseError());
  }
};

void globalIcuData(std::string pathString);  /// must be called before using any ICU APIs from multiple threads, and before u_init() if you use it

// returns a PATH-format string (unix: /bin:/usr/bin , windows: c:/bin;c:/usr/bin )
std::string platformPathString(std::vector<std::string> const& dirs);

inline void globalIcuData(std::vector<std::string> const& dirnames)
{
  globalIcuData(platformPathString(dirnames));
}

///singletons (don't delete)
icu::Normalizer2 const* icuNFC(); // compose: turn base char + accents into a single unicode point if applicable
icu::Normalizer2 const* icuNFD(); // decompose: expand e.g. accents so they follow base letter (if >1, in a canonical order). inverse of compose
///K means "compatability" - characters that could be represented as two unicode glyphs, but due to historical use, are a single glyph (e.g. some ligatures)
icu::Normalizer2 const* icuNFKC(); /// as NFC but creating Compatability glyphs
icu::Normalizer2 const* icuNFKD(); /// as NFD but decomposing Compatability glyphs. inverse of NFKC


}}

#endif
