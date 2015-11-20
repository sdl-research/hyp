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

    specify mapping from input file lines into tokens (chars or words).
*/

#ifndef HYP__HYPERGRAPH_PARSE_TOKENS_HPP
#define HYP__HYPERGRAPH_PARSE_TOKENS_HPP
#pragma once

#include <sstream>
#include <vector>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Config/Init.hpp>
#include <sdl/Util/LineOptions.hpp>
#include <sdl/Util/Split.hpp>

namespace sdl {
namespace Hypergraph {

typedef std::vector<std::string> Strings;

struct Utf8CharTokens {  // each char is a token
  void parse(std::string const& line, Strings& words) const { Util::toUtf8Chs(line, words); }
};

/// skips empty tokens
struct SpaceSepTokens {  // unquoted, separated only by space chars
  void parse(std::string const& line, Strings& words) const { Util::splitSpaces(words, line); }
};

struct QuotedTokens {  // according to ArcParser grammar for lexical tokens
  void parse(std::string const& line, Strings& words) const {
    SDL_THROW_LOG(Util.ParseTokens, UnimplementedException,
                  "TODO: QuotedTokens::parse needs ArcParse boost::qi grammar");
  }
};

enum TokenType { kUtf8CharTokens, kSpaceSepTokens, kQuotedTokens };

struct ParseTokensOptions : Util::LineOptions {
  enum { kWords = false, kCharacters = true };
  bool characterBased, quoted;
  ParseTokensOptions(bool characters = false, bool trimWs = true) {
    defaults();
    characterBased = characters;
    trim = trimWs;
  }
  void defaults() {
    characterBased = false;
    trim = true;
    Util::LineOptions::defaults();
    Config::inits(this);
  }
  template <class Config>
  void configure(Config& config) {
    Util::LineOptions::configure(config);
    config(usage());
    config("chars", &characterBased)('c')
        .defaulted()("(utf8) character tokens instead of space-separated (quoted?) strings");
    config("quoted", &quoted)('q')
        .init(false)("parse quoted tokens (else just space-separated, unquoted strings)");
  }
  static inline std::string usage() { return "parse text lines into tokens"; }
  TokenType tokenType() const {
    return characterBased ? kUtf8CharTokens : quoted ? kQuotedTokens : kSpaceSepTokens;
  }
};

ParseTokensOptions const kCharsHg(true, false);

// we just return the sequence, rather than providing an output iterator or Generator. don't expect to operate
// on any long seqs yet. also, input is always a string (in future, template on iterator incl input iter)

template <class Tokenize>
Strings parseTokens(std::string const& line, Tokenize const& t) {
  Strings ret;
  t.parse(line, ret);
  return ret;
}

inline Strings parseTokens(std::string const& line, TokenType tokenType) {
  switch (tokenType) {
    case kUtf8CharTokens:
      return parseTokens(line, Utf8CharTokens());
    case kSpaceSepTokens:
      return parseTokens(line, SpaceSepTokens());
    case kQuotedTokens:
      return parseTokens(line, QuotedTokens());
    default:
      SDL_THROW(BadTypeConstantException, tokenType);
  }
};

inline Strings parseTokens(std::string line, ParseTokensOptions const& opt = ParseTokensOptions()) {
  TokenType const tokenType = opt.tokenType();
  opt.normalize(line);
  return parseTokens(line, tokenType);
};


}}

#endif
