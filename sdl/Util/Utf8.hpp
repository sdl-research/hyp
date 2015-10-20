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

    Provides efficient utf8 bytes<->unicode codepoints decoding/encoding (inline, C++)

    ICU can do this but most straightforwardly does so via a UTF-16 copy in an
    icu::UnicodeString, which is slow.

    We still need ICU, however, for character classes, transliteration, case
    folding, and normalizing

*/

#ifndef SDL_UTIL__UTF8_HPP
#define SDL_UTIL__UTF8_HPP
#pragma once

// TODO: use utf8length instead of alternatives - it's the fastest.


/*
  Unicode: typedef for uint32
  UnicodeChars: Unicode (uint32) sequence
  e.g. vector<uint32> or basic_string<uint32>

  else: byte sequence iter
*/

#include <sdl/Util/Enum.hpp>
#include <sdl/IntTypes.hpp>
#include <sdl/Util/String32.hpp>
#include <graehl/shared/os.hpp>
#include <sdl/Util/ShrinkVector.hpp>
#include <graehl/shared/insert_to.hpp>
#include <vector>
#include <string>
#include <iterator>
#include <graehl/shared/warning_compiler.h>
CLANG_DIAG_OFF(unsequenced)
#include <utf8/checked.h>
#include <utf8/unchecked.h>
CLANG_DIAG_ON(unsequenced)
#define SDL_CHECKED_UTF8 1
#if SDL_CHECKED_UTF8
#define UTF8_CHECKED_NS utf8
#else
#define UTF8_CHECKED_NS utf8::unchecked
#endif
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/distance.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <sdl/Types.hpp>
#include <sdl/Array.hpp>
#include <sdl/Span.hpp>


namespace sdl {
namespace Util {


/**
   both ICU and utf8-cpp will replace bad utf8/unicode points with this char if you ask them to.
*/
Unicode const kUnicodeReplacementChar = (Unicode)65533U;
// TODO: figure out if/why windows compiler can't handle 0xFFFDU (which is equal
// to 65533 so no harm). we need the U to prevent sign extension in case Unicode
// is a signed type

inline bool isReplacementChar(Unicode ch) {
  // compare ch with U+FFFD
  return ch == kUnicodeReplacementChar;
}

// TODO: next(x) has a C++11 ADL conflict that didn't allow using, so these forward without ambiguity:
template <typename octet_iterator>
Unicode next(octet_iterator& it) {
  return utf8::unchecked::next(it);
}

template <typename octet_iterator>
Unicode next(octet_iterator& it, octet_iterator const& end) {
#if SDL_CHECKED_UTF8
  return utf8::next(it, end);
#else
  return utf8::unchecked::next(it);
#endif
}

/**
   \return span indexing bytes at begin[first,second) corresponding to unicode
   codepoint span unicodeSpan. will throw if [begin,end) isn't valid utf8 or
   unicodeSpan addresses past end.
*/
template <typename octet_iterator>
inline TokenSpan toUtf8ByteSpan(TokenSpan const& unicodeSpan, octet_iterator begin, octet_iterator end) {
  TokenSpan r;
  octet_iterator i = begin;
  utf8::advance(i, unicodeSpan.first, end);
  r.first = i - begin;
  utf8::advance(i, len(unicodeSpan), end);
  r.second = i - begin;
  return r;
}

inline TokenSpan toUtf8ByteSpan(TokenSpan const& unicodeSpan, std::string const& s) {
  return toUtf8ByteSpan(unicodeSpan, s.begin(), s.end());
}

template <typename octet_iterator>
octet_iterator append(Unicode c, octet_iterator const& result) {
  return UTF8_CHECKED_NS::append(c, result);
}

template <class ByteOutIter>
struct EncodeUtf8Iter : public boost::iterator_facade<EncodeUtf8Iter<ByteOutIter>, Unicode,
                                                      std::output_iterator_tag, EncodeUtf8Iter<ByteOutIter>&> {
  ByteOutIter& o;
  explicit EncodeUtf8Iter(ByteOutIter& o) : o(o) {}
  void increment() {}
  EncodeUtf8Iter<ByteOutIter>& dereference() const {
    return const_cast<EncodeUtf8Iter&>(*this);  // needed by boost iterator_facade
  }
  void operator=(Unicode c) { o = UTF8_CHECKED_NS::append(c, o); }
};

template <class ByteOutIter>
EncodeUtf8Iter<ByteOutIter> encodeUtf8Iter(ByteOutIter o) {
  return EncodeUtf8Iter<ByteOutIter>(o);
}


SDL_ENUM(BadUtf8HandlerType, 3, (Unsafe_Ignore, Remove, Replace));

Unicode const kMinWindows1252Unicode = 0x0080;
Unicode const kMaxWindows1252Unicode = 0x009f;
Unicode const kNWindows1252Unicode = 1 + kMaxWindows1252Unicode - kMinWindows1252Unicode;
/* these all take 2-byte utf8 to 2- or 3-byte utf8 so it's not safe to transform utf8 inplace*/
extern Unicode gWindows1252ToUnicode[kNWindows1252Unicode];

/// is this a control character that can be translated from windows 1252 codepage?
inline bool isControlInWindows1252Range(Unicode c) {
  return c >= kMinWindows1252Unicode && c <= kMaxWindows1252Unicode;
}

inline Unicode translateWindows1252(Unicode c) {
  return isControlInWindows1252Range(c) ? gWindows1252ToUnicode[c - kMinWindows1252Unicode] : 0;
}

inline void remapWindows1252(Unicode& c) {
  if (isControlInWindows1252Range(c)) c = gWindows1252ToUnicode[c - kMinWindows1252Unicode];
}

/// return whether c is a nonprintable ascii char except for ' ' '\n' '\t'. (\r and others return true)
inline bool isLowAsciiNonSpaceControl(Unicode c) {
  return (c < 0x20 || c == 0x7F) && c != '\n' && c != '\t';
}

inline bool isNonSpaceControl(Unicode c) {
  return isLowAsciiNonSpaceControl(c) || isControlInWindows1252Range(c);
}

template <class ByteIter>
inline bool containsNonSpaceControlChars(ByteIter i, ByteIter end) {
  /// utf8 control chars are ascii control isLowAsciiNonSpaceControl which fit in 1 utf8 byte, or else two
  /// bytes: (C2 [80-9F])
  bool c2 = false;
  for (; i != end; ++i) {
    unsigned char const c = *i;
    if (isLowAsciiNonSpaceControl(c)) return true;
    if (c == 0xC2)
      c2 = true;
    else {
      if (c2 && isControlInWindows1252Range(c)) return true;
      c2 = false;
    }
  }
  return false;
}

inline bool containsNonSpaceControlChars(std::string const& s) {
  return containsNonSpaceControlChars(s.begin(), s.end());
}

template <class Bytes>
inline bool validUtf8(Bytes const& bytes) {
  return utf8::is_valid(bytes.begin(), bytes.end());
}

/// until we have original-byte-span or original-unicode-code-point alignments
/// when doing nfc, we disable all on-by-default normalizations that might apply
/// before tokenizers record spans.

enum { kAlreadyValidUtf8 = true };

/// options for 1:1 and deleting Unicode translations for control chars and
/// kUnicodeReplacementChar, with badUtf8Handler also including the (not
/// recommended) option of ignoring corrupt utf8
struct FixUnicode {
  void initAssumingValidUtf8(bool assumeValidUtf8) {
    defaults();
    badUtf8Handler = assumeValidUtf8 ? kUnsafe_Ignore : kRemove;
  }

