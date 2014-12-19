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

/**
   \file

   Helper functions for ICU library (see more in Icu.hpp)

*/

#ifndef SDL_UTIL_ICU_UTIL_HPP
#define SDL_UTIL_ICU_UTIL_HPP
#pragma once

#include <string>
#include <sdl/Util/Icu.hpp>
#include <unicode/chariter.h>
#include <unicode/schriter.h>
#include <unicode/unistr.h>
#include <sdl/SharedPtr.hpp>
#include <sdl/Span.hpp>
#include <sdl/Types.hpp>
#include <sdl/Util/String32.hpp>
#include <sdl/Util/Utf8.hpp>

#define SDL_UTF8_CPP_TOKENIZER 1

namespace sdl {
namespace Util {

typedef FromUnicodes FromString32;

/**
   \return true if the UTF-8 string consists of whitespace only.
*/
bool isUtf8Whitespace(std::string const& utf8);

inline std::string toUtf8Char(Unicode ch)
{
  return utf8s(ch);
}

inline std::string toUtf8String(icu::UnicodeString const& str)
{
  return fromIcu(str);
}

String32 toString32(std::string const& utf8);
String16 toString16(std::string const& utf8);

String32 toString32(Slice const& utf8);
String16 toString16(Slice const& utf8);

void toString32(std::string const& utf8, String32 &to);
void toString16(std::string const& utf8, String16 &to);

void toString32(Slice const& utf8, String32 &to);
void toString16(Slice const& utf8, String16 &to);


/// pre: to is an empty wstring. post: to is utf8 converted to wstring
inline void toWideString(std::string const& utf8, std::wstring &to) {
  if (sizeof(wchar_t) == sizeof(Char32))
    toString32(utf8, reinterpret_cast<String32&>(to));
  else if (sizeof(wchar_t) == sizeof(Char16))
    toString16(utf8, reinterpret_cast<String16&>(to));
}

/// pre: to is an empty wstring. post: to is utf8 converted to wstring
inline void toWideString(Slice const& utf8, std::wstring &to) {
  if (sizeof(wchar_t) == sizeof(Char32))
    toString32(utf8, reinterpret_cast<String32&>(to));
  else if (sizeof(wchar_t) == sizeof(Char16))
    toString16(utf8, reinterpret_cast<String16&>(to));
}

/// for efficient creation of temporary wstring from utf8 or whatever
struct ToWideString : std::wstring {
  explicit ToWideString(Slice const& word)
  {
    toWideString(word, *this);
  }
  explicit ToWideString(std::string const& word)
  {
    toWideString(word, *this);
  }
  explicit ToWideString(Unicodes const& unicodes)
#if SDL_UNICODE_IS_WCHAR
      : std::wstring(unicodes.begin(), unicodes.end())
  {}
#else
  {
    toString16(FromUnicodes(unicodes), reinterpret_cast<String16&>(*this));
  }
#endif
  explicit ToWideString(String32 const& unicodes)
#if SDL_UNICODE_IS_WCHAR
      : std::wstring(unicodes.begin(), unicodes.end())
  {}
#else
  {
    toString16(FromString32(unicodes), reinterpret_cast<String16&>(*this));
  }
#endif
};

struct ToString32 : String32 {
  explicit ToString32(Slice const& word)
  {
    toString32(word, *this);
  }
  explicit ToString32(std::string const& word)
  {
    toString32(word, *this);
  }
  explicit ToString32(Unicodes const& unicodes)
      : String32(unicodes.begin(), unicodes.end())
  {}
  ToString32(std::wstring const& word)
#if SDL_UNICODE_IS_WCHAR
      : String32(word.begin(), word.end())
  {}
#else
  {
    std::vector<char> utf8;
    setUtf8From16(utf8, word.begin(), word.end());
    appendToUnicodes(utf8, *this);
  }
#endif
};

#if SDL_UNICODE_IS_WCHAR
typedef FromUnicodes FromWideString;
#else
struct FromWideString : std::string {
  FromWideString(std::wstring const& word) {
    setUtf8From16(*this, word.begin(), word.end());
  }
  FromWideString(WideSlice const& word) {
    setUtf8From16(*this, word.first, word.second);
  }
};
#endif

inline std::wstring toWideString(std::string const& utf8) {
  return ToWideString(utf8);
}

inline std::string toUtf8String(std::wstring const& wstr) {
  return FromWideString(wstr);
}

struct TokenizerBaseImpl {
  unsigned idx_;
  FixUnicode fix_;
#if SDL_UTF8_CPP_TOKENIZER
  FixedUtf8 fixed;
  std::string::const_iterator i, end;
#else
  icu::UnicodeString s;
  icu::StringCharacterIterator charIter_;
#endif
  bool done_;

  /**
     Moves to next non-space

     \return true if found
  */
  bool findNextNonSpace();

