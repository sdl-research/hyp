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
#include <sdl/Util/Delete.hpp>
#include <sdl/Types.hpp>

namespace sdl {

/// note: actual icu namespace is icu_NN e.g. icu_50
using icu::StringPiece;
using icu::UnicodeString;
using icu::Locale;
// UChar32 is in global ns, in machine.h. is int32_t
typedef shared_ptr<Locale const> LocalePtr;

inline icu::StringPiece icuStringPiece(Slice s) {
  return icu::StringPiece(s.first, s.second - s.first);
}

inline icu::StringPiece icuStringPiece(std::string const& s) {
  return s;
}

inline Slice toSlice(icu::StringPiece const& s) {
  char const* data = s.data();
  return Slice(data, data + s.size());
}

namespace Util {

typedef icu::Normalizer2 IcuNormalizer2;
typedef IcuNormalizer2 const* IcuNormalizer2Ptr;
/**
   compose: turn base char + accents into a single unicode point if

   decompose: expand e.g. accents so they follow base letter (if >1, in
   canonical order). inverse of compose

   K means "compatability" - characters that could be represented as two unicode
   glyphs, but due to historical use, are a single glyph (e.g. some ligatures)
   (destructive many:1)

   these may be used only after static init
 */
extern IcuNormalizer2Ptr gNfc, gNfkc, gNfd, gNfkd;

/// not ok for s == out. does NOT assign to out if s is already normalized.
bool maybeIcuNormalize(icu::UnicodeString const& s, icu::UnicodeString& out, IcuNormalizer2Ptr normalize);


/// checks exactly whether s was normalized. normalizes into out if it wasn't. this is slower than
/// maybeIcuNormalize but has the same result except the return value is strictly reflecting the normalization
/// status of s.
bool neededIcuNormalize(icu::UnicodeString const& s, icu::UnicodeString& out, IcuNormalizer2Ptr normalize);

/// ok for utf8 == out since we make a copy. does NOT assign to out if utf8 is already normalized.
bool maybeIcuNormalize(icu::StringPiece utf8, std::string& out, IcuNormalizer2Ptr normalize);

/// \return whether out was assigned to (w/ normalized utf8). if utf8 was already normalized (false) then out
/// is not modified
inline bool maybeIcuNormalize(Slice utf8, std::string& out, IcuNormalizer2Ptr normalize) {
  return maybeIcuNormalize(icuStringPiece(utf8), out, normalize);
}

/// \return whether out was assigned to (w/ normalized utf8). if utf8 was already normalized (false) then out
/// is not modified
inline bool maybeIcuNormalize(std::string const& in, std::string& out, IcuNormalizer2Ptr normalize) {
  return maybeIcuNormalize(icu::StringPiece(in), out, normalize);
}

/// \return normalize and return string
inline std::string& icuNormalize(std::string& out, IcuNormalizer2Ptr normalize) {
  maybeIcuNormalize(out, out, normalize);
  return out;
}

/** \param icuLanguageName empty string = default, else two-letter or three-letter ISO-639 code. This
   parameter can instead be an ICU style C locale (e.g. "en_US"), but the other parameters must not be used.

    \param icuCountryCode 2-letter country code (e.g. US). \param icuLocaleName if empty, return default

*/

/**
   e.g. Utf8Chars = vector<char> or string. appends utf8 to vec.
*/


template <class Utf8Chars>
inline void appendFromIcu(UnicodeString const& ustr, Utf8Chars& vec) {
  UChar const* begin = ustr.getBuffer();
  appendUtf8From16(vec, begin, begin + ustr.length());
}

/**
   e.g. Utf8Chars = vector<char> or string. sets ustr to vec utf8->UnicodeString

   (Illegal input is replaced with U+FFFD)
*/
template <class Utf8Buf>
inline void toIcu(UnicodeString& ustr, Utf8Buf const& vec) {
  UChar* out = ustr.getBuffer((int)vec.size());  // ascii upper bound on # of UTF16 code points needed
  ustr.releaseBuffer(utf8::unchecked::utf8to16(vec.begin(), vec.end(), out) - out);
}

template <class Utf8Iter>
inline void toIcu(UnicodeString& ustr, Utf8Iter begin, Utf8Iter end) {
  int maxSize = (int)(end - begin);  // ascii upper bound on # of UTF16 code points needed
  UChar* out = ustr.getBuffer(maxSize);
  int actualSize = (int)(utf8::unchecked::utf8to16(begin, end, out) - out);
  ustr.releaseBuffer(actualSize);
}

namespace {
unsigned const kIcuErrorInReplacing(-1);
}

/// like toIcu, set ustr to utf8 but return the number of U+FFEE replacements performed. \return
/// kIcuErrorInReplacing (max unsigned)
unsigned toIcuReplacing(UnicodeString& ustr, icu::StringPiece utf8);

///(Illegal input is replaced with U+FFFD)
inline icu::UnicodeString toIcu(std::string const& s) {
  return icu::UnicodeString::fromUTF8(s);
}

inline icu::UnicodeString toIcu(std::string const& s, std::string::size_type start, int32_t len) {
  return icu::UnicodeString::fromUTF8(icu::StringPiece(&s[0] + start, len));
}

inline std::size_t getNumGraphemes(icu::UnicodeString const& ustr) {
  using namespace icu;
  UErrorCode err = U_ZERO_ERROR;
  char* locale = ::setlocale(LC_ALL, NULL);
  Util::AutoDelete<BreakIterator> iter(BreakIterator::createCharacterInstance(locale, err));
  assert(U_SUCCESS(err));
  iter->setText(ustr);

  std::size_t count = 0;
  while (iter->next() != BreakIterator::DONE) {
    ++count;
  }
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
inline int32_t fromIcu(icu::UnicodeString const& str, char* bytesOut, int32_t bytesCapacity,
                       int32_t startUChar, int32_t lenUChar, UChar32 invalidCharReplacement = 0xFFFD) {
  using namespace icu;
  UErrorCode errorCode = U_ZERO_ERROR;
  int32_t bytesCopied;
  u_strToUTF8WithSub(bytesOut, bytesCapacity, &bytesCopied, str.getBuffer() + startUChar, lenUChar,
                     invalidCharReplacement, 0, &errorCode);
  return bytesCopied;
}

/**
   quickly convert a utf-16 substring from str[startUChar] ... str[startUChar+lenUChar] to a char vector or
   std::string

   \param out a ByteVector = string or vector<char> - something with resize and contiguous array storage
   starting at &out[0]
*/
template <class ByteVector>
inline void fromIcu(icu::UnicodeString const& str, ByteVector& out, int32_t startUChar, int32_t lenUChar) {
  int32_t bytes = maxUtf8PerUtf16 * lenUChar;
  out.resize(bytes);
  out.resize(fromIcu(str, &out[0], bytes, startUChar, lenUChar));
}

template <class ByteVector>
inline void fromIcu(icu::UnicodeString const& str, ByteVector& out) {
  fromIcu(str, out, 0, str.length());
}

inline std::string fromIcu(icu::UnicodeString const& str, int32_t startUChar, int32_t lenUChar) {
  std::string r;
  fromIcu(str, r, startUChar, lenUChar);
  return r;
}

struct FromIcu : std::string {
  explicit FromIcu(icu::UnicodeString const& str) { fromIcu(str, *this); }
};

inline std::string fromIcu(icu::UnicodeString const& str) {
  // s.toUTF8String(r); // same result as this but faster
  std::string r;
  fromIcu(str, r);
  return r;
}

inline std::string fromIcu(UChar* cstr) {
  return fromIcu(icu::UnicodeString(cstr));
}


/// Nfc : Unicode normal form composed - "Normalization Form C (NFC) Canonical
/// Decomposition, followed by Canonical Composition"
bool isNfc(icu::UnicodeString const& s, bool K = false);

inline bool isNfc(icu::StringPiece const& s, bool K) {
  return isNfc(icu::UnicodeString::fromUTF8(s), K);
}

inline bool isNfc(Slice utf8, bool nfkc = false) {
  return isNfc(icu::StringPiece(utf8.first, len(utf8)), nfkc);
}

inline bool isNfc(std::string const& utf8, bool nfkc = false) {
  return isNfc(icu::StringPiece(utf8), nfkc);
}

inline void warnUnnormalized(icu::UnicodeString const& us, char const* normalization) {
  SDL_WARN(Icu.Nfc, fromIcu(us) << " (UnicodeString) is not " << normalization);
}

inline void warnNotNfc(icu::UnicodeString const& us) {
  SDL_WARN(Icu.Nfc, fromIcu(us) << " (UnicodeString) is not normal-form-composed (NFC)");
}

template <class S>
inline void warnNotNfc(S const& utf8) {
  SDL_WARN(Icu.Nfc, "'" << utf8 << "' (utf8) is not normal-form-composed (NFC)");
}

inline void warnNotNfc(icu::StringPiece const& utf8) {
  SDL_WARN(Icu.Nfc, "'" << toSlice(utf8) << "' (utf8) is not normal-form-composed (NFC)");
}

inline bool maybeIcuNormalizeWarn(icu::UnicodeString const& s, icu::UnicodeString& out,
                                  IcuNormalizer2Ptr norm, bool warnIfUnnormalized,
                                  char const* normalizationName = "normal-form-composed (NFC)") {
  if (warnIfUnnormalized) {
    if (neededIcuNormalize(s, out, norm)) {
      warnUnnormalized(s, normalizationName);
      return true;
    } else
      return false;
  } else
    return maybeIcuNormalize(s, out, norm);
}

/// \param[out] buf <- (s -> nfc) and wasNfc <- false, returning buf;
/// or else wasNfc <- true, returning s (Illegal input is replaced with U+FFFD)
inline UnicodeString const& maybeNormalizedToNfc(UnicodeString const& s, UnicodeString& buf, bool& wasNfc,
                                                 bool warnIfNotNfc = false, bool K = false) {

  return maybeIcuNormalizeWarn(s, buf, K ? gNfkc : gNfc, warnIfNotNfc) ? buf : s;
}

inline UnicodeString maybeNormalizedToNfc(icu::UnicodeString const& s, bool& wasNfc,
                                          bool warnIfNotNfc = false, bool K = false) {
  UnicodeString buf;
  return maybeNormalizedToNfc(s, buf, wasNfc, warnIfNotNfc, K);
}

inline UnicodeString normalizedToNfc(icu::UnicodeString const& s, bool warnIfNotNfc = false, bool K = false) {
  bool wasNfc;
  return maybeNormalizedToNfc(s, wasNfc, warnIfNotNfc, K);
}

inline bool isNfcWarn(UnicodeString const& us, bool K = false) {
  bool const r = isNfc(us, K);
  if (!r) warnNotNfc(us);
  return r;
}

inline bool isNfcWarn(icu::StringPiece const& us, bool K = false) {
  bool const r = isNfc(us, K);
  if (!r) warnNotNfc(us);
  return r;
}

inline bool isNfcWarn(std::string const& s, bool K = false) {
  return isNfcWarn(icu::StringPiece(s), K);
}

inline bool isNfcWarn(Slice utf8, bool K = false) {
  return isNfcWarn(icuStringPiece(utf8), K);
}

std::string parseErrorString(UParseError const& e);  /// ICU lacks a strerror-like for parse errors

VERBOSE_EXCEPTION_DECLARE(IcuException)

std::string icuLanguagesList(const char* sep = ", ");
std::string icuCountriesList(const char* sep = ", ");

inline std::string localeName(Locale const& locale) {
  UnicodeString r;
  return fromIcu(locale.getDisplayName(r));
}

inline LocalePtr getLocalePtr(std::string const& icuLanguageName = "en",
                              std::string const& icuCountryCode = "") {
  if (icuLanguageName.empty()) return LocalePtr(&Locale::getDefault(), DoNothing());
  char const* country = icuCountryCode.empty() ? 0 : icuCountryCode.c_str();
  if (icuLanguageName == "en_US") return LocalePtr(&Locale::getUS(), DoNothing());
  if (icuLanguageName == "en_GB")  // bad ISO. should be UK
    return LocalePtr(&Locale::getUK(), DoNothing());
  LocalePtr r((new Locale(icuLanguageName.c_str(), country)));
  std::string describeArgs("locale config: language(ISO-639)=");
  describeArgs += icuLanguageName;
  if (country) {
    describeArgs += " country(ISO-3166)=";
    describeArgs += icuCountryCode;
  }
  if (r->isBogus()) SDL_THROW_LOG(ICU.Locale, ConfigException, "got bogus ICU Locale for " << describeArgs);
  std::string isoLanguage = r->getISO3Language();
  if (isoLanguage.empty())
    SDL_THROW_LOG(ICU.Locale, ConfigException, "ICU Locale doesn't know language: " << describeArgs);
  SDL_INFO(ICU.Locale, "Found ICU Locale " << localeName(*r) << " for specification " << describeArgs);
  return r;
}

struct IcuLocaleConfig {
  LocalePtr pLocale;
  Locale const& locale() const { return *pLocale; }
  std::string language, country;
  friend inline std::string to_string_impl(IcuLocaleConfig const& x) { return x.to_string_impl(); }
  std::string to_string_impl() const { return "language='" + language + "' country='" + country + "'"; }
  friend void validate(IcuLocaleConfig& x) { x.validate(); }
  void validate() {
    if (language.empty() && !country.empty())
      SDL_THROW_LOG(IcuLocaleConfig, ConfigException,
                    "don't specify country without specifying language: " << to_string_impl());
    if (language.size() > 3 && !country.empty())
      SDL_THROW_LOG(
          IcuLocaleConfig, ConfigException,
          "don't specify a language_country (e.g. en_US) --language while also specifying --country: "
              << to_string_impl());
    pLocale = getLocalePtr(language, country);
  }
  template <class Config>
  void configure(Config& c) {
    c.is("ICU locale");
    c("language", &language)(
        "ICU language 2-3 letter ISO code, or language_country e.g. en_US. empty means icu (OS?) default. "
        "valid choices: " + icuLanguagesList());
    c("country", &country)("ICU country 2 letter ISO code. may be empty. valid choices: " + icuCountriesList());
    // TODO: region? script? probably unnecessary for our LPs
  }
};

struct IcuErrorCode : icu::ErrorCode {
  boost::optional<UParseError> optParse;
  std::string detail, prefix;
  bool assertSuccessOnDestroy;
  IcuErrorCode(std::string const& detail = "ERROR", std::string const& prefix = "sdl.IcuErrorCode",
               bool assertSuccessOnDestroy = true)
      : detail(detail), prefix(prefix), assertSuccessOnDestroy(assertSuccessOnDestroy) {}
  ~IcuErrorCode() {
    if (assertSuccessOnDestroy) this->assertSuccess();
  }
  UParseError& storeParseError();  // pass this to ICU methods that write a parse error
  std::string parseError() const;
  void fail() { handleFailure(); }

 protected:
  virtual void handleFailure() const {
    THROW_LOG_NAMESTR(prefix, IcuException, detail << ": " << this->errorName() << parseError());
  }
};

void globalIcuData(std::string pathString);  /// must be called before using any ICU APIs from multiple
/// threads, and before u_init() if you use it

// returns a PATH-format string (unix: /bin:/usr/bin , windows: c:/bin;c:/usr/bin )
std::string platformPathString(std::vector<std::string> const& dirs);

inline void globalIcuData(std::vector<std::string> const& dirnames) {
  globalIcuData(platformPathString(dirnames));
}


}}

#endif
