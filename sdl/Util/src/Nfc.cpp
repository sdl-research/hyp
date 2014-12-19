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

#include <sdl/Util/Icu.hpp>
#include <sdl/Util/Nfc.hpp>
#include <sdl/IntTypes.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/Icu.hpp>

namespace sdl {
namespace Util {

void normalizeToNfc(std::string const& utf8, std::string &out, bool warnIfNotNfc) {
  assert(out.empty());
  if (!maybeNormalizeToNfc(utf8, out, warnIfNotNfc))
    out = utf8;
}

bool isNfc(std::string const& s) {
  return isNfc(toIcu(s));
}

bool isNfcWarn(std::string const& s) {
  return isNfcWarn(toIcu(s));
}

std::string normalizedToNfc(std::string const& s) {
  return fromIcu(normalizeToNfc(toIcu(s)));
}

UErrorCode nfcErr = U_ZERO_ERROR;

char const* const kIcuNfcNormalizer("nfc");
#ifdef _WIN32
static icu_48::Normalizer2 const* gNfc = icu_48::Normalizer2::getInstance(NULL, kIcuNfcNormalizer, UNORM2_COMPOSE, nfcErr);
#else
static icu::Normalizer2 const* gNfc =
#if U_ICU_VERSION_MAJOR_NUM < 49
                              icu::Normalizer2::getInstance(NULL, kIcuNfcNormalizer, UNORM2_COMPOSE, nfcErr)
#else
                              icu::Normalizer2::getNFCInstance(nfcErr)
#endif
                              ;
#endif


/// \param[out] buf <- (s -> nfc) and wasNfc <- false, returning buf;
/// or else wasNfc <- true, returning s (Illegal input is replaced with U+FFFD)
UnicodeString const& maybeNormalizeToNfc(UnicodeString const& s,
                                         UnicodeString &buf,
                                         bool &wasNfc,
                                         bool warnIfNotNfc) {
  using namespace icu;
  UErrorCode err = U_ZERO_ERROR;
  UNormalizationCheckResult check = gNfc->quickCheck(s, err); //TODO: immediately transform remainder using new API if not UNORM_YES
  if (!U_SUCCESS(err))
    SDL_THROW_LOG(Nfc, ProgrammerMistakeException, "ensure valid unicode before maybeNormalizeToNfc");
  assert(U_SUCCESS(err));
  wasNfc = check == UNORM_YES || gNfc->isNormalized(s, err);
  assert(U_SUCCESS(err));
  if (wasNfc)
    return s;
  else {
    if (warnIfNotNfc)
      warnNotNfc(s);
    buf = gNfc->normalize(s, err);
    assert(U_SUCCESS(err));
    return buf;
  }
}

bool isNfc(const UnicodeString& s) {
  using namespace icu;
  UErrorCode err = U_ZERO_ERROR;
  assert(U_SUCCESS(err));
  bool const r = gNfc->isNormalized(s, err);
  assert(U_SUCCESS(err));
  return r;
}

bool maybeNormalizeToNfc(std::string const& utf8Hopefully, std::string &buf, bool warnIfNotNfc, bool warnFalsePositiveOk) {
  using namespace icu;
  if (nfcErr != U_ZERO_ERROR)
    SDL_THROW_LOG(Nfc, ProgrammerMistakeException, "couldn't get icu normalizer singleton");

  //TODO: library that does this without going -> UnicodeString
  UErrorCode err = U_ZERO_ERROR;


  //TODO: ICU doc says: Illegal input is replaced with U+FFFD. but we want some
  // way fo detecting that this has happened so we can place a fixed string in
  // buf. C API http://icu-project.org/apiref/icu4c/ustring_8h.html ?
#if 0
  FixedUtf8 fixed(utf8Hopefully);
  std::string const& utf8 = fixed.str();
#else
  std::string const& utf8 = utf8Hopefully;
#endif
  UnicodeString u;
  unsigned replaced = toIcuReplacing(u, utf8);

  int32 len = u.length(), nfcPrefixLen = gNfc->spanQuickCheckYes(u, err);
  assert(U_SUCCESS(err));
  assert(len >= 0 && nfcPrefixLen >= 0);
  if (!replaced && len == nfcPrefixLen)
    return false;
  else {
    if (warnIfNotNfc) {
      if (replaced)
        warnNotNfc(utf8, replaced);
      else if (warnFalsePositiveOk)
        warnNotNfc(utf8);
      else {
        bool yes = gNfc->isNormalized(u.tempSubString(nfcPrefixLen, len), err);
        assert(U_SUCCESS(err));
        if (!yes)
          warnNotNfc(utf8);
      }
    }
    if (replaced == kIcuErrorInReplacing) {
      buf.reserve(utf8.size() + 4);
      fixUtf8To(utf8, buf);
    } else {
      buf.reserve(utf8.size());
      u.tempSubStringBetween(0, nfcPrefixLen).toUTF8String(buf);
      gNfc->normalize(u.tempSubStringBetween(nfcPrefixLen, len), err).toUTF8String(buf);
      assert(U_SUCCESS(err));
    }
    return true;
  }
}

}}