  void disableAllFixes() {
    removeControlChars = false;
    convertWindows1252 = false;
    badUtf8Handler = kUnsafe_Ignore;
  }

  void enableAllFixes() {
    removeControlChars = true;
    convertWindows1252 = true;
    badUtf8Handler = kRemove;
  }

  void defaults() {
    removeControlChars = false;
    convertWindows1252 = true;  // will never alter # of unicode chars
    badUtf8Handler = kRemove;
  }

  /// it's risky to enable options that remove unicode codepoints because the
  /// caller will probably be confused about alignments into the input string,
  /// which are in terms of unicode codepoints. therefore these options are
  /// disabled by default until further testing. as for NFC and removing bad
  /// utf8 (and U+FFFE, the 'replacement' char), callers should simply be aware
  /// of that (it's simple/standard enough)
  bool removesUnicodes() const { return removeControlChars; }
  // TODO: not safe unless we implement constraints (char alignment) adjustment

  FixUnicode() { defaults(); }

  FixUnicode(bool disabled) {
    if (disabled)
      disableAllFixes();
    else
      defaults();
  }

  bool removeControlChars;  // TODO: not safe unless we implement constraints (char alignment) adjustment

  bool convertWindows1252;  // safe to enable (1:1)
  BadUtf8HandlerType badUtf8Handler;

  bool cleanupEnabled() const {
    return badUtf8Handler != kUnsafe_Ignore || removeControlChars || convertWindows1252;
  }

  bool modifiesControlChars() const { return removeControlChars || convertWindows1252; }

  bool modifiesControlChars(std::string const& str) const {
    return modifiesControlChars() && containsNonSpaceControlChars(str);
  }

  template <class Config>
  void configure(Config& config) {
    config.is("FixUnicode");
    config("bad-utf8-handler", &badUtf8Handler)
        .init(kRemove)(
            "Handle malformed UTF-8 byte sequences, by either removing them, replacing them with the UTF-8 "
            "replacement character 0xfffd, or ignoring them. warning: the quality of our models is severely "
            "degraded if you use Ignore for text that is not really utf8");
    config("remove-control-characters", &removeControlChars)
        .self_init()(
            "Remove non-whitespace control characters (whitespace is normalized/handled by a different "
            "mechanism). TODO: this is unsafe to use w/ constraints unless those constraints magically refer "
            "to post-removal unicode ids");
    config("convert-windows-1252", &convertWindows1252)
        .self_init()(
            "Interpret Unicode range U+0080 - U+009F control characters as encoded Windows 1252 characters "
            "and convert to correct Unicode equivalents (this option has precedence over removal of control "
            "characters). If someone uses control characters for their intended purpose (unlikely) or for "
            "another font/codepage (somewhat likely) you will get some gibberish");
  }


