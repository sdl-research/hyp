/** \file

    NFC for utf8 strings

    NFC gets us close to Unicode = grapheme cluster (but not exactly)

    TODO: find a fast implementation that doesn't require the ICU libraries
    */

#ifndef NFC_JG_2014_01_14_HPP
#define NFC_JG_2014_01_14_HPP
#pragma once

#include <string>
#include <sdl/Util/Icu.hpp>

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
bool maybeNormalizeToNfc(Slice in, std::string& out, bool warnIfNotNfc = kSilentlyNfc,
                         bool warnFalsePositiveOk = true, bool nfkc = false);

inline bool maybeNormalizeToNfc(std::string const& in, std::string& out, bool warnIfNotNfc = kSilentlyNfc,
                         bool warnFalsePositiveOk = true, bool nfkc = false) {
  return maybeNormalizeToNfc(toSlice(in), out, warnIfNotNfc, warnFalsePositiveOk, nfkc);
}


/// pre: out is empty
void normalizeToNfc(Slice in, std::string& out, bool warnIfNotNfc = kSilentlyNfc,
                    bool nfkc = false);

/// pre: out is empty
void normalizeToNfc(std::string const& in, std::string& out, bool warnIfNotNfc = kSilentlyNfc,
                    bool nfkc = false);

std::string normalizedToNfc(std::string const& s, bool nfkc = false);

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
  NfcStringRef(std::string const& utf8, bool warnIfNotNfc = kSilentlyNfc, bool nfkc = false)
      : p(maybeNormalizeToNfc(utf8, buf, warnIfNotNfc, nfkc) ? &buf : &utf8) {}

  /// NfcStringRef must remain constructed while you use this
  operator std::string const&() const { return *p; }
};

/**
   \return s, modified -> NFC (Illegal input is replaced with U+FFFD).

   this is a fast no-op when a quick check (NFC_QC property) can show s is definitely already NFC.
*/
inline void normalizeToNfcInPlace(std::string& s, bool warnIfNotNfc = kSilentlyNfc, bool nfkc = false) {
  NfcStringRef nfcRef(s, warnIfNotNfc, nfkc);
  std::string const& nfc = nfcRef;
  if (&nfc != &s) s = nfc;
}

inline bool getlineNfc(std::istream& in, std::string& utf8, bool warnIfNotNfc = kSilentlyNfc,
                       bool nfkc = false) {
  if ((bool)std::getline(in, utf8)) {
    normalizeToNfcInPlace(utf8, warnIfNotNfc, nfkc);
    return true;
  } else
    return false;
}

inline bool getlineNfcUntil(std::istream& in, std::string& utf8, char until, bool warnIfNotNfc = kSilentlyNfc,
                            bool nfkc = false) {
  if ((bool)std::getline(in, utf8, until)) {
    normalizeToNfcInPlace(utf8, warnIfNotNfc, nfkc);
    return true;
  } else
    return false;
}

struct NfcOptions {
  bool nfc, nfkc, warnIfNotNfc, warnIfResultNotNfc;
  NfcOptions() : nfc(kXmtDefaultNfc), nfkc(), warnIfNotNfc(), warnIfResultNotNfc(true) {}
  NfcOptions(bool nfc) : nfc(nfc), nfkc(), warnIfNotNfc(), warnIfResultNotNfc(true) {}
  template <class Config>
  void configure(Config& config) {
    config("nfc", &nfc).self_init()(
        "normalize input utf8 to Unicode NFC (at decode time, if there are constraints, then caller must "
        "refer to post-NFC codepoints)");
    config("nfkc", &nfkc).self_init()(
        "(takes precedence over nfc) normalize input utf8 to Unicode NFKC (at decode time, if there are constraints, then caller must "
        "refer to post-NFKC codepoints)");
    config("warn-if-not-nfc", &warnIfNotNfc)
        .self_init()("warn if any non-NFC input is observed (and then nfc/nfkc normalize if enabled)");
    config("warn-if-result-not-nfc", &warnIfResultNotNfc)
        .self_init()("warn if the result isn't NFC (if nfc or nfkc normalization is enabled, then you won't ever see this warning, so it's safe to leave on)");
    //option warn-if-not-nfkc? wait til requested.
  }

  friend inline void validate(NfcOptions & x) {
    x.validate();
  }
  void validate() {
    if (warnIfNotNfc)
      warnIfResultNotNfc = true;
    if (nfkc)
      nfc = true;
  }

  void maybeWarn(std::string const& in) const {
    if (warnIfResultNotNfc)
      isNfcWarn(in, false);
  }

  void maybeWarn(Slice in) const {
    if (warnIfResultNotNfc)
      isNfcWarn(in, false);
  }

  void normalize(std::string& in) const {
    if (nfc)
      Util::normalizeToNfcInPlace(in, warnIfNotNfc, nfkc);
    else
      maybeWarn(in);
  }

  void normalize(std::string const& in, std::string& out) const {
    if (nfc)
      Util::normalizeToNfc(in, out, warnIfNotNfc, nfkc);
    else {
      maybeWarn(in);
      out = in;
    }
  }

  bool maybeNormalize(std::string const& in, std::string& out) const {
    if (nfc) {
      return maybeNormalizeToNfc(in, out, warnIfNotNfc, nfkc);
    } else {
      maybeWarn(in);
      return false;
    }
  }

  bool maybeNormalize(Slice in, std::string& out) const {
    if (nfc) {
      return maybeNormalizeToNfc(in, out, warnIfNotNfc, nfkc);
    } else {
      maybeWarn(in);
      return false;
    }
  }

  Slice normalized(Slice in, std::string& maybeOut) const {
    return maybeNormalize(in, maybeOut) ? toSlice(maybeOut) : in;
  }

  std::string const& normalized(std::string const& in, std::string& maybeOut) const {
    return maybeNormalize(in, maybeOut) ? maybeOut : in;
  }

  bool getlineNormalized(std::istream& in, std::string& utf8) const {
    if (nfc)
      return Util::getlineNfc(in, utf8, warnIfNotNfc, nfkc);
    else {
      if (std::getline(in, utf8)) {
        maybeWarn(utf8);
        return true;
      } else
        return false;
    }
  }

  bool enabled() const {
    return warnIfResultNotNfc || nfc;
  }
};


}}

#endif
