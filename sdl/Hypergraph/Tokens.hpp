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

    save the (input) tokens, assigning indicator features into a configurable
    range
*/

#ifndef TOKENS_JG2013130_HPP
#define TOKENS_JG2013130_HPP
#pragma once

#include <vector>
#include <sdl/Syms.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/FeatureIdRange.hpp>
#include <sdl/Span.hpp>
#include <sdl/Hypergraph/FeatureWeightUtil.hpp>

namespace sdl {
namespace Hypergraph {

struct Tokens {
  typedef FeatureId FeatureOffset;

  Tokens(IVocabularyPtr const& vocab, FeatureIdRange const& ids) : vocab(vocab), ids(ids) {
    symbols.reserve(1000);
  }

  Sym symbolForOffset(FeatureOffset offset) const {
    if (offset >= symbols.size())
      SDL_THROW_LOG(Hypergraph.Tokens, InvalidInputException,
                    "alignments info is incomplete for alignment word #"
                        << offset << " - no aligned src word for id=" << offset + ids.begin);
    return symbols[offset];
  }

  std::string const& stringForOffset(FeatureOffset offset) const {
    return vocab->str(symbolForOffset(offset));
  }

  Sym symbolForId(FeatureId id) const { return symbolForOffset(ids.offset(id)); }

  std::string const& stringForId(FeatureId id) const { return vocab->str(symbolForId(id)); }

  friend inline std::ostream& operator<<(std::ostream& out, Tokens const& self) {
    self.print(out);
    return out;
  }

  void print(std::ostream& out) const {
    out << Util::print(symbols, Util::stateRange(vocab)) << "(" << ids.begin << " indexed)";
  }

  void push_back(std::string const& symbol) { symbols.push_back(vocab->add(symbol, kTerminal)); }

  void operator()(Unicode c, Position) const { symbols.push_back(vocab->add(c, kTerminal)); }

  void operator()(std::string const& word, TokenSpan) const {
    symbols.push_back(vocab->add(word, kTerminal));
  }

  void operator()(Slice const& word, TokenSpan) const { symbols.push_back(vocab->add(word, kTerminal)); }

  void finishPhrase(float cost = 0) const {}

  void push_back(Sym id) { symbols.push_back(id); }

  FeatureId add(Sym sym) {
    FeatureId const id = ids.begin + symbols.size();
    if (id >= ids.end)
      SDL_THROW_LOG(Hypergraph.Tokens, ConfigException, "too many lexical input arcs for configured range of "
                                                            << ids);
    symbols.push_back(sym);
    return id;
  }

  void insert(Sym symId, FeatureId featId) {
    assert(ids.contains(featId));
    FeatureId const index = featId - ids.begin;
    symbols.set_grow(index, symId, NoSymbol);
  }

  FeatureId size() const { return (FeatureId)symbols.size(); }
  FeatureIdRange ids;
  mutable Syms symbols;
  IVocabularyPtr vocab;
};

typedef shared_ptr<Tokens> TokensPtr;

/// for FeatureWeight arcs, get the single alignment-id indexed TokenSpan
struct GetTokenSpan {
  typedef shared_ptr<TokenSpans> SpansPtr;

  GetTokenSpan() {}

  explicit GetTokenSpan(SpansPtr const& spans, FeatureIdRange const& ids) : spans(spans), ids(ids) {}

  SpansPtr spans;
  FeatureIdRange ids;

  mutable TokenSpan span;
  operator TokenSpan const&() const { return span; }

  bool enabled() const { return (bool)spans; }
  void disable() { spans.reset(); }

  bool haveSpan() const { return !nullTokenSpan(span); }

  //TODO: test
  template <class Arc>
  void operator()(Arc* arc) const {
    setNullTokenSpan(span);
    Hypergraph::visitFeatureRange(arc->weight(), *this, ids);
  }

  //TODO: test
  template <class Arc>
  TokenSpan const& spanFor(Arc* arc) const {
    operator()(arc);
    return span;
  }

  //TODO: test
  TokenSpan const& spanForFeature(FeatureId id) const {
    FeatureId const index = id - ids.begin;
    if (index >= spans->size())
      SDL_THROW_LOG(Hypergraph.Tokens, ProgrammerMistakeException,
                    "token feature id=" << id << " index=" << index << " is unknown to TokenSpans of size "
                                        << spans->size());
    return (*spans)[index];
  }

  //TODO: test
  void operator()(std::pair<FeatureId const, FeatureValue> const& idVal) const {
    FeatureId const id = idVal.first;
    if (haveSpan())
      SDL_THROW_LOG(Hypergraph.Tokens, ConfigException,
                    "second token feature id: " << id << " (should only have one per arc)");
    span = spanForFeature(id);
  }

  struct SpansForStates {
    GetTokenSpan const& getTokenSpan_;
    TokenSpans& spans_;
    IHypergraphStates const& hg_;
    template <class Arc>
    SpansForStates(GetTokenSpan const& getTokenSpan, TokenSpans& spans, IHypergraph<Arc> const& hg)
        : getTokenSpan_(getTokenSpan), spans_(spans), hg_(hg) {
      spans_.clear();
      spans_.resize(hg.size(), kNullTokenSpan);
      hg.forArcs(*this);
    }
    template <class Arc>
    void operator()(Arc* arc) const {
      StateIdContainer const& tails = arc->tails();
      TokenSpan const& span = getTokenSpan_.spanFor(arc);
      if (!nullTokenSpan(span)) {
        for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
          StateId const tail = *i;
          if (hg_.hasLexicalLabel(tail)) {
            TokenSpan& storeSpan = spans_[tail];
            if (!nullTokenSpan(storeSpan) && storeSpan != span)
              SDL_WARN(Hypergraph.Tokens, "(multiple token spans for input tail "
                                              << tail << ": " << PrintSpan(span) << " differs from previous "
                                              << PrintSpan(storeSpan));
            storeSpan = span;
          }
        }
      }
    }
  };

  template <class Hg>
  void spansForStates(Hg const& hg) const {
    if (spans) SpansForStates(*this, *spans, hg);
  }
};

/**
   arc visiting functor - for lexical labeled arcs, save symbol with new id in Tokens.
*/
template <class Arc>
struct SaveTokens {
  TokensPtr pTokens;

  mutable FeatureId nextId;  // so we can be const in visitor

  IHypergraph<Arc> const& hg;

  explicit SaveTokens(IHypergraph<Arc> const& hg, FeatureIdRange const& ids)
      : hg(hg), pTokens(new Tokens(hg.getVocabulary(), ids)), nextId(ids.begin) {
    SDL_DEBUG(SaveTokens, "Saving lexical tokens (first per arc only) to alignment ids " << ids);
    hg.visitArcs(*this);
  }
  void operator()(Arc* arc) const {
    // Limitation: At most one lexical tail per arc with alignment features
    // (http://jira.global.sdl.corp:8080/jira/browse/CM-237)
    Sym const sym = hg.firstLexicalInput(arc);
    if (sym) insertFeature(&arc->weight(), pTokens->add(sym), (FeatureValue)1);
  }
};


}}

#endif
