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

    The functions in this file (convertCharsToTokens and
    detokenize) rely on the TokenWeight

    \author Markus Dreyer, Jonathan Graehl
*/

#include <set>
#include <vector>
#include <algorithm>

#include <sdl/Exception.hpp>
#include <sdl/Sym.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>


#include <sdl/Util/LogHelper.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/TokenWeight.hpp>
#include <sdl/Hypergraph/TokenWeightMapper.hpp>
#include <sdl/Hypergraph/MapHypergraph.hpp>
#include <sdl/Hypergraph/StatesTraversal.hpp>
#include <sdl/Hypergraph/InsideAlgorithm.hpp>
#include <sdl/Util/StringBuilder.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/String.hpp>
#include <sdl/Vocabulary/Glue.hpp>

namespace sdl {
namespace Hypergraph {

// Namespace for functions that assemble tokens in a hypergraph.
namespace AssembleTokensUtil {

/**
   To be used in StatesVisitor: For each state, inspects the
   incoming tokens (i.e., the tokens that end there) and constructs an
   arc in the resulting hypergraph accordingly.

   The incoming tokens for each state are passed as a vector of
   TokenWeight. That vector is the result of running the inside
   algorithm using the TokenWeight semiring.

   \tparam TW: TokenWeight e.g. TokenWeightTpl<FeatureWeight>
*/
template <class Arc, class TW>
struct ConstructResultArcForeachIncomingToken : public IStatesVisitor {


  //  typedef std::set<StateId> StateSet;
  typedef TW TokenWeight;
  typedef typename TW::Weight Weight;


  bool keep(Sym sym) const {
    assert(!Vocabulary::isBlockSymbolOrJumpWall(sym));
    return !sym.isSpecial() || sym == GLUE::ID;
  }

  /**
     Default token -> std::string converter (for Token, see
     TokenWeight.hpp); ignores all special symbols (EPSILON, TOK_START,
     etc.) except GLUE and xmt-block
  */
  void str(Syms const& syms, std::string& r) const {
    assert(r.empty());
    unsigned n = syms.size();
    if (n <= 1) {
      if (n && keep(syms[0])) r = invoc_->str(syms[0]);
      return;
    }
    for (Syms::const_iterator i = syms.begin(), e = syms.end(); i != e; ++i) {
      if (keep(*i)) r += invoc_->str(*i);
    }
    return;
  }

  /**
     \pFctConvertTokenToString A functor that converts a Token object
     to a string, e.g., DefaultTokenToStringConverter. We take
     ownership and delete.
  */
  ConstructResultArcForeachIncomingToken(IHypergraph<Arc> const& hg,
                                         boost::ptr_vector<TokenWeight> const& incomingTokenWeights,
                                         IMutableHypergraph<Arc>* pHgResult)
      : inhg_(hg)
      , invoc_(hg.getVocabulary().get())
      , incomingTokenWeights_(incomingTokenWeights)
      , outhg_(pHgResult)
      , outvoc_(pHgResult->getVocabulary().get())
      , oldStateToNewState_(pHgResult)
      , instart_(hg.start())
      , infinal_(hg.final())
      , inarcs_(hg.inArcs()) {
    if (instart_ == kNoState) {
      --instart_;
      assert(instart_ == kImpossibleNotNoState);
    }
    assert(hg.storesInArcs());
    assert(outvoc_ != 0);

    StateId oldStart = inhg_.start();
    if (oldStart != kNoState) {
      StateId newStart = oldStateToNewState_.stateFor(oldStart);
      outhg_->setStart(newStart);
    }
    StateId oldFinal = inhg_.final();
    if (oldFinal != kNoState) {
      StateId newFinal = oldStateToNewState_.stateFor(oldFinal);
      outhg_->setFinal(newFinal);
    }
  }

  /**
     \return true If token must extend left (e.g., __LW_AT__x) but is
     at start of hypergraph.
  */
  bool isBadStartToken(Hypergraph::Token const& tok) const {
    return tok.mustExtendLeft() && tok.start() == instart_;
  }

  /**
     \return true If token must extend right (e.g., x__LW_AT__) but is
     at end of hypergraph (or only <eps> up to final state).
     *
     This works even if the bad token in the input hypergraph does not
     directly lead to the final state, but leads to a state from which
     there are <eps> arcs to the final state, because the inside
     algorithm will still store this token as an incoming token into
     the final state (because it handles <eps> correctly).
  */
  bool isBadFinalToken(Hypergraph::Token const& tok) const {
    return tok.mustExtendRight() && tok.endState() == infinal_;
  }

  /**
     \return true If is a bad start or a bad final token.
  */
  bool isTokenImpossibleToComplete(Token const& tok) const {
    return isBadStartToken(tok) || isBadFinalToken(tok);
  }