  /// return true if character isn't deleted, false if it is, modifying \param[out] c
  /// c must have already had the kReplace utf8 handler applied
  bool operator()(Unicode& c) const {
    if (badUtf8Handler == kRemove && c == kUnicodeReplacementChar) return false;
    if (convertWindows1252) remapWindows1252(c);
    if (removeControlChars && isNonSpaceControl(c)) return false;
    return true;
  }

  template <class UnicodeOut, class UnicodesIter>
  void unicodesTo(UnicodesIter i, UnicodesIter const& e, UnicodeOut& o) const {
    for (; i != e; ++i) {
      Unicode c = *i;
      if ((*this)(c)) {
        *o = c;
        ++o;
      }
    }
  }

  template <class UnicodeOut, class ByteIter>
  void utf8To(ByteIter i, ByteIter const& e, UnicodeOut& o) const {
    if (badUtf8Handler == kUnsafe_Ignore || utf8::is_valid(i, e))
      fixedUtf8To(i, e, o);
    else {
      std::vector<char> temp;
      temp.reserve((e - i) + 4);
      utf8::replace_invalid(i, e, std::back_inserter(temp), kUnicodeReplacementChar);
      fixedUtf8To(temp.begin(), temp.end(), o);
    }
  }

  template <class UnicodeOut, class ByteIter>
  void utf8To(ByteIter i, ByteIter const& e, UnicodeOut& o, bool alreadyValidUtf8) const {
    if (alreadyValidUtf8)
      fixedUtf8To(i, e, o);
    else {
      std::vector<char> temp;
      temp.reserve((e - i) + 4);
      utf8::replace_invalid(i, e, std::back_inserter(temp), kUnicodeReplacementChar);
      fixedUtf8To(temp.begin(), temp.end(), o);
    }
  }

  /// [i, e) must be valid utf8
  template <class UnicodeOut, class ByteIter>
  void fixedUtf8To(ByteIter i, ByteIter const& e, UnicodeOut& o) const {
    while (i != e) {
      Unicode c = utf8::unchecked::next(i);
      if (i > e) break;
      if ((*this)(c)) {
        *o = c;
        ++o;
      }
    }
  }

  /// return false if in is already normalized, else normalize(in, storage)
  bool maybeNormalize(std::string const& string, std::string& storage) const {
    if (cleanupEnabled()) {
      bool const valid = badUtf8Handler == kUnsafe_Ignore || validUtf8(string);
      if (valid && !modifiesControlChars(string))
        return false;
      else {
        normalize(string, storage, valid);
        return true;
      }
    } else
      return false;
  }

  typedef EncodeUtf8Iter<std::string::iterator> Encoder;

  void normalize(std::string& string, bool alreadyValidUtf8) const {
    if (alreadyValidUtf8 && !convertWindows1252) {
      // # of bytes in substring translation is <= input so we can do in-place
      std::string::iterator begin = string.begin(), o = begin, end = string.end();
      Encoder toutf8(o);
      utf8To(begin, end, toutf8, alreadyValidUtf8);
      assert(o <= end);
      string.resize(o - begin);
    } else {
      std::string out;
      normalize(string, out, alreadyValidUtf8);
      string = std::move(out);
    }
  }

  void normalize(std::string& string) const {
    if (cleanupEnabled()) {
      bool const valid = badUtf8Handler == kUnsafe_Ignore || validUtf8(string);
      if (!valid || modifiesControlChars(string)) normalize(string, valid);
    }
  }


  void normalize(std::string const& in, std::string& out) const {
    sizeOutputImpl(in.size(), out);
    std::string::iterator begin = out.begin(), o = begin;
    Encoder toutf8(o);
    utf8To(in.begin(), in.end(), toutf8);
    out.resize(o - begin);
  }

  template <class Pchar>
  void normalize(Pchar i, Pchar end, std::string& out, bool alreadyValidUtf8) const {
    sizeOutputImpl(end - i, out);
    std::string::iterator begin = out.begin(), o = begin;
    Encoder toutf8(o);
    utf8To(i, end, toutf8, alreadyValidUtf8);
    out.resize(o - begin);
  }

  void normalize(std::string const& in, std::string& out, bool alreadyValidUtf8) const {
    sizeOutputImpl(in.size(), out);
    std::string::iterator begin = out.begin(), o = begin;
    Encoder toutf8(o);
    utf8To(in.begin(), in.end(), toutf8, alreadyValidUtf8);
    out.resize(o - begin);
  }

