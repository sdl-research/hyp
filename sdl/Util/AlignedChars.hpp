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

    each char in a transformed/normalized string has an alignment to a
    contiguous span of some input (we always count the original input in unicode
    codepoints for Constraints, but all that monotone transliteration type
    algorithms need to do is combine the spans of character that are combined to
    produce some output character(s).

    TODO: use this to update constraint indices for stat-tok or string->string
    w/ nfc enabled

    note that the Icu nfc/nfd transformation doesn't give as precise as possible
    char alignment (the API is insufficient)
*/

#ifndef ALIGNEDCHARS_JG_2015_02_11_HPP
#define ALIGNEDCHARS_JG_2015_02_11_HPP
#pragma once

#include <sdl/Span.hpp>
#include <sdl/Types.hpp>
#include <sdl/Array.hpp>
#include <sdl/Util/IcuHeaders.hpp>
#include <sdl/Util/Nfc.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/Delete.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Util {

Unicode const kNoMoreUnicode = (Unicode)-1;

struct IChars {
  virtual Unicode next() = 0;
  virtual ~IChars() {}
  static inline bool done(Unicode c) { return c == kNoMoreUnicode; }
};

struct IAlignedChars : IChars {
  virtual Unicode nextWithSpan(TokenSpan& span) = 0;
};

struct ITakeChars {
  virtual void take(Unicode c) = 0;

  /// nonvirtual - hidden by ITakeAlignedChars
  void takeWithSpan(Unicode c, TokenSpan span) { take(c); }
  virtual ~ITakeChars() {}
};

struct ITakeAlignedChars : ITakeChars {
  virtual void takeWithSpan(Unicode c, TokenSpan span) = 0;
};

template <class IChars>
struct IcuNormalizeByChunks {
  IChars& chars;
  icu::UnicodeString todo, normalizedChunk;
  TokenSpan normalizedSpan, nextSpan;
  IcuNormalizer2Ptr normalizer;

  UErrorCode err;
  Unicode inert;
  IcuNormalizeByChunks(IChars& chars, IcuNormalizer2Ptr normalizer)
      : chars(chars), normalizer(normalizer), err(U_ZERO_ERROR) {
    clearNextSpan();
    inert = kNoMoreUnicode;
  }
  bool next() {
    TokenSpan span;
    Unicode c;
    if (inert != kNoMoreUnicode) {
      assert(todo.isEmpty());
      normalizedChunk.setTo((UChar32)inert);
      normalizedSpan = nextSpan;
      inert = kNoMoreUnicode;
      clearNextSpan();
      return true;
    }
    for (;;) {
      c = chars.nextWithSpan(span);
      SDL_TRACE(Util.AlignedChars, "Got U+" << std::hex << c << " span=[" << span.first << "," << span.second
                                            << ")");
      if (c == kNoMoreUnicode) {
        if (todo.isEmpty()) return false;
        // TODO: test
        normalizeTodo();
        normalizedSpan = nextSpan;
        return true;
      } else {
        if (normalizer->hasBoundaryBefore(c)) {
          bool const already = !todo.isEmpty();
          if (normalizer->isInert(c)) {  // slower than hasBoundaryBefore so nested inside
            if (already) {
              inert = c;
              normalizeTodo();
              normalizedSpan = nextSpan;
              nextSpan = span;
              return true;
            } else {
              normalizedChunk.setTo((UChar32)c);
              normalizedSpan = span;
              clearNextSpan();
              return true;
            }
          } else {
            if (already) {
              normalizeTodo();
              normalizedSpan = nextSpan;
            }
            nextSpan = span;
            todo.setTo((UChar32)c);
            if (already) return true;
          }
        } else {  // keep accumulating
          // TODO: test
          growSpan(nextSpan, span);
          todo += (UChar32)c;
        }
      }
    }
  }

  /// after next(), visits current chunk's chars (doesn't clear chunk)
  template <class Taker>
  void take(Taker& taker) {
    // TODO: test
    using namespace icu;
    UText utext = UTEXT_INITIALIZER;
    LocalUTextPointer close(&utext);
    utext_openConstUnicodeString(&utext, &normalizedChunk, &err);
    assert(U_SUCCESS(err));
    for (;;) {
      UChar32 c = UTEXT_NEXT32(&utext);
      if (c == U_SENTINEL) break;
      taker.take((Unicode)c);
    }
  }

  /// after next(), visits current chunk's chars w/ current chunk span (doesn't clear chunk)
  template <class Taker>
  void takeWithSpan(Taker& taker) {
    using namespace icu;
    UText utext = UTEXT_INITIALIZER;
    LocalUTextPointer close(&utext);
    utext_openConstUnicodeString(&utext, &normalizedChunk, &err);
    assert(U_SUCCESS(err));
    for (;;) {
      UChar32 c = UTEXT_NEXT32(&utext);
      if (c == U_SENTINEL) break;
      taker.takeWithSpan((Unicode)c, normalizedSpan);
    }
    utext_close(&utext);
  }

  /// normalize and take the whole of chars (w/ individual input Unicode spans)
  template <class Taker>
  void takeAllWithSpan(Taker& taker) {
    while (next()) takeWithSpan(taker);
  }

  /// normalize and take the whole of chars as a single chunk (since we don't care about span)
  template <class Taker>
  void takeAll(Taker& taker) {
    // TODO: test
    Unicode c;
    while ((c = chars.next())) {
      if (c == kNoMoreUnicode) break;
      todo += (UChar32)c;
    }
    take(taker);
  }

  void clearNextSpan() { nextSpan = kMinTokenSpan; }

  void normalizeTodo() {
    assert(!todo.isEmpty());
    // we *could* attempt to heuristically align within a chunk, remembering the  (by edit
    // distance or similar). but in most cases alignments are into whole words,
    // so that shouldn't be necessary
    normalizer->normalize(todo, normalizedChunk, err);
    todo.remove();
    assert(U_SUCCESS(err));
  }
};

/// nonvirtual for IcuNormalizeByChunks
struct CharsFromUtf8Impl : FixUnicode {
  Slice chars;
  Position i;
  explicit CharsFromUtf8Impl(Slice const& bytes) : chars(bytes.first, bytes.second), i() {}
  explicit CharsFromUtf8Impl(std::string const& bytes) : chars(arrayBegin(bytes), arrayEnd(bytes)), i() {}
  explicit CharsFromUtf8Impl(std::vector<char> const& bytes)
      : chars(arrayBegin(bytes), arrayEnd(bytes)), i() {}
  Unicode next() {
    // TODO: test
    for (; chars.first < chars.second; ++chars.first) {
      ++i;
      Unicode c = utf8::next(chars.first, chars.second);
      if (operator()(c)) return c;
    }
    return kNoMoreUnicode;
  }
  Unicode nextWithSpan(TokenSpan& span) {
    for (; chars.first < chars.second; ++chars.first) {
      span.first = i;
      span.second = ++i;
      Unicode c = utf8::next(chars.first, chars.second);
      if (operator()(c)) return c;
    }
    return kNoMoreUnicode;
  }
  CharsFromUtf8Impl() {
    chars.first = chars.second = 0;
    i = 0;
  }
};

typedef IcuNormalizeByChunks<CharsFromUtf8Impl> IcuNormalizeUtf8ByChunks;
struct CharsFromUtf8 : CharsFromUtf8Impl, IAlignedChars {
  typedef CharsFromUtf8Impl Impl;
  Unicode next() OVERRIDE { return Impl::next(); }
  Unicode nextWithSpan(TokenSpan& span) OVERRIDE { return Impl::nextWithSpan(span); }
  CharsFromUtf8() {}
  template <class T>
  explicit CharsFromUtf8(T const& t)
      : Impl(t) {}
};

/// nonvirtual CharsFromUnicodes
struct CharsFromUnicodesImpl {
  Position i;
  UnicodeSlice chars;
  Unicode next() {
    ++i;
    return chars.first >= chars.second ? kNoMoreUnicode : *chars.first++;
  }
  Unicode nextWithSpan(TokenSpan& span) {
    if (chars.first >= chars.second) return kNoMoreUnicode;
    span.first = i;
    span.second = ++i;
    return *chars.first++;
  }
  CharsFromUnicodesImpl() {
    chars.first = chars.second = 0;
    i = 0;
  }
  explicit CharsFromUnicodesImpl(Unicodes const& u) : chars(arrayBegin(u), arrayEnd(u)), i() {}
};

struct CharsFromUnicodes : CharsFromUnicodesImpl, IAlignedChars {
  typedef CharsFromUnicodesImpl Impl;
  Unicode next() OVERRIDE { return Impl::next(); }
  Unicode nextWithSpan(TokenSpan& span) OVERRIDE { return Impl::nextWithSpan(span); }
  CharsFromUnicodes() {}
  explicit CharsFromUnicodes(Unicodes const& u) : Impl(u) {}
};

/// nonvirtual CharsFromUnicodes
struct CharsFromUnicodeStringImpl {
  Position i;
  icu::StringCharacterIterator chars;
  Unicode next() {
    if (!chars.hasNext()) return kNoMoreUnicode;
    ++i;
    return chars.next32PostInc();
  }
  Unicode nextWithSpan(TokenSpan& span) {
    // TODO: test
    if (!chars.hasNext()) return kNoMoreUnicode;
    span.first = i;
    span.second = ++i;
    return chars.next32PostInc();
  }
  explicit CharsFromUnicodeStringImpl(icu::UnicodeString& u) : chars(u), i() {}
};

struct CharsFromUnicodeString : CharsFromUnicodeStringImpl, IAlignedChars {
  typedef CharsFromUnicodeStringImpl Impl;
  Unicode next() OVERRIDE { return Impl::next(); }
  Unicode nextWithSpan(TokenSpan& span) OVERRIDE { return Impl::nextWithSpan(span); }
  explicit CharsFromUnicodeString(icu::UnicodeString& u) : Impl(u) {}
};

template <class Bytes>
struct TakeUtf8 : ITakeAlignedChars {
  explicit TakeUtf8(Bytes* bytes = 0, TokenSpans* spans = 0) : bytes(bytes), spans(spans) {}
  Bytes* bytes;
  TokenSpans* spans;
  virtual void take(Unicode c) {
    // TODO: test
    if (spans) {
      Position i = spans->empty() ? 0 : spans->back().second;
      spans->push_back(TokenSpan(i, i + 1));
    }
    append(c);
  }
  virtual void takeWithSpan(Unicode c, TokenSpan span) {
    if (spans) spans->push_back(span);
    append(c);
  }
  void append(Unicode c) {
    assert(bytes);
    utf8::unchecked::append(c, std::back_inserter(*bytes));
    SDL_TRACE(Util.AlignedChars.append, "appended U+" << std::hex << c << ": '" << *bytes << "'");
  }
};

template <class Bytes>
TakeUtf8<Bytes> takeUtf8(Bytes& bytes, TokenSpans* spans = 0) {
  return TakeUtf8<Bytes>(&bytes, spans);
}


struct TakeUnicodes : ITakeAlignedChars {
  explicit TakeUnicodes(Unicodes* unicodes, TokenSpans* spans = 0) : unicodes(unicodes), spans(spans) {}
  Unicodes* unicodes;
  TokenSpans* spans;
  TakeUnicodes() : spans() {}
  void take(Unicode c) OVERRIDE {
    if (spans) {
      Position i = spans->empty() ? 0 : spans->back().second;
      spans->push_back(TokenSpan(i, i + 1));
    }
    assert(unicodes);
    unicodes->push_back(c);
  }
  void takeWithSpan(Unicode c, TokenSpan span) OVERRIDE {
    if (spans) spans->push_back(span);
    assert(unicodes);
    unicodes->push_back(c);
  }
};

template <class OutBytes>
OutBytes appendUnicodeToUtf8(OutBytes out, TokenSpans& spans, Unicode c, TokenSpan const& s) {
  spans.push_back(s);
  return utf8::unchecked::append(c, out);
}

template <class OutUnicodes>
OutUnicodes appendUnicode(OutUnicodes out, TokenSpans& spans, Unicode c, TokenSpan const& s) {
  spans.push_back(s);
  *out = c;
  return ++out;
}

template <class Bytes>
inline void pushUnicodeToUtf8(Bytes& bytes, TokenSpans& spans, Unicode c, TokenSpan const& s) {
  appendUnicodeToUtf8(std::back_inserter(bytes), spans, c, s);
}

inline void pushUnicode(std::vector<char>& bytes, TokenSpans& spans, Unicode c, TokenSpan const& s) {
  appendUnicodeToUtf8(std::back_inserter(bytes), spans, c, s);
}

inline void pushUnicode(std::string& bytes, TokenSpans& spans, Unicode c, TokenSpan const& s) {
  appendUnicodeToUtf8(std::back_inserter(bytes), spans, c, s);
}

template <class Unicodes>
inline void pushUnicode(Unicodes& unicodes, TokenSpans& spans, Unicode c, TokenSpan const& s) {
  unicodes.push_back(c);
  spans.push_back(s);
}

void alignedNormalize(IChars& in, IcuNormalizer2Ptr normalizer, ITakeAlignedChars& out);

void alignedNormalizeStringPiece(icu::StringPiece utf8, IcuNormalizer2Ptr, ITakeAlignedChars& out);

void alignedNormalizeUnicodeString(icu::UnicodeString const& ustr, IcuNormalizer2Ptr, ITakeAlignedChars& out);

template <class Bytes>
void alignedNormalize(Bytes const& utf8, IcuNormalizer2Ptr normalizer, ITakeAlignedChars& out) {
  alignedNormalizeStringPiece(utf8, normalizer, out);
}

inline void alignedNormalize(Slice utf8, IcuNormalizer2Ptr normalizer, ITakeAlignedChars& out) {
  alignedNormalizeStringPiece(icuStringPiece(utf8), normalizer, out);
}

template <class Utf8String, class OutUtf8String>
inline void alignedNormalize(Utf8String const& str, IcuNormalizer2Ptr normalizer, OutUtf8String& out,
                             TokenSpans* spans = 0) {
  CharsFromUtf8Impl chars(str);
  IcuNormalizeUtf8ByChunks norm(chars, gNfc);
  if (spans) {
    TakeUtf8<OutUtf8String> take(&out, spans);
    norm.takeAllWithSpan(take);
  } else {
    // TODO: test
    TakeUtf8<OutUtf8String> take(&out);
    norm.takeAll(take);
  }
}


}}

#endif