  void visit(StateId stateid) {
    SDL_TRACE(Hypergraph.ConvertCharsToTokens, "Visiting state " << stateid);
    if (stateid >= (*inarcs_).size() || (*inarcs_)[stateid].empty()) return;

    // Construct incoming tokens
    std::size_t cntIncomingTokens = 0;
    TokenWeight const& tokenWeight = incomingTokenWeights_[stateid];
    SDL_TRACE(Hypergraph.ConvertCharsToTokens, "TokenWeight(" << stateid << ") = " << tokenWeight);
    const StateId tokenEndState = stateid;

    if (!tokenWeight.isZero()) {
      SDL_TRACE(Hypergraph.ConvertCharsToTokens, "w[" << stateid << "]: " << tokenWeight);
      for (typename TokenWeight::value_type const& tokWeightPair : tokenWeight) {
        Hypergraph::Token const& tok = tokWeightPair.first;
        Syms const& toksyms = tok.syms();
        if (tok.isComplete() || isTokenImpossibleToComplete(tok)) {
          assert(!(tok.isComplete() && isTokenImpossibleToComplete(tok)));
          SDL_TRACE(Hypergraph.ConvertCharsToTokens,
                    (tok.isComplete() ? "Complete" : "")
                        << (isTokenImpossibleToComplete(tok) ? "Can't be completed" : "") << ": " << tok
                        << " weight=" << tokWeightPair.second);
          bool start = true;
          StateId to = oldStateToNewState_.stateFor(tokenEndState);
          StateId const tokstart = tok.start();
          Arc* arc = tokstart == kNoState
                         ? new Arc(HeadAndWeight(), to, tokWeightPair.second)
                         : new Arc(to, tokWeightPair.second, oldStateToNewState_.stateFor(tokstart));
          // axiom else start from (output copy of) tokstart
          Util::StringBuilder buf;
          for (Syms::const_iterator i = toksyms.begin(), e = toksyms.end();;) {
            StateIdContainer& tails = arc->tails();
            for (; i != e; ++i) {
              Sym const s = *i;
              if (Vocabulary::isBlockSymbolOrJumpWall(s)) {
                if (!buf.empty()) break;
                tails.push_back(outhg_->addState(s));
              } else if (keep(s)) {
                buf(outvoc_->str(s));
              }
            }
            if (!buf.empty()) {
              tails.push_back(outhg_->addState(outvoc_->addTerminal(buf.slice())));
              ++cntIncomingTokens;
            }  // else could add EPSILON::ID for fsm
            if (i == e) {
              SDL_TRACE(Hypergraph.ConvertCharsToTokens, "Adding arc: " << *arc);
              outhg_->addArc(arc);
              break;
            }
            StateId mid = outhg_->addState();
            arc->setHead(mid);
            SDL_TRACE(Hypergraph.ConvertCharsToTokens, "Adding arc: " << *arc);
            outhg_->addArc(arc);
            arc = new Arc(HeadAndTail(), to, mid);
            buf.clear();
          }
        }
      }
    }

    if (cntIncomingTokens) Util::setGrow(statesWithIncomingTokens_, stateid);
  }

  bool hasIncomingTokens(StateId stateid) const {
    return Util::testSparse(statesWithIncomingTokens_, stateid);
  }

  /**
     In a CFG, some states don't have incoming tokens that end
     there (e.g., the state 'S' in 'S' <- 'NP' 'VP'). Arcs to these
     states are added here.
  */
  void finishVisit() {
    SDL_TRACE(Hypergraph.ConvertCharsToTokens, "finishVisit");
    for (StateId stateid : inhg_.getStateIds()) {
      for (ArcId aid : inhg_.inArcIds(stateid)) {
        Arc* arc = inhg_.inArc(stateid, aid);
        bool allTailsOk = true;
        for (StateId tailId : arc->tails()) {
          if (!hasIncomingTokens(tailId)) {
            allTailsOk = false;
            break;
          }
        }
        if (allTailsOk) {
          outhg_->addArc(copyArcWithMappedStates(oldStateToNewState_, *arc));
        }
      }
    }
  }

  IMutableHypergraph<Arc>* getResult() const { return outhg_; }

  typedef typename IHypergraph<Arc>::AdjsPtr AdjsPtr;
  IHypergraph<Arc> const& inhg_;
  IVocabulary const* invoc_;
  boost::ptr_vector<TokenWeight> incomingTokenWeights_;
  StateSet statesWithIncomingTokens_;
  IMutableHypergraph<Arc>* outhg_;
  IVocabulary* outvoc_;
  StateIdTranslation oldStateToNewState_;
  StateId instart_, infinal_;
  AdjsPtr inarcs_;
};
}

// ConvertCharsToTokens

namespace ConvertCharsToTokensUtil {

/*
  Assigns token weights as inside weights of axioms; to be
  used in inside algorithm. Determines that a "<tok>" is extendable
  to the right, but not to the left, "</tok>" extends to the left, a
  regular lexical item (e.g. "a", "b", ...) must extend to the left
  and to the right.
*/
template <class TW>
struct AssignTokenWeight : public StateToWeight<TW> {

