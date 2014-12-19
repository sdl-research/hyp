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

    like StringToString but one input word becomes a sequence of 0 or more
   output words (e.g. compound splitter)

   watch the return value: false implies you should keep the original (call
   acceptElseOriginal if you want to avoid thinking about this)

    see docs/writing-string-transformation-modules.md

*/

#ifndef WORDTOPHRASE_JG_2013_12_03_HPP
#define WORDTOPHRASE_JG_2013_12_03_HPP
#pragma once

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/AcceptString.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/Fields.hpp>
#include <sdl/Syms.hpp>
#include <sdl/Types.hpp>
#include <sdl/Config/Named.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/IcuUtil.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Util/WordFilter.hpp>
#include <sdl/Evictable.hpp>
#include <sdl/Util/Delete.hpp>
#include <sdl/Span.hpp>
#include <sdl/Util/AcceptStringImpls.hpp>

namespace sdl {

namespace xmt {
class ModuleManager;
}

namespace Util {

/// removal of duplicates if for your WordToPhrase transform to
/// perform. include-original, the TransformLabelsOptions member, can inform
/// PreventDuplicateOptions+WordToPhrase - you don't want to prevent the
/// original if the arc is being dropped!
struct PreventDuplicateOptions {
  PreventDuplicateOptions() : preventDuplicateOriginal(true) {}
  template <class Config>
  void configure(Config& config) {
    config("prevent-duplicate-original", &preventDuplicateOriginal)
        .self_init()(
            "don't propose original correction if it's already present via 'include-original: true' in "
            "module options. this means you won't get transform features wherever output phrase = input "
            "word");
  }
  bool preventDuplicateOriginal;
  bool preventOriginal(bool originalIncluded) const { return preventDuplicateOriginal && originalIncluded; }
};

typedef PreventDuplicateOptions WordToPhraseOptions;

/**
   you call bool word2phrase(word, AcceptString const& phraseVisitor), which returns
   false if the word was skipped (meaning phraseVisitor wasn't called at
   all). caller may want to leave the input word alone (identity) or delete it
   if it's skipped; we just return false and let caller decide.

   WordToPhrase should *not* return false after calling phraseVisitor.

   see AcceptString docs for how you can accept (possibly) multiple phrases,
   stopping early if you want

*/
struct IWordToPhrase : Evictable {
  friend inline std::ostream& operator<<(std::ostream& out, IWordToPhrase const& self) {
    self.print(out);
    return out;
  }
  virtual void print(std::ostream& out) const {
    out << '[' << category() << ' ' << name() << " inputSpans=" << inputSpans()
        << " outputSpans=" << outputSpans() << " multiple=" << multiple() << ']';
  }

  /**
     \return true if there's any point in calling nextTokenSpan or Position.
  */
  virtual bool inputSpans() const { return false; }

  /**
     \return true iff calls accept(word, span) instead of just
     accept(word). obviously spans are referring to positions within input token
     (this should be true even if inputSpans() is true)
  */
  virtual bool outputSpans() const { return false; }

  virtual void spanRange(Position begin, Position) const { spanBegin = begin; }
  /**
     usage: span(TokenSpan(0,3)) ("the") spanRange(4,6) ("in" ...

     the span shouldn't affect the span passed to IAcceptString (which should
     refer to input word positions starting at 0) but it may inform your model
  */
  void span(TokenSpan span) const { spanRange(span.first, span.second); }

  /// for StringToStringModule
  virtual void suggestOutputVocabulary(IVocabularyPtr const&) {}

  /// for StringToStringModule
  virtual IVocabularyPtr* getPreferredInputVocabulary() { return NULL; }

  /// for StringToStringModule
  virtual void setModuleManager(sdl::xmt::ModuleManager* moduleManager) {}

  char const* category() const { return "transform"; }

  typedef FeatureValue Cost;

  /// for convenience: inheritance gives you these unqualified
  typedef Util::Field Field;
  typedef std::string string;
  typedef std::wstring wstring;
  typedef IAcceptString AcceptString;

  IWordToPhrase() : spanBegin(), originalAlreadyIncluded() {}

  virtual ~IWordToPhrase() {}

  template <class Word>
  void acceptElseOriginal(Word const& word, AcceptString const& phrase, FeatureValue costElse = 0) const {
    if (!operator()(word, phrase)) {
      phrase(word);
      phrase.finishPhrase(costElse);
    }
  }

