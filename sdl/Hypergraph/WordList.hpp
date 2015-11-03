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

    file containing words and (with optional 'counts' number prefix)
*/

#ifndef HYP__WORDLIST_HPP
#define HYP__WORDLIST_HPP
#pragma once

#include <iostream>
#include <string>
#include <graehl/shared/normalize_range.hpp>
#include <graehl/shared/split_noquote.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/InvalidInputException.hpp>
#include <sdl/Hypergraph/WeightedStrings.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>

#include <boost/range/algorithm/transform.hpp>
#include <sdl/Util/LineOptions.hpp>

namespace sdl {
namespace Hypergraph {

VERBOSE_EXCEPTION_DECLARE(TrieWordListException)

typedef graehl::normalize_options<double> NormOpt;

struct WordListOptions : Util::LineOptions {
  NormOpt normalizeOpt;
  bool chars;
  std::string wordsep;
  std::size_t maxlines;
  double unigram_addk;
  double lengthBase;
  double maxLength;
  bool counts;
  bool enablenormalize;
  static char const* caption() { return "Word List Input Format"; }
  WordListOptions()
      : chars(true)
      , wordsep(" ")
      , maxlines((std::size_t)-1)
      , unigram_addk(.001)
      , lengthBase(1)
      , maxLength(10)
      , counts(true)
      , enablenormalize(true) {}
  std::string usage() const { return "input list of lines: COUNT WORD (if --chars) else WORD*"; }
  template <class Config>
  void configure(Config& c) {
    Util::LineOptions::configure(c);
    normalizeOpt.configure(c);
    c("enable-normalize",
      &enablenormalize)("normalize counts (true). If set to false, will take scores as -log weights")
        .defaulted();
    c("counts", &counts)("wordlist has counts (implicit count of 1 is used if false)").defaulted();
    c("unigram-addk", &unigram_addk)(
        "weight for unigram backoff - take all known chars and add this count - even if the char never "
        "appeared as a word (had count 0). this should be >0 or else there may be character sequences that "
        "have no derivations")
        .defaulted();
    c('c')("chars", &chars)("treat input as utf8 characters (else space separated words)").defaulted();
    c("wordsep", &wordsep)("if not chars, then separate tokens by this string").defaulted();
    c("length-base", &lengthBase)(
        "scale counts based on length of word: L^n where word is n long; 1 means neutral, <1 rewards shorter "
        "words. note: applies to unigram-addk too")
        .defaulted();
    c("max-length-exponent",
      &maxLength)("limit the length M used in B^M to this - result should fit in double")
        .defaulted();
  }
};

template <class W>
void readWordList(std::istream& in, WeightedStrings<W>& ws, WordListOptions const& opt = WordListOptions(),
                  bool unweighted = false) {
  assert(ws.size() == 0);
  ws.unigramk = opt.unigram_addk * opt.lengthBase;
  typename WeightedStrings<W>::Counts counts;
  SDL_TRACE(Hypergraph.WordList, "normalized counts: " << sdl::printer(counts, Util::multiLine()));
  std::string tok;
  for (std::size_t i = 0; i < opt.maxlines && in; ++i) {
    double c = 1;
    if (!opt.counts || in >> c) {
      if (opt.chars) {
        if (!(in >> tok)) {
          if (!opt.counts) break;
          SDL_THROW_LOG(TrieWordList, TrieWordListException,
                        "reading word list: got count "
                            << c << " but no word (looking for non-whitespace utf8 chars on line " << i << ")");
        }
        opt.normalize(tok);
        ws.openString();
        for (std::string::const_iterator c = tok.begin(), e = tok.end(); c != e; /* utf8::next advances c */)
          ws.addChar(Util::utf8s(Util::next(c, e)));
      } else {
        if (!opt.getlineNormalized(in, tok)) {
          if (!opt.counts) break;
          SDL_THROW_LOG(TrieWordList, TrieWordListException,
                        "reading word list: got count "
                            << c << " but no word (looking for whitespace separated tokens on line " << i
                            << ")");
        }
        ws.openString();
        graehl::split_noquote(tok, [&ws](std::string const& s) {
          ws.addNonEmptyChar(s);
          return true;
        }, opt.wordsep);
      }
      double l = (double)ws.addedLength();
      if (l > opt.maxLength) l = opt.maxLength;
      double p = std::pow(opt.lengthBase, l);
      counts.push_back(c * p);
      ws.closeString();
    } else if ((in.bad() || in.fail()) && !in.eof())
      Util::throwInvalidInputException(in, "Couldn't read count (double) for TrieWordList", i);
  }
  SDL_TRACE(Hypergraph.WordList,
            "utf8 char strings: " << sdl::printer(ws.strings, Util::stateRange(ws.voc)));  // ws
  if (unweighted)
    ws.doneAdding();
  else {
    ws.doneAdding(counts);
    SDL_TRACE(Hypergraph.WordList, "pre-normalized counts: " << sdl::printer(counts, Util::multiLine()));
    if (opt.enablenormalize) {
      graehl::normalize(counts, opt.normalizeOpt);
      SDL_TRACE(Hypergraph.WordList, "normalized counts: " << sdl::printer(counts, Util::multiLine()));
      boost::transform(counts, ws.weights.begin(), ProbToNeglog<typename W::FloatT>());
    } else {
      boost::transform(counts, ws.weights.begin(), Identity<typename W::FloatT>());
    }
  }
  SDL_TRACE(Hypergraph.WordList, "final weighted strings: " << ws << "\n");  // ws
}


}}

#endif
