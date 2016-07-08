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
#include <sdl/Util/Nfc.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/IntTypes.hpp>

namespace sdl {
namespace Util {

bool constraintsIndexUnicodes(Constraints const& c) {
  return false;
}

void NfcOptions::normalize(std::string& in, Constraints& c) const {
  bool warn = warnIfResultNotNfc;
  if (nfc && !constraintsIndexUnicodes(c)) {
    std::string out;
    normalizeToNfc(in, out, nfkc);
    in = std::move(out);
    return;
  }
  if (warn) maybeWarn(in);  // TODO: adjust constraints while -> nfc (by  AlignedChars.hpp)
}

void normalizeToNfc(std::string const& utf8, std::string& out, bool warnIfNotNfc, bool K) {
  // TODO: test
  assert(out.empty());
  if (!maybeNormalizeToNfc(utf8, out, warnIfNotNfc, K)) out = utf8;
}

void normalizeToNfc(Slice utf8, std::string& out, bool warnIfNotNfc, bool K) {
  assert(out.empty());
  if (!maybeNormalizeToNfc(utf8, out, warnIfNotNfc, K)) assignSlice(out, utf8);
}

std::string normalizedToNfc(std::string const& s, bool K) {
  // TODO: test
  return fromIcu(normalizedToNfc(toIcu(s), K));
}

UErrorCode nfcErr = U_ZERO_ERROR;

char const* const kIcuNfc = "nfc";
char const* const kIcuNfkc = "nfkc";

typedef icu::Normalizer2 IcuNormalizer;
#if 0
#if U_ICU_VERSION_MAJOR_NUM < 49
IcuNormalizer2Ptr gNfc = icu::Normalizer2::getInstance(NULL, kIcuNfc, UNORM2_COMPOSE, nfcErr);
IcuNormalizer2Ptr gNfkc = icu::Normalizer2::getInstance(NULL, kIcuNfkc, UNORM2_COMPOSE, nfcErr);
IcuNormalizer2Ptr gNfd = icu::Normalizer2::getInstance(NULL, kIcuNfc, UNORM2_DECOMPOSE, nfcErr);
IcuNormalizer2Ptr gNfkd = icu::Normalizer2::getInstance(NULL, kIcuNfkc, UNORM2_DECOMPOSE, nfcErr);
#else
IcuNormalizer2Ptr gNfc = icu::Normalizer2::getNFCInstance(nfcErr);
IcuNormalizer2Ptr gNfkc = icu::Normalizer2::getNFKCInstance(nfcErr);
IcuNormalizer2Ptr gNfd = icu::Normalizer2::getNFDInstance(nfcErr);
IcuNormalizer2Ptr gNfkd = icu::Normalizer2::getNFKDInstance(nfcErr);
#endif
#else
IcuNormalizer2Ptr getNfc() {
  return icu::Normalizer2::getNFCInstance(nfcErr);
}
IcuNormalizer2Ptr getNfkc() {
  return icu::Normalizer2::getNFKCInstance(nfcErr);
}
IcuNormalizer2Ptr getNfd() {
  return icu::Normalizer2::getNFDInstance(nfcErr);
}
IcuNormalizer2Ptr getNfkd() {
  return icu::Normalizer2::getNFKDInstance(nfcErr);
}
#endif

bool isNfc(UnicodeString const& s, bool K) {
  using namespace icu;
  UErrorCode err = U_ZERO_ERROR;
  assert(U_SUCCESS(err));
  bool const r = (K ? getNfkc() : getNfc())->isNormalized(s, err);
  assert(U_SUCCESS(err));
  return r;
}


bool maybeNormalizeToNfc(Slice utf8, std::string& buf, bool warnIfNotNfc, bool warnFalsePositiveOk, bool K) {
  unsigned utf8len = len(utf8);
  if (!utf8len) return false;
  using namespace icu;
  if (nfcErr != U_ZERO_ERROR)
    SDL_THROW_LOG(Nfc, ProgrammerMistakeException, "couldn't get icu normalizer singleton");

  // TODO: library that does this without going -> UnicodeString
  UErrorCode err = U_ZERO_ERROR;

  IcuNormalizer2Ptr norm = K ? getNfkc() : getNfc();

  UnicodeString u;
  // ICU doc says: Illegal input is replaced with U+FFFD. therefore we need not use FixedUtf8
  unsigned replaced = toIcuReplacing(u, icuStringPiece(utf8));

  int32 len = u.length(), nfcPrefixLen = norm->spanQuickCheckYes(u, err);
  assert(U_SUCCESS(err));
  assert(len >= 0 && nfcPrefixLen >= 0);
  if (!replaced && len == nfcPrefixLen)
    return false;  // already all valid utf8 (!replaced) and NFC.
  else {
    if (warnIfNotNfc) {
      // TODO: test
      if (replaced)
        SDL_WARN(Icu.Nfc, "'" << utf8 << "' (utf8 with " << replaced
                              << " invalid codepoints) is not normal-form-composed (NFC)");
      else if (warnFalsePositiveOk)
        warnNotNfc(utf8);
      else {
        bool yes = norm->isNormalized(u.tempSubString(nfcPrefixLen, len), err);
        assert(U_SUCCESS(err));
        if (!yes) warnNotNfc(utf8);
      }
    }
    if (replaced == kIcuErrorInReplacing) {
      // TODO: test
      SDL_WARN(Nfc,
               "Unexpected: Icu lib reported error in replacing bad utf8 characters. Retrying after cleaning "
               "utf8.");
      std::string tmp;
      tmp.reserve(utf8len + 4);
      fixUtf8To(utf8, tmp);
      normalizeToNfc(tmp, buf, warnIfNotNfc, K);
      return true;
    } else {
      buf.reserve(utf8len);
      u.tempSubStringBetween(0, nfcPrefixLen).toUTF8String(buf);
      UnicodeString tmp;
      norm->normalize(u.tempSubStringBetween(nfcPrefixLen, len), tmp, err);
      tmp.toUTF8String(buf);
      assert(U_SUCCESS(err));
    }
    return true;
  }
}


}}
