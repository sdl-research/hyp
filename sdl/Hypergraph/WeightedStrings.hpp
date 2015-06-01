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

    set of strings with optional weights (for building StringUnion). note: you can
    load a set from a file (with normalization options) via WordList.hpp

*/

#ifndef WEIGHTEDSTRINGS_JG2013215_HPP
#define WEIGHTEDSTRINGS_JG2013215_HPP
#pragma once

#include <sdl/Hypergraph/LexicalOrEpsilon.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/Sorted.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Sym.hpp>
#include <boost/range/irange.hpp>


namespace sdl {
namespace Hypergraph {


/// a set of character-token-sequences, each with a weight.
/// you can build up the set one sequence at a time with either addString() or this protocol:
/// openString(Weight), addChar(SymbolId) repeatedly, closeString().
template <class W>
struct WeightedStrings {
  typedef W Weight;
  typedef boost::tuple<Syms, W> SW;
  // typedef std::vector<SW> SWS;
  typedef std::vector<Syms> Strings;
  typedef std::vector<W> Weights;
  Strings strings;
  Weights weights;  // parallel arrays

  IVocabularyPtr voc;
  void addString(Syms const& s, W w = W::one()) {
    strings.push_back(s);
    weights.push_back(w);
  }
  void openString(W w = W::one()) { addString(Syms(), w); }
  void closeString() {}
  Sym charId(std::string const& s) const { return lexicalSymbol(s, *voc); }
  bool operator()(std::string const& s) {
    if (!s.empty()) addChar(charId(s));
    return true;
  }
  void addChar(std::string const& s) {
    SDL_TRACE(Hypergraph.WeightedStrings, "addChar " << s);
    addChar(charId(s));
  }
  typedef unordered_set<Sym> Unigrams;
  Unigrams unigrams;
  void addChar(Sym s) {
    if (unigramk) bool added = unigrams.insert(s).second;
    strings.back().push_back(s);
  }
  std::size_t addedLength() const { return strings.back().size(); }
  typedef std::vector<double> Counts;  // note: counts may be empty if unigram_addk==0 (default)
  void doneAdding() {
    Util::sort(strings);  // not lexic. order by string order, but by symbolid
  }
  void doneAdding(Counts& counts) {
    Util::sortParallelArrays(strings, counts);
    if (unigramk) {
      for (std::size_t i = 0, N = (std::size_t)strings.size(); i < N; ++i) {
        Syms const& w = strings[i];
        if (w.size() == 1) {
          Sym s = w.front();
          assert(Util::contains(unigrams, s));
          counts[i] += unigramk;
          unigrams.erase(s);
        }
      }
      double k = unigramk;
      unigramk = 0;  // prevent adding more counts as we add unseen as words
      for (typename Unigrams::const_iterator i = unigrams.begin(), e = unigrams.end(); i != e; ++i) {
        Sym s = *i;
        counts.push_back(k);
        addString(Syms(1, s));
        assert(counts.size() == strings.size());
        assert(counts.size() == weights.size());
      }
    }
  }
  std::size_t size() const {
    assert(weights.size() == strings.size());
    return strings.size();
  }
  double unigramk;
  WeightedStrings(IVocabularyPtr voc, double unigramk = 0) : voc(voc), unigramk(unigramk) {}
  Util::RangeSep sepchars;
  void print(std::ostream& o, Syms const& s) const { Util::print(o, s, stateRange(voc, sepchars)); }
  void print(std::ostream& o, std::size_t i) const {
    assert(i < size());
    o << weights[i] << " ";
    print(o, strings[i]);
  }
  friend inline void print(std::ostream& o, std::size_t i, WeightedStrings<W> const& ws) { ws.print(o, i); }
  void print(std::ostream& o, Util::RangeSep so = Util::multiLine()) const {
    Util::printRangeState(o, *this, boost::irange((std::size_t)0, size()), so);
  }
  friend std::ostream& operator<<(std::ostream& o, WeightedStrings<W> const& ws) {
    ws.print(o);
    return o;
  }
};


}}

#endif
