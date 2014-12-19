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

    NFC for utf8 strings

    NFC gets us close to Unicode = grapheme cluster (but not exactly)

    TODO: find a fast implementation that doesn't require the ICU libraries
    */

#ifndef NFC_JG_2014_01_14_HPP
#define NFC_JG_2014_01_14_HPP
#pragma once

#include <string>

#include <sdl/Util/IcuHeaders.hpp>

namespace sdl {

enum { kXmtDefaultNfc = false };

namespace Util {

/// until we have original-byte-span or original-unicode-code-point alignments
/// when doing nfc, we disable all on-by-default normalizations that might apply
/// before tokenizers record spans.

enum { kSilentlyNfc = false, kWarnUnlessNfc = true };

/// return true if out.append(normalize(in)), else in was already normalized and out
/// is unmodified. if warnFalsePositiveOk will warn if quick check doesn't say
/// 'yes'
bool maybeNormalizeToNfc(std::string const& in, std::string& out, bool warnIfNotNfc = kSilentlyNfc,
                         bool warnFalsePositiveOk = true);

/// pre: out is empty
void normalizeToNfc(std::string const& in, std::string& out, bool warnIfNotNfc = kSilentlyNfc);

bool isNfc(std::string const& s);

bool isNfcWarn(std::string const& s);

std::string normalizedToNfc(std::string const& s);

/**
   possibly noncopying std::string const& for utf8 ->NFC (Illegal input is replaced with U+FFFD)

   use after static init only (static icu normalizer pointer).
*/

struct NfcStringRef {
  std::string buf;
  std::string const* p;

  bool modified() const { return p == &buf; }

  /// nfc must remain constructed while you use this
  NfcStringRef(NfcStringRef const& nfc) : buf(nfc.buf), p(nfc.modified() ? &buf : nfc.p) {}

  /// utf8 must remain constructed while you use this
  NfcStringRef(std::string const& utf8, bool warnIfNotNfc = kSilentlyNfc)
      : p(maybeNormalizeToNfc(utf8, buf, warnIfNotNfc) ? &buf : &utf8) {}

  /// NfcStringRef must remain constructed while you use this
  operator std::string const&() const { return *p; }
};

/**
   \return s, modified -> NFC (Illegal input is replaced with U+FFFD).

   this is a fast no-op when a quick check (NFC_QC property) can show s is definitely already NFC.
*/
inline void normalizeToNfcInPlace(std::string& s, bool warnIfNotNfc = kSilentlyNfc) {
  NfcStringRef nfcRef(s, warnIfNotNfc);
  std::string const& nfc = nfcRef;
  if (&nfc != &s) s = nfc;
}

inline bool getlineNfc(std::istream& in, std::string& utf8, bool warnIfNotNfc = kSilentlyNfc) {
  if ((bool)std::getline(in, utf8)) {
    normalizeToNfcInPlace(utf8, warnIfNotNfc);
    return true;
  } else
    return false;
}

inline bool getlineNfcUntil(std::istream& in, std::string& utf8, char until, bool warnIfNotNfc = kSilentlyNfc) {
  if ((bool)std::getline(in, utf8, until)) {
    normalizeToNfcInPlace(utf8, warnIfNotNfc);
    return true;
  } else
    return false;
}

struct NfcOptions {
  bool nfc, warnIfNotNfc;
  NfcOptions() : nfc(kXmtDefaultNfc), warnIfNotNfc() {}
  NfcOptions(bool nfc) : nfc(nfc), warnIfNotNfc() {}
  template <class Config>
  void configure(Config& config) {
    config("nfc", &nfc).self_init()("normalize input utf8 to Unicode NFC (recommended)");
    config("warn-if-not-nfc", &warnIfNotNfc)
        .self_init()("warn if any non-NFC input is observed when attempting to normalize");
  }

  void normalize(std::string& s) const {
    if (nfc)
      Util::normalizeToNfcInPlace(s, warnIfNotNfc);
    else if (warnIfNotNfc)
      isNfcWarn(s);
  }

  void normalize(std::string const& in, std::string& out) const {
    if (nfc)
      Util::normalizeToNfc(in, out, warnIfNotNfc);
    else if (warnIfNotNfc) {
      out = in;
      isNfcWarn(out);
    }
  }

  bool maybeNormalize(std::string const& in, std::string& out) const {
    if (nfc) {
      return maybeNormalizeToNfc(in, out, warnIfNotNfc);
    } else {
      if (warnIfNotNfc) isNfcWarn(in);
      return false;
    }
  }

  bool getlineNormalized(std::istream& in, std::string& utf8) const {
    if (nfc)
      return Util::getlineNfc(in, utf8, warnIfNotNfc);
    else
      return (bool)std::getline(in, utf8);
  }
};


}}

#endif