  typedef TW TokenWeight;
  typedef typename TokenWeight::Weight Weight;
  typedef ArcTpl<TokenWeight> Arc;

  explicit AssignTokenWeight(IHypergraph<Arc> const& hg) : hg_(hg) {}

  TokenWeight operator()(StateId stateId) const {
    if (hg_.hasTerminalLabel(stateId)) {
      Sym sym = hg_.inputLabel(stateId);
      SDL_TRACE(Hypergraph.AssignTokenWeight, "sym(" << stateId << ")=" << sym);
      typename Token::Properties tokenProps = Token::kExtendableLeft | Token::kExtendableRight
                                              | Token::kMustExtendLeft | Token::kMustExtendRight;
      if (sym == TOK_START::ID) {
        tokenProps = Token::kExtendableRight | Token::kMustExtendRight;
      } else if (sym == TOK_END::ID) {
        tokenProps = Token::kExtendableLeft | Token::kMustExtendLeft;
      } else if (sym == EPSILON::ID) {
        // <eps> is extendable, but doesn't *have* to extend
        tokenProps = Token::kExtendableLeft | Token::kExtendableRight;
      } else if (Vocabulary::isBlockSymbolOrJumpWall(sym))
        tokenProps = Token::kBlockSymbolTokenProperties;
      TokenWeight tw(Token(sym, tokenProps), Weight::one());
      SDL_TRACE(Hypergraph.AssignTokenWeight, "State " << stateId << ": Assigning token weight " << tw);
      return tw;
    }
    return TokenWeight(Token(stateId, kNoState, false), Weight::one());
  }

  IHypergraph<Arc> const& hg_;
};
}

/**
   The general implementation is as follows:

   1. Maps the HG to TokenWeight arcs (where the times() operation
   extends a token, and the plus() operation accumulates tokens).

   2. Runs inside algorithm, which, for each state, keeps track of the
   tokens that end there.

   3. Traverses states of original HG. Foreach state S: Foreach token
   that ends at S, add an arc to the result machine.

*/
template <class Arc>
void convertCharsToTokens(IHypergraph<Arc> const& hg, IMutableHypergraph<Arc>* pHgResult) {
  if (!hg.storesInArcs())
    SDL_THROW_LOG(Hypergraph.ConvertCharsToTokens, ConfigException,
                  "convert chars->tokens requires indexing in-arcs");

  using namespace ConvertCharsToTokensUtil;

  typedef typename Arc::Weight Weight;
  typedef TokenWeightTpl<Weight> TokenWeight;
  typedef ArcTpl<TokenWeight> TokenArc;

  typedef SetTokenWeightMapper<Arc> map_AT;

  MapHypergraph<Arc, TokenArc, map_AT> mapped(hg, map_AT());

  boost::ptr_vector<TokenWeight> distances;
  ConvertCharsToTokensUtil::AssignTokenWeight<TokenWeight> stateWt(mapped);
  insideAlgorithm(mapped, &distances, stateWt, false);

  if (!pHgResult->getVocabulary()) {
    SDL_WARN(Hypergraph.ConvertCharsToToken, "using vocabulary from the input Hypergraph");
    pHgResult->setVocabulary(hg.getVocabulary());
  }
  using AssembleTokensUtil::ConstructResultArcForeachIncomingToken;
  typedef ConstructResultArcForeachIncomingToken<Arc, TokenWeight> ConstructResult;
  ConstructResult statesVisitor(hg, distances, pHgResult);
  TopsortStatesTraversal<Arc>(hg, &statesVisitor);
  statesVisitor.finishVisit();
  SDL_TRACE(Hypergraph.ConvertCharsToTokens, "Result:\n" << *pHgResult);
}

// Detokenize

namespace DetokenizeUtil {

/*
  Assigns token weights as inside weights of axioms; to be
  used in inside algorithm. Determines, for example, that a
  "(<glue>" is extendable to the right, but not to the left,
  "<glue>)" extends to the left, a regular lexical item
  (e.g. "the", "man", ...) *may* extend to the left or to the right.
*/
template <class TW>
class AssignTokenWeight : public StateToWeight<TW> {

