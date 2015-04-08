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

    a few common AcceptString.
*/

#ifndef ACCEPTSTRINGIMPLS_JG_2013_12_15_HPP
#define ACCEPTSTRINGIMPLS_JG_2013_12_15_HPP
#pragma once

#include <sdl/Util/AcceptString.hpp>
#include <sdl/Util/StringBuilder.hpp>
#include <sdl/Vocabulary/ToSymbols.hpp>
#include <sdl/Util/Add.hpp>

namespace sdl {
namespace Util {

struct AcceptSingle : IAcceptString {
  char const* type() const { return "AcceptSingle"; }

  std::string& out;
  AcceptSingle(std::string& out) : out(out) {}
  void operator()(std::string const& str) const { out = str; }
  void operator()(Slice const& word) const { out.assign(word.first, word.second); }
};

struct AcceptSingleCheck : IAcceptString {
  char const* type() const { return "AcceptSingleCheck"; }

  std::string& out;
  mutable Flag got;
  AcceptSingleCheck(std::string& out) : out(out) {}
  void operator()(std::string const& str) const {
    check();
    out = str;
  }
  void operator()(Slice const& word) const {
    check();
    out.assign(word.first, word.second);
  }
  void require() const {
    if (!got)
      SDL_THROW_LOG(Util.IWordToPhrase.AcceptSingle, ProgrammerMistakeException,
                    "required an output string for input " << out);
  }

 private:
  void check() const {
    if (!got.first())
      SDL_THROW_LOG(Util.IWordToPhrase.AcceptSingle, ProgrammerMistakeException,
                    "tried to put multi-word phrase into single word - last was " << out);
  }
};


struct AcceptJoin : IAcceptString {
  char const* type() const { return "AcceptJoin"; }

  StringBuilder& stringBuilder;
  mutable bool first;
  AcceptJoin(StringBuilder& stringBuilder) : stringBuilder(stringBuilder), first(true) {}
  void operator()(Slice const& word) const { stringBuilder.space_except_first(first)(word); }
  void operator()(std::string const& str) const { stringBuilder.space_except_first(first)(str); }
};

struct AcceptConcat : IAcceptString {
  char const* type() const { return "AcceptConcat"; }

  StringBuilder& stringBuilder;
  AcceptConcat(StringBuilder& stringBuilder) : stringBuilder(stringBuilder) {}
  void operator()(std::string const& str) const { stringBuilder(str); }
  void operator()(Slice const& word) const { stringBuilder(word); }
};

struct AcceptPushBack : IAcceptString {
  char const* type() const { return "AcceptPushBack"; }

  Strings& vec;
  AcceptPushBack(Strings& vec) : vec(vec) {}
  void operator()(std::string const& str) const { vec.push_back(str); }
  void operator()(Slice const& word) const { vec.push_back(std::string(word.first, word.second)); }
};

struct PushBackWithSpan {
  char const* type() const { return "PushBackWithSpan"; }
  bool finishPhrase(float cost = 0) const { return false; }
  Strings& tokens_;
  TokenSpans* spans_;
  bool spans() const { return spans_; }
  PushBackWithSpan(Strings& tokens, TokenSpans* spans = 0) : tokens_(tokens), spans_(spans) {}
  void operator()(std::string const& word) const {
    if (spans_) SDL_THROW_LOG(AcceptString, ConfigException, "needs span");
    tokens_.push_back(word);
  }
  void operator()(Slice const& word) const {
    if (spans_) SDL_THROW_LOG(AcceptString, ConfigException, "needs span");
    tokens_.push_back(std::string(word.first, word.second));
  }
  void operator()(Slice const& word, TokenSpan span) const {
    if (spans_) spans_->push_back(span);
    tokens_.push_back(std::string(word.first, word.second));
  }
  void operator()(std::string const& word, TokenSpan span) const {
    if (spans_) spans_->push_back(span);
    tokens_.push_back(word);
  }
  void operator()(Unicode c, Position pos) const {
    if (spans_) spans_->push_back(TokenSpan(pos, pos + 1));
    tokens_.push_back(Util::utf8s(c));
  }
  void operator()(Pchar a, Pchar b) const { operator()(Slice(a, b)); }
  void operator()(Pchar a, Pchar b, TokenSpan span) const { operator()(Slice(a, b), span); }
};

struct AcceptPushBackWithSpan : PushBackWithSpan, IAcceptString {
  char const* type() const { return "AcceptPushBackWithSpan"; }
  AcceptPushBackWithSpan(Strings& vec, TokenSpans* spans = 0) : PushBackWithSpan(vec, spans) {}
  bool spans() const { return PushBackWithSpan::spans(); }
  void operator()(std::string const& word) const { PushBackWithSpan::operator()(word); }
  void operator()(Slice const& word) const { PushBackWithSpan::operator()(word); }
  void operator()(Slice const& word, TokenSpan span) const { PushBackWithSpan::operator()(word, span); }
  virtual void operator()(std::string const& word, TokenSpan span) const {
    PushBackWithSpan::operator()(word, span);
  }
};

struct AppendSym : IAcceptString {
  char const* type() const { return "AppendSym"; }

  Syms& syms;
  AddSymbol addSymbol;

  IVocabulary& vocabulary() const { return *addSymbol.pVoc; }

  template <class AddSymbolConstruct>
  AppendSym(Syms& syms, AddSymbolConstruct const& addSymbol)
      : syms(syms), addSymbol(addSymbol) {}

  void operator()(Sym sym) const { syms.push_back(sym); }

  void operator()(std::string const& str) const { syms.push_back(addSymbol(str)); }

  void operator()(Unicodes const& unicodes) const {
    std::string str;
    Util::fromUnicodes(str, unicodes);
    syms.push_back(addSymbol(str));
  }
};


}}

#endif