 private:
  void sizeOutputImpl(std::size_t sz, std::string& out) const {
    out.resize((1 + convertWindows1252) * sz);
    // 1252 can take utf8 2 bytes to 3 bytes. but we provide 4.
  }
};

#ifdef _MSC_VER
// this gives an unknown pragma warning in vs2010. how does it work?
#pragma execution_character_set("utf-8")
#endif

#define UTF8(str) str

static const std::size_t MAX_UTF8_CHAR_BYTES = 4;
inline std::size_t bytesForUtf(std::size_t nchar) {
  return MAX_UTF8_CHAR_BYTES * nchar;
}

template <class Utf16Iter, class Utf8Chars>
inline void appendUtf8From16(Utf8Chars& vec, Utf16Iter begin, Utf16Iter end) {
  unsigned n16 = (unsigned)(end - begin);
  if (!n16) return;
  vec.reserve(vec.size() + (n16 * 3) / 2);  // middle of road estimate for asian languages (not 4 for worst
  // case), asian languages commonly pay 3 bytes per char but one
  // resize+grow wouldn't kill you, would it?
  utf8::unchecked::utf16to8(begin, end, std::back_inserter(vec));
}

template <class Utf16Chars, class Utf8Chars>
inline void appendUtf8From16(Utf8Chars& vec, Utf16Chars const& append) {
  unsigned n16 = (unsigned)append.size();
  if (!n16) return;
  vec.reserve(vec.size() + (n16 * 3) / 2);  // middle of road estimate for asian languages (not 4 for worst
  // case), asian languages commonly pay 3 bytes per char but one
  // resize+grow wouldn't kill you, would it?
  utf8::unchecked::utf16to8(append.begin(), append.end(), std::back_inserter(vec));
}

template <class Utf8Iter, class Utf16Chars>
inline void appendUtf16From8(Utf16Chars& vec, Utf8Iter begin, Utf8Iter end) {
  unsigned n8 = (unsigned)(end - begin);
  if (!n8) return;
  vec.reserve(n8 * 2);  // common case: ascii (probably also worst-case)
  utf8::unchecked::utf8to16(begin, end, std::back_inserter(vec));
}

template <class Utf16Iter, class Utf8Chars>
inline void setUtf8From16(Utf8Chars& vec, Utf16Iter begin, Utf16Iter end) {
  unsigned n16 = (unsigned)(end - begin);
  if (!n16) {
    vec.clear();
    return;
  }
  vec.resize(bytesForUtf(n16));
  typename Utf8Chars::iterator out = vec.begin();
  vec.resize(utf8::unchecked::utf16to8(begin, end, out) - out);
}


template <class Int>
bool charFits(Unicode uc) {
  Int i = (Int)uc;
  return (Unicode)i == uc;
}

template <class Int>
bool charFits(Unicode uc, Int& i) {
  i = (Int)uc;
  return (Unicode)i == uc;
}

/*

  The library used in this file (Utf8.cpp) in fact has a maximum length of 4 octets. So the code is correct
  (won't buffer overflow). But this does make me worry about interoperability with someone who thinks they've
  written valid UTF-8 but hasn't. I'm not sure how to address this risk.

  However, "In November 2003 UTF-8 was restricted by RFC 3629 to four bytes in order to match the constraints
  of the UTF-16 character encoding."

*/

namespace Utf8 {
using utf8::unchecked::iterator;
}

typedef utf8::unchecked::iterator<std::string::const_iterator> FromUtf8Iter;
typedef utf8::unchecked::iterator<char const*> FromUtf8IterC;
// checked iterator must be initialized with the whole range

// could use boost::iterator_range<Utf8::iterator<Bytes::iterator> > instead - this is essentially a template
// typedef
template <class Bytes>
struct DecodeUtf8Range {
  typedef Utf8::iterator<Bytes> iterator;
  typedef Utf8::iterator<Bytes> const_iterator;
  iterator i, e;
  template <class ByteRange>
  explicit DecodeUtf8Range(ByteRange const& r)
      : i(boost::begin(r)), e(boost::end(r)) {}
  DecodeUtf8Range(iterator const& i, iterator const& e) : i(i), e(e) {}
  iterator begin() const { return i; }
  iterator end() const { return e; }
};

template <class ByteRange>
DecodeUtf8Range<typename boost::range_const_iterator<ByteRange>::type> decodeUtf8Range(ByteRange const& r) {
  return DecodeUtf8Range<typename boost::range_const_iterator<ByteRange>::type>(r);
}


/// in-place - if input is valid already, this is more efficient
std::string& fixUtf8(std::string& str);

template <class Strings>
void fixUtf8All(Strings& strings) {
  for (typename Strings::iterator i = strings.begin(), e = strings.end(); i != e; ++i) fixUtf8(*i);
}


/// stores result in fixedStr
inline void fixUtf8To(std::string const& string, std::string& fixedStr) {
  fixedStr.reserve(string.size() + 4);
  utf8::replace_invalid(string.begin(), string.end(), std::back_inserter(fixedStr), kUnicodeReplacementChar);
}

inline void fixUtf8To(Slice s, std::string& fixedStr) {
  utf8::replace_invalid(s.first, s.second, std::back_inserter(fixedStr), kUnicodeReplacementChar);
}

/**
   supply a valid string ref (valid as long as you use this), and avoid paying
   for string copy if the string is already fixed utf8
*/
struct FixedUtf8 : boost::noncopyable {
  /// std::string forwarding:
  typedef std::string::const_iterator const_iterator;
  typedef std::string::size_type size_type;
  typedef const_iterator iterator;
  const_iterator begin() const { return fixed->begin(); }
  const_iterator end() const { return fixed->end(); }
  size_type size() const { return fixed->size(); }
  bool empty() const { return fixed->empty(); }
  char operator[](size_type i) const { return (*fixed)[i]; }
  char const* c_str() const { return fixed->c_str(); }