  template <class Word>
  void acceptElseOriginal(Word const& word, AcceptString const& phrase, TokenSpan spanElse,
                          FeatureValue costElse = 0) const {
    if (!operator()(word, phrase)) {
      SDL_DEBUG(WordToPhrase, word << " wasn't modified - keeping original");
      phrase(word, spanElse);
      phrase.finishPhrase(costElse);
    }
  }

  bool operator()(std::string const& word, AcceptString const& phrase) const {
    return stringToPhrase(word, phrase);
  }

  /// for Hypergraph/TransformLabels
  bool operator()(std::string& utf8) const {
    assert(!mayInsertOrDelete());
    Util::AcceptSingleCheck single(utf8);
    single.require();
    return true;
  }

  bool operator()(Sym sym, IVocabulary& voc, AcceptString const& phrase) const {
    return symToPhrase(sym, voc, phrase);
  }

  bool operator()(Field word, AcceptString const& phrase) const { return fieldToPhrase(word, phrase); }

  bool operator()(Slice const& word, AcceptString const& phrase) const {
    return fieldToPhrase(Field(word), phrase);
  }

  bool operator()(Unicodes const& word, AcceptString const& phrase) const {
    return unicodesToPhrase(word, phrase);
  }

  bool operator()(std::wstring const& word, AcceptString const& phrase) const {
    return wstringToPhrase(word, phrase);
  }

  /**
     these are differently named so you can override one without hiding the other
  */
  virtual bool fieldToPhrase(Field word, AcceptString const& phrase) const = 0;

  virtual bool stringToPhrase(std::string const& word, AcceptString const& phrase) const {
    return fieldToPhrase(Field(word), phrase);
  }

  virtual bool symToPhrase(Sym sym, IVocabulary& voc, AcceptString const& phrase) const {
    std::string const& word(voc.str(sym));  // may need a copy because ...
    return stringToPhrase(word,
                          phrase);  // ... this may add symbols to our vocab while possibly referring to word
  }

  virtual bool unicodesToPhrase(Unicodes const& word, AcceptString const& phrase) const {
    return stringToPhrase(FromUnicodes(word), phrase);
  }

  virtual bool wstringToPhrase(std::wstring const& word, AcceptString const& phrase) const {
    return stringToPhrase(FromWideString(word), phrase);
  }

  virtual bool string32ToPhrase(Util::String32 const& word, AcceptString const& phrase) const {
    return stringToPhrase(FromString32(word), phrase);
  }

  /// \return false only if we know all *ToPhrase calls will return false (do nothing)
  virtual bool enabled() const { return true; }

  /// \return true if any phrase output may have 0 length
  virtual bool mayDelete() const { return !isStringToString(); }

  virtual bool mayInsert() const { return !isStringToString(); }

  virtual bool mayInsertOrDelete() const { return mayDelete() || mayInsert(); }

  /// \return whether we can cast this to IStringToString
  virtual bool isStringToString() const { return false; }

  /**
     \return true if a stringToPhrase call may have multiple bool AcceptString::finishPhrase(Cost)

  */
  virtual bool multiple() const { return false; }

  /**
     \return true if any operator() may return false (meaning no translations were proposed - blocked by
     filter)
  */
  virtual bool mayFilter() const { return true; }

  virtual void notifyIncludeOriginal(bool include) const { originalAlreadyIncluded = include; }

 protected:
  /// meaning TransformLabelsOptions has include-original, so you needn't output
  /// original label. WordToPhrase can be used by multiple threads w/ different
  /// include-original.
  mutable Util::ThreadSpecificBool originalAlreadyIncluded;

  mutable Position
      spanBegin;  // default span implementation - won't get set unless you override spans() => true
  TokenSpan spanRelative(Position begin, Position end) const {
    return TokenSpan(begin + spanBegin, end + spanBegin);
  }
  TokenSpan spanRelative(TokenSpan span) const { return spanRelative(span.first, span.second); }
};

struct IStringToPhrase : IWordToPhrase {
  bool fieldToPhrase(Field word, AcceptString const& phrase) const OVERRIDE {
    return operator()(std::string(word.first, word.second), phrase);
  }