  TokenizerBaseImpl(std::string const& utf8Str, FixUnicode const& fix)
      : idx_(0)
      , fix_(fix)
#if SDL_UTF8_CPP_TOKENIZER
      , fixed(utf8Str)
      , i(fixed.begin()), end(fixed.end())
#else
      , s(icu::UnicodeString::fromUTF8(utf8Str)), charIter_(s)
        // fromUTF8: "Illegal input is replaced with U+FFFD. Otherwise, errors result in a bogus string"
#endif
      , done_()
  {}

  //TODO: replace w/ faster Utf8.hpp
  bool charHasNext() const {
#if SDL_UTF8_CPP_TOKENIZER
    return i != end;
#else
    return charIter_.hasNext();
#endif
  }

  Unicode charNext() {
#if SDL_UTF8_CPP_TOKENIZER
    Unicode r = utf8::unchecked::next(i); // unchecked is ok because we already sanitized input utf8
#else
    Unicode r = charIter_.current32();
    charIter_.next32();
#endif
    ++idx_;
    return r;
  }

  void charAdvance() {
    ++idx_;
#if SDL_UTF8_CPP_TOKENIZER
    utf8::unchecked::next(i);
#else
    charIter_.next32();
#endif
  }

  Unicode charPeek() {
#if SDL_UTF8_CPP_TOKENIZER
    return utf8::unchecked::peek_next(i); // unchecked is ok because we already sanitized input utf8
#else
    return charIter_.current32();
#endif
  }

  /// return true iff isspace = space. if true, cursor advanced by one
  bool readUnlessSpace(Unicode &ch, bool space = true) {
    using namespace icu;
#if SDL_UTF8_CPP_TOKENIZER
    if (i == end) return false;
    std::string::const_iterator current(i);
    ch = utf8::unchecked::next(i); // unchecked is ok because we already sanitized input utf8
    if ((bool)u_isspace(ch) == space) {
      i = current;
      return false;
    }
#else
    if (!charIter_.hasNext()) return false;
    ch = charIter_.current32();
    if (u_isspace(ch) == space) return false;
    charIter_.next32();
#endif
    ++idx_;
    return true;
  }

};

struct UnicodeSpace {
  bool operator()(Unicode c) const {
    using namespace icu;
    return u_isspace(c);
  }
};

inline bool unicodeSpace(Unicode c) {
  return u_isspace(c);
}

struct UnicodeAll {
  bool operator()(Unicode) const {
    return true;
  }
};

/**
   Tokenizes using any unicode space character as delimiter. output to whatever kind of string you like; input is via TokenizerBaseImpl (UnicodeString)

   \tparam GenericString std::basic_string<char32> or icu::UnicodeString

*/
template<class GenericString = icu::UnicodeString>
struct Tokenizer : private TokenizerBaseImpl {
 public:

  explicit Tokenizer(std::string const& str, FixUnicode const& fix = FixUnicode());

  bool done() const {
    return done_;
  }

  void next();

  GenericString const& value() const;
  GenericString const& value(TokenSpan&) const;

 private:

  GenericString currToken_;
  TokenSpan tokSpan_;
};

////////////////////////////

inline void appendChar(UChar32 ch, Unicodes* pStr) {
  assert(sizeof(Unicode)==sizeof(UChar32));
  pStr->push_back(static_cast<Unicode>(ch));
}

inline void appendChar(UChar32 ch, String32* pStr) {
  assert(sizeof(Char32)==sizeof(UChar32));
  pStr->push_back(static_cast<Char32>(ch));
}

inline void appendChar(UChar32 ch, String16* pStr) {
  icu::UnicodeString icuStr(ch);
  assert(sizeof(Char16)==sizeof(UChar));
  pStr->append(static_cast<Char16 const*>(icuStr.getTerminatedBuffer()));
}

inline void appendChar(UChar32 ch, icu::UnicodeString* pStr) {
  pStr->append(ch);
}

template<class GenericString>
Tokenizer<GenericString>::Tokenizer(std::string const& utf8Str, FixUnicode const& fix)
    : TokenizerBaseImpl(utf8Str, fix)
{
  next();
}

template <class GenericString>
void clearString(GenericString &str) {
  str.clear();
}

inline void clearString(icu::UnicodeString &str) {
  str.truncate(0);
}

template<class GenericString>
void Tokenizer<GenericString>::next() {
  using namespace icu;
  if (!charHasNext()) {
    done_ = true;
    return;
  }
  clearString(currToken_);
  if (!findNextNonSpace()) {
    done_ = true;
    return;
  }

  tokSpan_.first = idx_;

  Unicode ch;
  while (readUnlessSpace(ch)) {
    if (fix_(ch))
      appendChar(ch, &currToken_);
  }
  tokSpan_.second = idx_;
}

template<class GenericString>
GenericString const& Tokenizer<GenericString>::value() const {
  return currToken_;
}

template<class GenericString>
GenericString const& Tokenizer<GenericString>::value(TokenSpan& tokSpan) const {
  tokSpan = tokSpan_;
  return currToken_;
}


}}

#endif