 public:
  typedef TW TokenWeight;
  typedef typename TokenWeight::Weight Weight;
  typedef ArcTpl<TokenWeight> Arc;
  AssignTokenWeight(IHypergraph<Arc> const& hg, IVocabularyPtr const& pVoc,
                    std::string const& glueSymbol = Vocabulary::kGlueAffix)
      : hg_(hg)
      , invoc_(hg.getVocabulary().get())
      , outvoc_(pVoc)
      , glueSymbol_(glueSymbol)
      , glueSymbolLen_(glueSymbol.length()) {}

  // A sequence of symbol strings is, for example:
  // (<glue> foo bar <glue>)

  TokenWeight operator()(StateId stateId) const {
    if (hg_.hasLexicalLabel(stateId)) {
      Sym sym = hg_.inputLabel(stateId);
      // Note: A regular symbol doesn't *have* to extend, but it will
      // extend if there is a symbol on the left or on the right that
      // must extend (e.g., __LW_AT__])
      typename Token::Properties tokenProps = analyzeSymbol<Token>(&sym);
      return TokenWeight(Token(sym, tokenProps), Weight::one());
    }
    return TokenWeight(Token(stateId, kNoState, false), Weight::one());
  }

 private:
  /**
     Examples: "<glue>)", "<glue>-<glue>"
  */
  bool startsWithGlueSymbol(std::string const& str) const { return Util::startsWith(str, glueSymbol_); }

  /**
     Examples: "(<glue>", "<glue>-<glue>"
  */
  bool endsWithGlueSymbol(std::string const& str) const { return Util::endsWith(str, glueSymbol_); }

  /**
     \param[inout] sym

     Analyzes a symbol identified by the Sym; the
     symbol can be regular, left-extending (e.g., "<glue>."),
     right-extending (e.g., "(<glue>"), or both (e.g.,
     "<glue>-<glue>").
  */
  template <class Token>
  typename Token::Properties analyzeSymbol(Sym* modifysym) const {
    if (modifysym->isSpecialTerminal()) return Token::kBlockSymbolTokenProperties;

    typename Token::Properties r = Token::kExtendable;

    std::string const& sym = invoc_->str(*modifysym);
    bool const startglue = startsWithGlueSymbol(sym);
    bool const endglue = endsWithGlueSymbol(sym);

    if (!startglue && !endglue) {
      if (invoc_ != outvoc_.get()) *modifysym = outvoc_->addTerminal(sym);
      return r;
    }

    Pchar s = sym.data();
    Slice subsym(s, s + sym.size());

    if (startglue) {
      subsym.first += glueSymbolLen_;
      r |= Token::kMustExtendLeft;
    }

    if (endglue) {
      subsym.second -= glueSymbolLen_;
      r |= Token::kMustExtendRight;
    }

    *modifysym = outvoc_->addTerminal(subsym);

    return r;
  }

  IHypergraph<Arc> const& hg_;
  IVocabulary* invoc_;
  IVocabularyPtr outvoc_;  // output voc
  const std::string glueSymbol_;
  const std::size_t glueSymbolLen_;
};
}

/**
   Same implementation as convertCharsToTokens but different AssignTokenWeight policy
*/
template <class Arc>
void detokenize(IHypergraph<Arc> const& hgInput, IMutableHypergraph<Arc>* pHgResult) {
  if (!hgInput.storesInArcs())
    SDL_THROW_LOG(Hypergraph.ConvertCharsToTokens, ConfigException, "detokenize requires indexing in-arcs");
  SDL_DEBUG(Hypergraph.detokenize, "detokenize()");
  SDL_TRACE(Hypergraph.detokenize, hgInput);
  using namespace ConvertCharsToTokensUtil;

  typedef typename Arc::Weight Weight;
  typedef TokenWeightTpl<Weight> TokenWeight;
  typedef ArcTpl<TokenWeight> TokenArc;

  typedef SetTokenWeightMapper<Arc> map_AT;

  MapHypergraph<Arc, TokenArc, map_AT> mapped(hgInput, map_AT());

  if (!hgInput.getVocabulary()) {
    SDL_THROW_LOG(Hypergraph, ConfigException, "detokenize input hypergraph needs vocabulary");
  }

  if (!pHgResult->getVocabulary()) {
    pHgResult->setVocabulary(Vocabulary::createDefaultVocab());
  }

  boost::ptr_vector<TokenWeight> distances;
  DetokenizeUtil::AssignTokenWeight<TokenWeight> tokenWt(mapped, pHgResult->getVocabulary());
  insideAlgorithm(mapped, &distances, tokenWt, false);
  using AssembleTokensUtil::ConstructResultArcForeachIncomingToken;
  typedef ConstructResultArcForeachIncomingToken<Arc, TokenWeight> ConstructResult;
  ConstructResult statesVisitor(hgInput, distances, pHgResult);
  TopsortStatesTraversal<Arc>(hgInput, &statesVisitor);
  statesVisitor.finishVisit();
}


}}