  /** (you implement this)

      caution: if `word` comes from vocab and AcceptString adds to same vocab,
      caller must pass a copy of `word` or else your stringToPhrase must make
      one word before calling AcceptString(...), if `word` will be used after
      that - see xmt-code-gotchas.md - caller is the only one who can be
      informed about this in all cases. we don't copy always because of the cost
      and the lack of need in most use cases. if user-friendly bug avoidance is
      desired, it's IVocabulary that should be fixed.
  */
  bool stringToPhrase(std::string const& word, AcceptString const& phrase) const OVERRIDE {
    SDL_THROW_LOG(WordToPhrase.stringToPhrase, ProgrammerMistakeException, "unimplemented stringToPhrase");
    return false;
  }
};

struct IUnicodesToPhrase : IWordToPhrase {
  bool unicodesToPhrase(Unicodes const& word, AcceptString const& phrase) const OVERRIDE {
    SDL_THROW_LOG(WordToPhrase.unicodesToPhrase, ProgrammerMistakeException,
                  "unimplemented unicodesToPhrase");
    return false;
  }

  virtual bool wstringToPhrase(std::wstring const& word, AcceptString const& phrase) const OVERRIDE {
    return unicodesToPhrase(ToUnicodes(word), phrase);
  }

  bool fieldToPhrase(Field word, AcceptString const& phrase) const OVERRIDE {
    return unicodesToPhrase(ToUnicodes(word), phrase);
  }

  bool stringToPhrase(std::string const& word, AcceptString const& phrase) const OVERRIDE {
    return unicodesToPhrase(ToUnicodes(word), phrase);
  }

  bool symToPhrase(Sym sym, IVocabulary& voc, AcceptString const& phrase) const OVERRIDE {
    return unicodesToPhrase(ToUnicodes(voc.str(sym)),
                            phrase);  // we already copy to Unicodes, so no additional copy needed
  }

  virtual bool string32ToPhrase(Util::String32 const& word, AcceptString const& phrase) const OVERRIDE {
    return unicodesToPhrase(ToUnicodes(word), phrase);
  }
};

struct IString32ToPhrase : IWordToPhrase {
  virtual bool string32ToPhrase(Util::String32 const& word, AcceptString const& phrase) const OVERRIDE {
    SDL_THROW_LOG(WordToPhrase.string32ToPhrase, ProgrammerMistakeException,
                  "unimplemented string32ToPhrase");
    return false;
  }

  bool wstringToPhrase(std::wstring const& word, AcceptString const& phrase) const OVERRIDE {
    return string32ToPhrase(ToString32(word), phrase);
  }

  bool fieldToPhrase(Field word, AcceptString const& phrase) const OVERRIDE {
    return string32ToPhrase(ToString32(word), phrase);
  }

  bool stringToPhrase(std::string const& word, AcceptString const& phrase) const OVERRIDE {
    return string32ToPhrase(ToString32(word), phrase);
  }

  bool symToPhrase(Sym sym, IVocabulary& voc, AcceptString const& phrase) const OVERRIDE {
    return string32ToPhrase(ToString32(voc.str(sym)),
                            phrase);  // we already copy to Unicodes, so no additional copy needed
  }

  bool unicodesToPhrase(Unicodes const& word, AcceptString const& phrase) const OVERRIDE {
    return string32ToPhrase(ToString32(word), phrase);
  }
};

struct IWideStringToPhrase : IWordToPhrase {
  bool wstringToPhrase(std::wstring const& word, AcceptString const& phrase) const OVERRIDE {
    SDL_THROW_LOG(WordToPhrase.wstringToPhrase, ProgrammerMistakeException, "unimplemented wstringToPhrase");
    return false;
  }

  bool string32ToPhrase(Util::String32 const& word, AcceptString const& phrase) const OVERRIDE {
    return wstringToPhrase(ToWideString(word), phrase);
  }

  bool fieldToPhrase(Field word, AcceptString const& phrase) const OVERRIDE {
    return wstringToPhrase(ToWideString(word), phrase);
  }

  bool stringToPhrase(std::string const& word, AcceptString const& phrase) const OVERRIDE {
    return wstringToPhrase(ToWideString(word), phrase);
  }

  bool symToPhrase(Sym sym, IVocabulary& voc, AcceptString const& phrase) const OVERRIDE {
    return wstringToPhrase(ToWideString(voc.str(sym)),
                           phrase);  // we already copy to Unicodes, so no additional copy needed
  }

  bool unicodesToPhrase(Unicodes const& word, AcceptString const& phrase) const OVERRIDE {
    return wstringToPhrase(ToWideString(word), phrase);
  }
};


}}

#endif
