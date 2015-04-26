// Copyright 2014 SDL plc
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

    some string/span/cost acceptors - see IWordToPhrase

    an acceptor a will be called with

    a("first");
    a("word");
    bool keepSendingPhrases = a.finishPhrase(cost);

*/

#ifndef ACCEPTSTRING_JG_2013_12_05_HPP
#define ACCEPTSTRING_JG_2013_12_05_HPP
#pragma once

#include <string>
#include <vector>
#include <sdl/Util/Fields.hpp>
#include <sdl/Features.hpp>
#include <sdl/Span.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Syms.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Util/IcuUtil.hpp>
#include <sdl/Config/Named.hpp>

namespace sdl {
namespace Util {

struct IAcceptString : Config::INamed {
  virtual void operator()(Unicode c) const { operator()(Util::utf8s(c)); }

  virtual void operator()(Unicode c, Position pos) const {
    operator()(Util::utf8s(c), TokenSpan(pos, pos + 1));
  }

  virtual void operator()(Unicode c, TokenSpan span) const { operator()(Util::utf8s(c), span); }

  friend inline std::ostream& operator<<(std::ostream& out, IAcceptString const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    out << '[' << type() << " spans=" << spans() << " multiple=" << multiple() << ']';
  }

  virtual char const* type() const { return "IAcceptString"; }
  /**
     \return whether any more phrases are desired.

     if you accept more than one phrase w/ different costs you'll need to return true and do something w/ cost
  */
  virtual bool finishPhrase(FeatureValue = (FeatureValue)1) const { return false; }

  void operator()(Sym sym, IVocabulary& voc) const { operator()(voc.str(sym)); }

  void operator()(std::vector<char> const& word, TokenSpan span) const {
    operator()(std::string(word.begin(), word.end()), span);
  }

  void operator()(std::vector<char> const& word) const { operator()(std::string(word.begin(), word.end())); }

  virtual void operator()(std::string const& word) const = 0;

  /// note: span is in Unicode chars indices
  virtual void operator()(std::string const& word, TokenSpan span) const { operator()(word); }

  void operator()(Pchar a, Pchar b) const { operator()(Slice(a, b)); }
  void operator()(Pchar a, Pchar b, TokenSpan span) const { operator()(Slice(a, b), span); }

  virtual void operator()(Slice const& field) const { operator()(std::string(field.first, field.second)); }
  virtual void operator()(Slice const& field, TokenSpan span) const {
    operator()(std::string(field.first, field.second), span);
  }

  virtual void operator()(Unicodes const& word, TokenSpan span) const {
    operator()(FromUnicodes(word), span);
  }
  virtual void operator()(Unicodes const& word) const { operator()(FromUnicodes(word)); }
  virtual void operator()(UnicodeSlice const& word, TokenSpan span) const {
    operator()(FromUnicodes(word), span);
  }
  virtual void operator()(UnicodeSlice const& word) const { operator()(FromUnicodes(word)); }
  virtual void operator()(std::wstring const& word, TokenSpan span) const {
    operator()(FromWideString(word), span);
  }
  virtual void operator()(std::wstring const& word) const { operator()(FromWideString(word)); }
#ifdef _WIN32
  // Windows-only signatures. On Linux the types collide.
  virtual void operator()(Util::String32 const& word) const { operator()(FromUnicodes(word)); }
  virtual void operator()(Util::String32 const& word, TokenSpan span) const {
    operator()(FromUnicodes(word), span);
  }
#endif

  typedef FeatureValue Cost;

  /**
     convenience. doesn't call finishPhrase.
  */
  void phrase(Syms const& syms, IVocabulary& voc) const {
    for (Syms::const_iterator i = syms.begin(), e = syms.end(); i != e; ++i) operator()(voc.str(*i));
  }

  /**
     \return true iff acceptor will do something interesting w/ (word, span)
     vs. just (word). it's harmless to call (word, span) in any case; you just
     might be able to save some work if you check spans() first.
  */
  virtual bool spans() const { return false; }

  /// \return true if finishPhrase may return true
  virtual bool multiple() const { return false; }
};

template <class Accept>
inline void acceptUnicode(Accept const& accept, Unicode c, TokenSpan span) {
  accept(Util::utf8s(c), span);
}

inline void acceptUnicode(IAcceptString const& accept, Unicode c, TokenSpan span) {
  accept(c, span);
}

template <class Accept>
inline void acceptUnicode(Accept const& accept, Unicode c, Position pos) {
  accept(Util::utf8s(c), TokenSpan(pos, pos + 1));
}

inline void acceptUnicode(IAcceptString const& accept, Unicode c, Position pos) {
  accept(c, pos);
}

template <class Accept>
inline void acceptUnicode(Accept const& accept, Unicode c) {
  accept(Util::utf8s(c));
}

inline void acceptUnicode(IAcceptString const& accept, Unicode c) {
  accept(c);
}

struct IAcceptSlice : IAcceptString {
  char const* type() const { return "IAcceptSlice"; }
  void operator()(std::vector<char> const& word) const { operator()(Slice(&*word.begin(), &*word.end())); }

  /// note: span is in Unicode chars indices
  virtual void operator()(Slice const& field, TokenSpan span) const { operator()(field); }
  virtual void operator()(Slice const& field) const {
    SDL_THROW_LOG(Util.IAcceptSlice, UnimplementedException, "override (Slice) and/or (Slice, span)");
  }

  virtual void operator()(std::string const& word, TokenSpan span) const { operator()(toSlice(word), span); }
  virtual void operator()(std::string const& word) const { operator()(toSlice(word)); }

  virtual void operator()(Unicodes const& word, TokenSpan span) const {
    operator()(FromUnicodes(word), span);
  }
  virtual void operator()(Unicodes const& word) const { operator()(FromUnicodes(word)); }
  virtual void operator()(std::wstring const& word, TokenSpan span) const {
    operator()(FromWideString(word), span);
  }
  virtual void operator()(std::wstring const& word) const { operator()(FromWideString(word)); }

  virtual ~IAcceptSlice() {}
};

template <class Word>
bool acceptSingleWord(IAcceptString const& phrase, Word const& word, FeatureValue cost = (FeatureValue)1) {
  phrase(word);
  return phrase.finishPhrase(cost);
}


}}

#endif
