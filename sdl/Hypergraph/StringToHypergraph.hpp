/** \file

    parse token sequence string into hypergraph (maybe with alignment features)
*/


#ifndef HYP__HYPERGRAPH_STRINGTOHYPERGRAPH_HPP
#define HYP__HYPERGRAPH_STRINGTOHYPERGRAPH_HPP
#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <boost/noncopyable.hpp>

#include <sdl/SharedPtr.hpp>
#include <sdl/LexicalCast.hpp>
#include <sdl/Exception.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/FeatureWeightUtil.hpp>
#include <sdl/Hypergraph/IsFeatureWeight.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/Tokens.hpp>
#include <sdl/Hypergraph/FeaturesPerInputPosition.hpp>
#include <sdl/Hypergraph/MixFeature.hpp>

#include <sdl/Vocabulary/SpecialSymbols.hpp>

#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Options for StringToHypergraph function; the inputFeatures
   object will be deleted on destruction.
 */
struct StringToHypergraphOptions {

  Sym terminalMaybeUnk(IVocabulary* pVoc, std::string const& token) const {
    if (doAddUnknownSymbols)
      return pVoc->addTerminal(token);
    else {
      Sym const sym = pVoc->getTerminal(token);
      return sym ? sym : UNK::ID;
    }
  }

  StringToHypergraphOptions() : doAddUnknownSymbols(true), inputFeatures(new NoFeatures()) {}

  StringToHypergraphOptions(IFeaturesPerInputPosition* newInputFeatures)
      : doAddUnknownSymbols(true), inputFeatures(newInputFeatures) {}

  explicit StringToHypergraphOptions(Hypergraph::TokensPtr const& tokens, IFeaturesPerInputPosition* feats)
      : doAddUnknownSymbols(true), tokens(tokens), inputFeatures(feats) {}

  /*
  void setOneFeaturePerInputPosition() {
    return inputFeatures.reset(new OneFeaturePerInputPosition());
  }
  */

  bool doAddUnknownSymbols;

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


/**
   Converts a vector of strings, inputTokens to a flat-line hypergraph. Gives all
   non-lexical states lower state IDs than lexical states, which is
   what most algorithms need.
 */
template <class Arc, class Strings>
void stringToHypergraph(Strings const& inputTokens, IMutableHypergraph<Arc>* pHgResult,
                        StringToHypergraphOptions const& opts = StringToHypergraphOptions(),
                        TokenWeights const& inputWeights = TokenWeights()) {
  IVocabularyPtr const& pVoc = pHgResult->getVocabulary();
  if (!pVoc) SDL_THROW_LOG(Hypergraph, InvalidInputException, "pHgResult hypergraph must contain vocabulary");
  for (std::size_t i = 0, numNonlexicalStates = inputTokens.size() + 1; i < numNonlexicalStates; ++i)
    pHgResult->addState();
  pHgResult->setStart(0);
  StateId prevSid = 0;

  typedef typename Arc::Weight Weight;
  typedef FeatureInsertFct<Weight> FI;
  FI insertFeature;

  for (Position i = 0, n = inputTokens.size(); i != n; ++i) {
    std::string const& token = inputTokens[i];
    SDL_TRACE(Hypergraph.StringToHypergraph, i << ": " << token);
    const Sym symId = opts.terminalMaybeUnk(pVoc.get(), token);
    const StateId nextSid = prevSid + 1;
    Arc* pArc = new Arc(Head(nextSid), Tails(prevSid, pHgResult->addState(symId)));
    Weight& weight = pArc->weight();
    assert(opts.inputFeatures != NULL);
    forall (FeatureId featureId, opts.inputFeatures->getFeaturesForInputPosition(i)) {
      insertFeature(&weight, (typename FI::key_type)featureId, 1);
      if (opts.tokens) opts.tokens->insert(symId, (typename FI::key_type)featureId);
    }
    inputWeights.reweight(i, weight);
    pHgResult->addArc(pArc);
    prevSid = nextSid;
  }
  pHgResult->setFinal(prevSid);
}

/**
   take a utf8 string and break it into single-codepoint symbols, creating a straight-line hypergraph in
   \param pHgResult
*/
template <class Arc>
void stringToHypergraph(std::string const& utf8string, IMutableHypergraph<Arc>* pHgResult,
                        StringToHypergraphOptions const& opts = StringToHypergraphOptions(),
                        TokenWeights const& inputWeights = TokenWeights()) {
  std::vector<std::string> utf8chars;
  Util::toUtf8Chs(utf8string, utf8chars);
  stringToHypergraph(utf8chars, pHgResult, opts, inputWeights);
}

/**
   Creates a flat-line FST from two strings.
 */
template <class Arc, class Strings>
void stringPairToFst(Strings const& inputTokens, std::vector<std::string> const& outputTokens,
                     IMutableHypergraph<Arc>* pHgResult,
                     StringToHypergraphOptions const& opts = StringToHypergraphOptions()) {
  if (inputTokens.size() != outputTokens.size()) {
    SDL_THROW_LOG(Hypergraph.stringPairToFst, IndexException,
                  "The two strings must have same number of words");
  }

  // 1. Create simple FSA from input tokens:
  stringToHypergraph(inputTokens, pHgResult, opts);

  // 2. Insert output tokens:
  IVocabularyPtr pVoc = pHgResult->getVocabulary();
  std::vector<std::string>::const_iterator it = outputTokens.begin();
  StateId stateId = pHgResult->start();
  const StateId finalId = pHgResult->final();
  while (stateId != finalId) {
    Arc* arc = pHgResult->outArc(stateId, 0);
    const Sym symId = opts.terminalMaybeUnk(pVoc.get(), *it);
    setFsmOutputLabel(pHgResult, *arc, symId);
    ++it;
    stateId = arc->head();
  }
}


}}

#endif