  operator std::string const&() const { return *fixed; }

  bool modified() const { return fixed == &storage; }
  std::string const& str() const { return *fixed; }

  ///
  /**
     usage: FixedUtf8(inout).moveTo(x);

     where x may be inout.
  */
  void moveTo(std::string &out) {
    if (&out != fixed)
      out = std::move(*fixed);
  }

  /**
     input string must be valid for as long as you use the fixed result.
  */
  FixedUtf8(std::string const& string, bool enableFix = true) { init(string, enableFix); }

  /// replace string by fix(string) nondestructively
  FixedUtf8(std::string const& string, FixUnicode const& fix) {
    fixed = fix.maybeNormalize(string, storage) ? &storage : &string;
  }

  void init(std::string const& string, FixUnicode const& fix) {
    fixed = fix.maybeNormalize(string, storage) ? &storage : &string;
  }

  /// replace bad utf8 in string with kUnicodeReplacementChar, nondestructively
  void init(std::string const& string, bool enableFix = true) {
    if (!enableFix || validUtf8(string))
      fixed = &string;
    else {
      fixed = &storage;
      storage.reserve(string.size() + 4);
      fixUtf8To(string, storage);
    }
  }

 private:
  std::string const* fixed;
  std::string storage;
};


template <class Out>
Out toUtf8(Unicode ch, Out out) {
  return UTF8_CHECKED_NS::append(ch, out);
}

#if !SDL_UNICODE_IS_WCHAR
template <class Out>
Out toUtf8(wchar_t ch, Out out) {
  return UTF8_CHECKED_NS::append((Unicode)ch, out);
}
#endif

template <class V>
void toUtf8v(Unicode c, V& vec) {
  vec.resize(MAX_UTF8_CHAR_BYTES);
  typename V::iterator out = vec.begin();
  vec.resize(UTF8_CHECKED_NS::append(c, out) - out);
}


// note: unchecked output iter
// note: In = Unicodes iter
template <class In, class Out>
Out toUtf8(In in, In const& iend, Out out) {
  for (; in != iend; ++in) out = UTF8_CHECKED_NS::append(*in, out);
  return out;
}

template <class UnicodeChars, class Out>
Out toUtf8(UnicodeChars const& c, Out const& o) {
  return toUtf8(boost::begin(c), boost::end(c), o);
}

template <class I, class V>
void toUtf8vShrink(I i, I const& iend, V& v) {
  typename V::iterator out = v.begin();
  v.resize(toUtf8(i, iend, out) - out);
}

template <class I, class V>
void toUtf8v(I i, I const& iend, V& v) {
  v.resize(bytesForUtf(std::distance(i, iend)));  // could be more efficient: back_inserter?
  toUtf8vShrink(i, iend, v);
}

template <class I, class V>
void appendToUtf8v(I i, I const& iend, V& v) {
  std::size_t const sz = v.size();
  v.resize(sz + bytesForUtf(std::distance(i, iend)));  // could be more efficient: back_inserter?
  v.resize(toUtf8(i, iend, &v[sz]) - &v[0]);
}

template <class Unicodes, class V>
void appendToUtf8v(Unicodes const& unicodes, V& v) {
  std::size_t const sz = v.size();
  v.resize(sz + bytesForUtf(unicodes.size()));  // could be more efficient: back_inserter?
  v.resize(toUtf8(unicodes.begin(), unicodes.end(), &v[sz]) - &v[0]);
}

// UnicodeChars is a vector of Unicode
template <class UnicodeChars, class V>
void toUtf8v(UnicodeChars const& c, V& v) {
  toUtf8v(boost::begin(c), boost::end(c), v);
}

template <class UnicodeChars>
std::string utf8s(UnicodeChars const& c) {
  std::string r;
  toUtf8v(c, r);
  return r;
}

inline std::string utf8s(Unicode c) {
  std::string o(MAX_UTF8_CHAR_BYTES, char());
  o.resize(UTF8_CHECKED_NS::append(c, o.begin()) - o.begin());
  return o;
}


// note: unchecked output iter
// I = Unicodes iter
template <class I, class Out>
Out toUtf8s(I i, I const& iend, Out o) {
  for (; i != iend; ++i, ++o) *o = utf8s(*i);
  return o;
}

template <class I, class V>
void toUtf8sv(I i, I const& iend, V& v) {
  v.resize(bytesForUtf(std::distance(i, iend)));  // could be more efficient: back_inserter?
  typename V::iterator out = v.begin();
  v.resize(toUtf8s(i, iend, out) - out);
}

// v is a vector of strings
template <class UnicodeChars, class V>
void toUtf8sv(UnicodeChars const& c, V& v) {
  v.resize(bytesForUtf(boost::distance(c)));
  toUtf8sv(boost::begin(c), boost::end(c), v);
}


struct PrintUtf8 {};

template <class Out, class UnicodeChars>
void print(Out& o, UnicodeChars const& c, PrintUtf8) {
  toUtf8(c, ::graehl::put_to(o));
}

struct PrintUnicodes {
  Unicodes const& c;
  explicit PrintUnicodes(Unicodes const& c) : c(c) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintUnicodes const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { toUtf8(c, ::graehl::put_to(out)); }
};


struct PrintString32 {
  String32 const& c;
  explicit PrintString32(String32 const& c) : c(c) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintString32 const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { toUtf8(c, ::graehl::put_to(out)); }
};

struct PrintUnicodeSlice : UnicodeSlice {
  explicit PrintUnicodeSlice(UnicodeSlice const& c) : UnicodeSlice(c) {}
  PrintUnicodeSlice(Unicodes const& c, std::size_t len) : UnicodeSlice(&c[0], &c[len]) {
    assert(len <= c.size());
  }
  PrintUnicodeSlice(Unicodes const& c, std::size_t begin, std::size_t end)
      : UnicodeSlice(&c[begin], &c[end]) {
    assert(end >= begin && end <= c.size());
  }
  PrintUnicodeSlice(Punicode begin, Punicode end) : UnicodeSlice(begin, end) { assert(end >= begin); }
  PrintUnicodeSlice(Unicodes::const_iterator begin, Unicodes::const_iterator end)
      : UnicodeSlice(&*begin, &*end) {
    assert(end >= begin);
  }
  PrintUnicodeSlice(String32::const_iterator begin, String32::const_iterator end)
      : UnicodeSlice(reinterpret_cast<Punicode>(&*begin), reinterpret_cast<Punicode>(&*end)) {
    assert(end >= begin);
  }
  friend inline std::ostream& operator<<(std::ostream& out, PrintUnicodeSlice const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { toUtf8(first, second, ::graehl::put_to(out)); }
};

struct PrintUnicode {
  Unicode c;
  explicit PrintUnicode(Unicode c) : c(c) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintUnicode const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { toUtf8(c, ::graehl::put_to(out)); }
};

// output o holds Unicode
template <class UnicodeOut, class Bytes>
UnicodeOut toUnicodes(Bytes i, Bytes const& e, UnicodeOut o) {
  while (i != e) {
    Unicode c = next(i, e);
    *o = c;
    ++o;
  }
  return o;
}

/// for ascii chars only
inline Unicode singleUnicode(unsigned char c) {
  assert(c < 128);
  return (Unicode)c;
}

inline Unicode singleUnicode(std::string const& s) {
  std::string::const_iterator i = s.begin();
  return next(i, s.end());
}


template <class UnicodeCharContainer, class ByteRange>
void appendToUnicodes(ByteRange const& b, UnicodeCharContainer& ucs) {
  ucs.reserve(
      boost::size(b));  // may leave some unused space e.g. if b is not ascii #chars may be 1/3 of bytes
  toUnicodes(boost::begin(b), boost::end(b), std::back_inserter(ucs));
}

template <class UnicodeCharContainer, class ByteRange>
void toUnicodesShrink(ByteRange const& b, UnicodeCharContainer& ucs) {
  assert(ucs.size() == boost::size(b));
  shrinkToNewEnd(ucs, toUnicodes(boost::begin(b), boost::end(b), ucs.begin()));
}

template <class UnicodeCharContainer>
void toUnicodesShrink(Slice const& slice, UnicodeCharContainer& ucs) {
  assert(ucs.size() == len(slice));
  shrinkToNewEnd(ucs, toUnicodes(slice.first, slice.second, ucs.begin()));
}

template <class UnicodeCharContainer, class ByteRange>
void toUnicodes(ByteRange const& b, UnicodeCharContainer& ucs) {
  ucs.resize(boost::size(b));
  toUnicodesShrink(b, ucs);
}

/**
   less code for adapting a utf8 string into a Unicodes temporary vector.
*/
struct ToUnicodes : Unicodes {
  explicit ToUnicodes(std::string const& utf8) : Unicodes(utf8.size()) { toUnicodesShrink(utf8, *this); }
  explicit ToUnicodes(Slice const& slice) : Unicodes(slice.second - slice.first) {
    toUnicodesShrink(slice, *this);
  }
  explicit ToUnicodes(std::wstring const& word)
#if SDL_UNICODE_IS_WCHAR
      : Unicodes(word.begin(), word.end()) {
  }
#else
  {
    std::vector<char> utf8;
    setUtf8From16(utf8, word.begin(), word.end());
    appendToUnicodes(utf8, *this);
  }
  explicit ToUnicodes(String32 const& unicodes) : Unicodes(unicodes.begin(), unicodes.end()) {}
#endif
};

struct FromUnicodes : std::string {
  explicit FromUnicodes(Unicodes const& word) : std::string(bytesForUtf(word.size()), '\0') {
    toUtf8vShrink(word.begin(), word.end(), *this);
  }
  explicit FromUnicodes(String32 const& word) : std::string(bytesForUtf(word.size()), '\0') {
    toUtf8vShrink(word.begin(), word.end(), *this);
  }
  explicit FromUnicodes(UnicodeSlice const& slice)
      : std::string(bytesForUtf(slice.second - slice.first), '\0') {
    toUtf8vShrink(slice.first, slice.second, *this);
  }
  FromUnicodes(Punicode begin, Punicode end) : std::string(bytesForUtf(end - begin), '\0') {
    toUtf8vShrink(begin, end, *this);
  }
  FromUnicodes(Unicodes::const_iterator begin, Unicodes::const_iterator end)
      : std::string(bytesForUtf(end - begin), '\0') {
    toUtf8vShrink(begin, end, *this);
  }
  FromUnicodes(String32::const_iterator begin, String32::const_iterator end)
      : std::string(bytesForUtf(end - begin), '\0') {
    toUtf8vShrink(begin, end, *this);
  }
#if SDL_UNICODE_IS_WCHAR
  explicit FromUnicodes(WideSlice const& slice) : std::string(bytesForUtf(slice.second - slice.first), '\0') {
    toUtf8vShrink(slice.first, slice.second, *this);
  }
#endif
};

/// naming for symmetry: toUnicodes(str, ustr); ustr.resize(2); fromUnicodes(str, ustr);
template <class UnicodeCharContainer, class ByteVector>
void fromUnicodes(ByteVector& str, UnicodeCharContainer const& ucs) {
  toUtf8v(ucs, str);
}

template <class StringsOutIter>
struct Utf8StringsForUnicodeChars
    : public boost::iterator_facade<Utf8StringsForUnicodeChars<StringsOutIter>, Unicode,
                                    std::output_iterator_tag, Utf8StringsForUnicodeChars<StringsOutIter>&> {
  StringsOutIter o;
  explicit Utf8StringsForUnicodeChars(StringsOutIter o) : o(o) {}
  void increment() {}
  Utf8StringsForUnicodeChars<StringsOutIter>& dereference() const {
    return const_cast<Utf8StringsForUnicodeChars&>(*this);  // needed by boost iterator_facade
  }
  void operator=(Unicode c) {
    *o = utf8s(c);
    ++o;
  }
};

template <class ByteIter, class StringsOutIter>
StringsOutIter toUtf8Chs(ByteIter i, ByteIter const& e, StringsOutIter const& o,
                         FixUnicode const& fix = FixUnicode()) {
  Utf8StringsForUnicodeChars<StringsOutIter> out(o);
  fix.utf8To(i, e, out);
  return out.o;
}

// toUtf8Chs - vector<char> (utf8 enc) -> vector<string> (unicode chars in utf8)
template <class Strs, class BytesRange>
void toUtf8Chs(BytesRange const& b, Strs& ucs, FixUnicode const& fix = FixUnicode()) {
  toUtf8Chs(boost::begin(b), boost::end(b), std::back_inserter(ucs), fix);
}

template <class ByteIter>
unsigned lengthUtf8Chs(ByteIter first, ByteIter const& last) {
  unsigned length = 0;
  while (first != last) {
    next(first, last);
    length++;
  }
  return length;
}

template <class BytesRange>
unsigned lengthUtf8Chs(BytesRange const& b) {
  return lengthUtf8Chs(boost::begin(b), boost::end(b));
}

/**
   *Get the length of a UTF-8 string.
   This method is from Colin Percival described at:
   http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html

   // WARNING from JG: this will read uninitialized bytes (in a harmless way) and so trigger valgrind. so
 don't call it in debug mode unit tests; instead call the char * range version I adapted (utf8length)
 **/

// 0x0101010101010101ull
#define SDL_UTF8_ONEMASK ((std::size_t)(-1) / 0xFF)
inline std::size_t strlen_utf8(const char* _s) {
  const char* s;
  std::size_t count = 0;
  std::size_t u;
  unsigned char b;

  /* Handle any initial misaligned bytes. */
  for (s = _s; (uintptr)(s) & (sizeof(std::size_t) - 1); s++) {
    b = *s;

    /* Exit if we hit a zero byte. */
    if (b == '\0') goto done;

    /* Is this byte NOT the first byte of a character? */
    count += (b >> 7) & ((~b) >> 6);
  }

  /* Handle complete blocks. */
  for (;; s += sizeof(std::size_t)) {
#ifndef _MSC_VER
    /* Prefetch 256 bytes ahead. */
    __builtin_prefetch(&s[256], 0, 0);
#endif

    /* Grab 4 or 8 bytes of UTF-8 data. */
    u = *(std::size_t*)(s);

    /* Exit the loop if there are any zero bytes. */
    if ((u - SDL_UTF8_ONEMASK) & (~u) & (SDL_UTF8_ONEMASK * 0x80)) break;

    /* Count bytes which are NOT the first byte of a character. */
    u = ((u & (SDL_UTF8_ONEMASK * 0x80)) >> 7) & ((~u) >> 6);
    count += (u * SDL_UTF8_ONEMASK) >> ((sizeof(std::size_t) - 1) * 8);
  }

  /* Take care of any left-over bytes. */
  for (;; s++) {
    b = *s;

    /* Exit if we hit a zero byte. */
    if (b == '\0') break;

    /* Is this byte NOT the first byte of a character? */
    count += (b >> 7) & ((~b) >> 6);
  }

done:
  return ((s - _s) - count);
}

/**
   utf8 additional bytes 2-6 all have msb 10, i.e. are on [128,192).
*/
inline bool byteContinuesUtf8Char(unsigned char b) {
  // first bit is 1 and second bit is 0
  return (b >> 7) & (~b >> 6);
}

/**

   \return number of codepoints in bytes [begin, end) if it's valid utf8 - much faster than
   iterating over codepoints.

   if it's a substring of valid utf8, returns number of codepoint-intitial
   bytes in str. (doesn't verify that str completes its final codepoint).

   sum over an arbitrary split of a utf8 string of utf8length gives the correct
   total length.

   if you have a non-vector iterator over bytes, see lengthUtf8Chs which uses
   the slower utf8::next

   adapted for std::string and std::vector<char> from Colin Percival's C-string
   version http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html
*/
inline std::size_t utf8length(char const* begin, char const* end) {
  typedef uintptr Uptr;
  char const* s = begin;
  std::size_t continuationBytes
      = 0;  // number of bytes which do not start a codepoint (of the form 10xxxxxx binary)
  std::size_t bytes = (std::size_t)(end - s);
  std::size_t block;

  if (bytes >= 2 * sizeof(std::size_t)) {
    // we definitely have a full aligned size_t block

    // unaligned bytes first
    for (; (uintptr)(s) & (sizeof(std::size_t) - 1); s++) continuationBytes += byteContinuesUtf8Char(*s);

    char const* endAligned = (char const*)((Uptr)end & ~((Uptr)(sizeof(std::size_t) - 1)));

    // Handle complete (std::size_t sized) blocks.

    // we could just use a std::size_t * but sticking to char * simplifies leftover handling
    for (; s != endAligned; s += sizeof(std::size_t)) {
      assert(s < endAligned);
#ifndef _MSC_VER
      /* Prefetch 256 bytes ahead. */
      // TODO: make this smarter (don't prefetch past actual end - 2nd loop or if)
      __builtin_prefetch(&s[256], 0, 0);
#endif

      block = *reinterpret_cast<std::size_t const*>(s);

      // relies heavily on constant int expr optimization - works for size_t of any size (not just 4 or 8
      // bytes)
      block = ((block & (SDL_UTF8_ONEMASK * 0x80)) >> 7) & ((~block) >> 6);
      // now a bit in each byte in block is either 1 (if input started with '10') or 0.
      // by multiplying, count the number of set bits
      // continuationBytes += count from 0 to 8 of bytes starting with '10'
      continuationBytes += (block * SDL_UTF8_ONEMASK) >> ((sizeof(std::size_t) - 1) * 8);
    }
  }

  // remaining bytes that won't fit in a full block
  for (; s != end; s++) {
    assert(s < end);
    continuationBytes += byteContinuesUtf8Char(*s);
  }

  return bytes - continuationBytes;
}

/**
   \return number of codepoints in str if str is valid utf8 - much faster than
   iterating over codepoints.
*/
inline std::size_t utf8length(std::string const& str) {
  char const* cstr = str.data();
  return utf8length(cstr, cstr + str.size());
}

/**
   \return number of codepoints in str if str is valid utf8 - much faster than
   iterating over codepoints.
*/
inline std::size_t utf8length(std::vector<char> const& str) {
  return utf8length(arrayBegin(str), arrayEnd(str));
}

inline std::size_t utf8length(Slice const& str) {
  return utf8length(str.first, str.second);
}


}}

#endif
