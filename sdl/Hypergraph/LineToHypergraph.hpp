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

    options for parsing text symbol or char sequences into hypergraphs.
*/

#ifndef HYP__HG_LINE_TO_HYPERGRAPH_HPP
#define HYP__HG_LINE_TO_HYPERGRAPH_HPP
#pragma once

#include <sdl/LexicalCast.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/StringToHypergraph.hpp>
#include <sdl/Hypergraph/ParseTokens.hpp>
#include <sdl/Config/Init.hpp>


namespace sdl {
namespace Hypergraph {

/// maybe the input is a line of text; maybe it's a hypergraph. LineToHypergraph lets the user choose which.
struct LineToHypergraph : ParseTokensOptions
{
  bool reuse;
  Properties hgProperties;
  static inline std::string usage() {
    return ParseTokensOptions::usage()
        + ", converting input to single-string hypergraphs"
        ;
  }

  LineToHypergraph(ParseTokensOptions const& parse, Properties hgProperties_ = kDefaultArcProperties)
      : ParseTokensOptions(parse)
      , reuse(true)
      , hgProperties(hgProperties_)
  {}

  LineToHypergraph(bool characters = false, Properties hgProperties_ = kDefaultArcProperties)
      : ParseTokensOptions(characters)
  {
    Config::inits(this);
    setProperties(hgProperties_);
  }

  void setChars()
  {
    characterBased = true;
  }

  //same as constructing a new LineToHypergraph object and assigning to *this
  void setProperties(Properties hgProperties_ = kDefaultArcProperties) {
    hgProperties = hgProperties_;
  }

  static char const* caption() { return "Line to Hypergraph"; }
  template <class Config>
  void configure(Config &c)
  {
    ParseTokensOptions::configure(c);
    c.is(caption());
    c(caption());
    c("reuse-words", &reuse).init(true).verbose()
        ("reuse repeated words' states");
  }

  enum { kDefaultArcProperties = kStoreInArcs|kStoreOutArcs };

  Properties properties() const
  {
    return kGraph | kFsm | hgProperties | (reuse?kCanonicalLex:0);
  }

  template <class A>
  void toHypergraph(std::string const& line, IMutableHypergraph<A> *phg, std::size_t lineNum = 0) const
  {
    Strings words = parseTokens(line, (ParseTokensOptions const&)*this);
    SDL_DEBUG(Hypergraph.HgConvertString, lineNum << ": " << sdl::printer(words, Util::RangeSep(" ","","")));
    SDL_INFO(Hypergraph.HgConvertString, lineNum << ": len=" << words.size());
    phg->clear(properties());
    assert(phg->storesArcs());
    assert(phg->getVocabulary());
    stringToHypergraph(words, phg);
  }

  template <class A>
  shared_ptr<IHypergraph<A> > makeHypergraph(std::string const& line, IVocabularyPtr const& vocab,
                                             Properties prop=kFsmOutProperties, std::size_t lineNum = 0) const
  {
    assert(vocab);
    shared_ptr<MutableHypergraph<A> > const& hg = make_shared<MutableHypergraph<A> >(prop);
    hg->setVocabulary(vocab);
    toHypergraph(line, hg.get(), lineNum);
    return static_pointer_cast<IHypergraph<A> >(hg);
  }

};


}}

#endif
