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

    parse token sequence string into hypergraph (maybe with alignment features)
*/


#ifndef HYP__HYPERGRAPH_STRINGTOHYPERGRAPH_HPP
#define HYP__HYPERGRAPH_STRINGTOHYPERGRAPH_HPP
#pragma once

#include <sdl/Hypergraph/FeatureWeightUtil.hpp>
#include <sdl/Hypergraph/FeaturesPerInputPosition.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IsFeatureWeight.hpp>
#include <sdl/Hypergraph/MixFeature.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Tokens.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Exception.hpp>
#include <sdl/LexicalCast.hpp>
#include <sdl/SharedPtr.hpp>
#include <boost/noncopyable.hpp>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace sdl {
namespace Hypergraph {


/**
   Options for StringToHypergraph function; the inputFeatures
   object will be deleted on destruction.

struct UnkOptions {
  Sym terminalMaybeUnk(IVocabulary* pVoc, std::string const& token) const {
    if (doAddUnknownSymbols)
      return pVoc->addTerminal(token);
    else {
      Sym const sym = pVoc->terminal(token);
      return sym ? sym : UNK::ID;
    }
  }
  bool doAddUnknownSymbols = true;
};

 */
struct StringToHypergraphOptions {
  StringToHypergraphOptions() : inputFeatures(new NoFeatures()) {}

  StringToHypergraphOptions(IFeaturesPerInputPosition* newInputFeatures) : inputFeatures(newInputFeatures) {}

  explicit StringToHypergraphOptions(Hypergraph::TokensPtr const& tokens, IFeaturesPerInputPosition* feats)
      : tokens(tokens), inputFeatures(feats) {}

  Hypergraph::TokensPtr tokens;

  // Determines what features to put on what input position.
  shared_ptr<IFeaturesPerInputPosition> inputFeatures;
};


typedef std::vector<FeatureValue> TokenCosts;

struct TokenWeights {
  TokenWeights() : costs_() {}
  TokenWeights(FeatureValue const* costs, MixFeature<> const& mix) : costs_(costs), mixCost_(&mix) {}

  FeatureValue const* costs_;
  MixFeature<> const* mixCost_;

  template <class Weight>
  void reweight(Position i, Weight& weight) const {
    if (costs_) (*mixCost_)(costs_[i], weight);
  }
};

struct SymsFromStrings {
  IVocabulary* voc;
  span<std::string const> strings;

  SymsFromStrings(IVocabulary* voc, span<std::string const> const& strings) : voc(voc), strings(strings) {}
  Sym operator[](Position i) const { return voc->addTerminal(strings[i]); }
  Position size() const { return strings.size(); }
};

/**
   Converts a vector of strings, inputTokens to a flat-line hypergraph. Gives all
   non-lexical states lower state IDs than lexical states, which is
   what most algorithms need.

   \return number of tokens
 */
template <class Arc, class Syms>
Position symsToHypergraph(Syms const& inputTokens, IMutableHypergraph<Arc>* pHgResult,
                          StringToHypergraphOptions const& opts = StringToHypergraphOptions(),
                          TokenWeights const& inputWeights = TokenWeights()) {
  for (Position i = 0, numNonlexicalStates = inputTokens.size() + 1; i < numNonlexicalStates; ++i)
    pHgResult->addState();
  pHgResult->setStart(0);
  StateId prevState = 0;

  typedef typename Arc::Weight Weight;
  typedef FeatureInsertFct<Weight> FI;
  Position i = 0, n = inputTokens.size();
  for (; i < n; ++i) {
    const Sym sym = inputTokens[i];
    const StateId nextState = prevState + 1;
    Arc* pArc = new Arc(nextState, Tails(prevState, pHgResult->addState(sym)));
    Weight& weight = pArc->weight();
    assert(opts.inputFeatures != NULL);
    for (FeatureId featureId : opts.inputFeatures->getFeaturesForInputPosition(i)) {
      FI::insertNew(&weight, featureId, 1);
      if (opts.tokens) opts.tokens->insert(sym, featureId);
    }
    inputWeights.reweight(i, weight);
    pHgResult->addArc(pArc);
    prevState = nextState;
  }
  pHgResult->setFinal(prevState);
  return n;
}

template <class Arc, class Strings>
Position stringToHypergraph(Strings const& inputTokens, IMutableHypergraph<Arc>* pHgResult,
                            StringToHypergraphOptions const& opts = StringToHypergraphOptions(),
                            TokenWeights const& inputWeights = TokenWeights()) {
  return symsToHypergraph(SymsFromStrings(pHgResult->vocab(), inputTokens), pHgResult, opts, inputWeights);
}

/**
   take a utf8 string and break it into single-codepoint symbols, creating a straight-line hypergraph in
   \param pHgResult
*/
template <class Arc>
Position stringToHypergraph(std::string const& utf8string, IMutableHypergraph<Arc>* pHgResult,
                            StringToHypergraphOptions const& opts = StringToHypergraphOptions(),
                            TokenWeights const& inputWeights = TokenWeights()) {
  std::vector<std::string> utf8chars;
  Util::toUtf8Chs(utf8string, utf8chars);
  return stringToHypergraph(utf8chars, pHgResult, opts, inputWeights);
}

/**
   Creates a flat-line FST from two strings.
 */
template <class Arc, class Syms, class SymsOut>
void symsPairToFst(Syms const& inputTokens, SymsOut const& outputTokens, IMutableHypergraph<Arc>* pHgResult,
                   StringToHypergraphOptions const& opts = StringToHypergraphOptions()) {
  Position N = outputTokens.size();
  if (inputTokens.size() != N)
    SDL_THROW_LOG(Hypergraph.stringPairToFst, IndexException,
                  "The two strings must have same number of words");

  // 1. Create simple FSA from input tokens:
  symsToHypergraph(inputTokens, pHgResult, opts);

// 2. Insert output tokens:
#ifndef NDEBUG
  const StateId finalId = pHgResult->final();
#endif
  StateId stateId = pHgResult->start();
  for (Position i = 0; i < N; ++i) {
    Arc* arc = pHgResult->outArc(stateId, 0);
    assert(arc);
    setFsmOutputLabel(pHgResult, *arc, outputTokens[i]);
    stateId = arc->head();
  }
#ifndef NDEBUG
  assert(stateId == finalId);
#endif
}

template <class Arc, class Strings, class StringsOut>
void stringPairToFst(Strings const& inputTokens, StringsOut const& outputTokens,
                     IMutableHypergraph<Arc>* pHgResult,
                     StringToHypergraphOptions const& opts = StringToHypergraphOptions()) {
  IVocabulary* voc = pHgResult->vocab();
  symsPairToFst(SymsFromStrings(voc, inputTokens), SymsFromStrings(voc, outputTokens), pHgResult, opts);
}


}}

#endif
