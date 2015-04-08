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

    Defines Token and TokenWeight classes.

    TokenWeight is used by several functions that transform tokens in
    Hypergraphs.

    Example for how convertCharsToTokens uses TokenWeight:
    Transform "<TOK>" "a" "b" "c" "</TOK>" to "abc"

    Example for how detokenize uses TokenWeight:
    Transform "a" "b" "<GLUE>c" to "a" "bc"

    also permit <xmt-blockN> and </...> inside or outside <TOK>

    For simple usage examples, see test/TestTokenWeight.hpp.

    \author Markus Dreyer
*/

#ifndef HYP__HYPERGRAPH_TOKEN_WEIGHT_HPP
#define HYP__HYPERGRAPH_TOKEN_WEIGHT_HPP
#pragma once

#include <sdl/IntTypes.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/CacheStaticLocal.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <map>
#include <utility>
#include <vector>

#include <boost/make_shared.hpp>
#include <boost/cstdint.hpp>
#include <boost/ptr_container/ptr_set.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Compare.hpp>

#include <sdl/IVocabulary-fwd.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>

namespace sdl {
namespace Hypergraph {

/**
   A token with a start state and an end state.

   Defines Token properties like kMustExtendLeft, kMustExtendRight,
   etc. that are used as follows (see ConvertCharsToTokens.hpp):

   A token "x<GLUE>" must extend to the right. (Example: x<GLUE> y
   becomes xy)

   A token "<GLUE>y" must extend to the left. (Example: x <GLUE>y
   becomes xy)

   A token "<GLUE>y<GLUE>" must extend to both the left and the
   right. (Example: x <GLUE>y<GLUE> z becomes xyz)

   A token <TOK> must extend to the right.

   A token </TOK> must extend to the left.

   These properties have an effect on the times(TokenWeightTpl,
   TokenWeightTpl) operation , which combines two TokenWeights by
   concatenating the contained tokens.
*/
class Token {

 public:
  typedef Syms SymsVector;
  typedef SymsVector::const_iterator const_iterator;

  typedef unsigned Properties;

  // TODO: why not start at 1?
  enum {
    kUnspecified = 0,
    kExtendableLeft = 1,
    kExtendableRight = 2,
    kMustExtendLeft = 4,
    kMustExtendRight = 8,
    //    kBlockLeft = 32, // unused
    kBlockRight = 16,
    kBlockAny = kBlockRight, // | kBlockLeft // unused
    kExtendable = kExtendableLeft | kExtendableRight,
    // if we don't allow extending, then have to put block syms on separate arcs (not graph arcs, but fsm)
    kDefaultTokenProperties = kExtendableLeft | kExtendableRight | kMustExtendLeft | kMustExtendRight,
    kBlockSymbolTokenProperties = kBlockAny | kExtendable,
    kMustExtend = kMustExtendLeft | kMustExtendRight,
    kRightExtendability = kExtendableRight | kMustExtendRight | kBlockRight
  };

  bool blockRight() const { return props_ & kBlockRight; }

  Token() : props_(kDefaultTokenProperties), syms_(), start_(kNoState), endState_(kNoState) {}

  // default copy/equal/assign

  Token(Sym symId, Properties props = kDefaultTokenProperties)
      : props_(props), start_(kNoState), endState_(kNoState) {
    syms_.push_back(symId);
  }

  Token(Sym symId, Properties props, StateId startState, StateId endState)
      : props_(props), start_(startState), endState_(endState) {
    if (symId != EPSILON::ID) syms_.push_back(symId);
  }

  /**
     create empty token.
  */
  static inline Token createEmptyToken(StateId startState, StateId endState) {
    return Token(startState, endState, false);
  }

  Token(StateId startState, StateId endState, bool)
      : props_(kUnspecified), start_(startState), endState_(endState) {}

  /**
     Is this token extendable to the left?
  */
  bool isExtendableLeft() const { return props_ & kExtendableLeft; }

  /**
     Is this token extendable to the right?
  */
  bool isExtendableRight() const { return props_ & kExtendableRight; }

  bool isExtendable() const { return props_ & kExtendable; }

  bool mustExtendLeft() const { return props_ & kMustExtendLeft; }

  bool isComplete() const { return !(props_ & kMustExtend); }

  bool mustExtendRight() const { return props_ & kMustExtendRight; }

  void setExtendableLeft(bool isExtendableLeft) {
    if (isExtendableLeft)
      props_ |= kExtendableLeft;
    else
      props_ &= ~kExtendableLeft;
  }

  void setExtendableRight(bool isExtendableRight) {
    if (isExtendableRight)
      props_ |= kExtendableRight;
    else
      props_ &= ~kExtendableRight;
  }

  void setMustExtendLeft(bool mustExtendLeft) {
    if (mustExtendLeft)
      props_ |= kMustExtendLeft;
    else
      props_ &= ~kMustExtendLeft;
  }

  void setMustExtendRight(bool mustExtendRight) {
    if (mustExtendRight)
      props_ |= kMustExtendRight;
    else
      props_ &= ~kMustExtendRight;
  }

  void setProperties(Properties props) { props_ = props; }

  StateId start() const { return start_; }
  StateId endState() const { return endState_; }

  void setStart(StateId s) { start_ = s; }

  StateId getEndState() const { return endState_; }

  void setEndState(StateId s) { endState_ = s; }

  const_iterator begin() const { return syms_.begin(); }

  const_iterator end() const { return syms_.end(); }

  SymsIndex size() const { return (SymsIndex)syms_.size(); }

  bool empty() const { return syms_.empty(); }

  Sym const& front() const { return syms_.front(); }

  Sym const& back() const { return syms_.back(); }

  Sym const& operator[](SymsIndex n) const { return syms_[n]; }

  Properties properties() const { return props_; }

  void print(std::ostream& out, IVocabularyPtr const& pVoc) const { print(out, pVoc.get()); }
  void print(std::ostream& out, IVocabulary const* voc) const;

  bool operator==(Token const& other) const {
    return props_ == other.props_ && start_ == other.start_ && endState_ == other.endState_
           && syms_ == other.syms_;
  }

  bool operator!=(Token const& other) const { return !(*this == other); }

  /// \return some total order (ultimately is topological in tokens)
  bool operator<(Token const& other) const {
    if (props_ < other.props_) return true;
    if (other.props_ < props_) return false;

    if (start() < other.start()) return true;
    if (other.start() < start()) return false;

    if (getEndState() < other.getEndState()) return true;
    if (other.getEndState() < getEndState()) return false;

    return syms_ < other.syms_;
  }
  Syms const& syms() const { return syms_; }

  SymsVector syms_;
  Properties props_;
  StateId start_;
  StateId endState_;
};  // end class Token

inline std::ostream& operator<<(std::ostream& out, Token const& tok) {
  tok.print(out, IVocabularyPtr());
  return out;
}

/**
   The TokenWeight concats single characters into tokens (words).

   Note: The times operation is not commutative.
*/
template <class W>
class TokenWeightTpl {

  typedef TokenWeightTpl<W> Self;

 public:
  void allTimesBy(W const& delta) {
    for (typename TokenMap::iterator i = pTokens_->begin(), e = pTokens_->end(); i != e; ++i)
      timesBy(delta, i->second);
  }

  typedef W Weight;
  typedef std::map<Token, Weight> TokenMap;  // TODO: speed-up using (Token*)?
  typedef shared_ptr<TokenMap> TokenMapPtr;

  typedef std::pair<Token, Weight> TokenAndWeight;
  typedef typename TokenMap::value_type value_type;  // a pair
  typedef typename TokenMap::const_iterator const_iterator;
  typedef typename TokenMap::iterator iterator;

  TokenWeightTpl() : pTokens_(make_shared<TokenMap>()) {}

  // TODO: do we use endState for anything important?
  TokenWeightTpl(StateId startState, StateId endState) : pTokens_(make_shared<TokenMap>()) {
    insert(Token(startState, endState, false), Weight::one());
  }

  TokenWeightTpl(Token const& token, Weight const& weight) : pTokens_(make_shared<TokenMap>()) {
    insert(token, weight);
  }

  TokenWeightTpl(Self const& other)
      : pTokens_(new TokenMap(other.begin(), other.end())), pVoc_(other.pVoc_) {}

  typedef void HasIsZero;
  bool isZero() const { return pTokens_->size() == 1 && Hypergraph::isZero(pTokens_->begin()->second); }
  typedef void HasIsOne;
  bool isOne() const { return pTokens_->empty(); }

  static Self kOne;
  static Self kZero;

#if SDL_WEIGHT_USE_AT_STATIC_INIT
  // reasonably cheap construction. spare us the thread synch difficulty
  static inline Self one() { return Self(); }
  // this is a little slower, so probably worth the caching overhead
  SDL_CACHE_STATIC_LOCAL(Self, zero(), Self(Token(kNoState, kNoState, false), W::zero()))
#else
  static Self const& one() { return kOne; }
  static Self const& zero() { return kZero; }
#endif

  std::pair<iterator, bool> insert(Token const& tok, Weight const& weight) {
    return pTokens_->insert(value_type(tok, weight));
  }

  void insertProd(Token const& tok, Weight const& weight1, Weight const& weight2) {
    timesBy(weight2, (*pTokens_)[tok] = weight1);
  }

  std::pair<iterator, bool> insert(TokenAndWeight const& aPair) {
    return pTokens_->insert(reinterpret_cast<value_type const&>(aPair));
  }
  std::pair<iterator, bool> insert(value_type const& aPair) { return pTokens_->insert(aPair); }

  const_iterator begin() const { return pTokens_->begin(); }

  const_iterator end() const { return pTokens_->end(); }

  iterator begin() { return pTokens_->begin(); }

  iterator end() { return pTokens_->end(); }

  /**
     Returns number of tokens
  */
  std::size_t size() const { return pTokens_->size(); }

  bool empty() const { return pTokens_->empty(); }

  /**
     Only for better printing (optional).
  */
  void setVocabulary(IVocabularyPtr const& pVoc) { pVoc_ = pVoc; }

  IVocabularyPtr getVocabulary() const { return pVoc_; }

  bool operator<(Self const& other) const { return *pTokens_ < *(other.pTokens_); }

  bool operator==(Self const& other) const { return *pTokens_ == *(other.pTokens_); }

  bool operator!=(Self const& other) const { return !(*this == other); }

  // this has no meaning, but makes the getCost arc method compile. this means you can't get a best path for
  // TokenWeight (but you can with CompositeWeight<W1, TokenWeight<W2> >
  typedef float FloatT;
  FloatT getValue() const {
    SDL_THROW_LOG(Hypergraph.TokenWeight, UnimplementedException,
                  "getValue not allowed - we have a set of tokens and their values");
    return 0;
  }

 private:
  TokenMapPtr pTokens_;
  IVocabularyPtr pVoc_;

};  // end TokenWeightTpl

template <class W>
TokenWeightTpl<W> TokenWeightTpl<W>::kOne;

template <class W>
TokenWeightTpl<W> TokenWeightTpl<W>::kZero(Token(kNoState, kNoState, false), W::zero());

template <class W>
inline void parseWeightString(std::string const& str, TokenWeightTpl<W>*) {
  SDL_THROW_LOG(Hypergraph.TokenWeight, std::runtime_error, "Not implemented");
}

template <class W>
inline std::ostream& operator<<(std::ostream& out, TokenWeightTpl<W> const& tokWeight) {
  if (isZero(tokWeight)) return out << "Zero";
  Util::Sep sep(", ");
  out << "(";
  typedef typename TokenWeightTpl<W>::value_type value_type;
  forall (value_type tokWeightPair, tokWeight) {
    out << sep << "[";
    tokWeightPair.first.print(out, tokWeight.getVocabulary());
    out << ",w:" << tokWeightPair.second << "]";
  }
  out << ")";
  return out;
}

// TODO: more efficient plusBy?
template <class W>
inline TokenWeightTpl<W> plus(TokenWeightTpl<W> const& w1, TokenWeightTpl<W> const& w2) {
  typedef TokenWeightTpl<W> TokWt;
  if (isZero(w1)) return w2;
  if (isZero(w2)) return w1;
  TokWt sum(w1);
  typedef typename TokWt::value_type value_type;
  forall (value_type const& tokWeightPair, w2) {
    typedef typename TokWt::TokenMap TokenMap;
    std::pair<typename TokenMap::iterator, bool> result = sum.insert(tokWeightPair);
    if (!result.second) Hypergraph::plusBy(tokWeightPair.second, result.first->second);
  }
  SDL_TRACE(Hypergraph.TokenWeight, "plus(" << w1 << "," << w2 << ") = " << sum);
  return sum;
}

/**
   Combines (concats) all tokens from tokWeight1 and tokWeight2.

   noncommutative.
*/
template <class W>
inline TokenWeightTpl<W> times(TokenWeightTpl<W> const& tokWeight1, TokenWeightTpl<W> const& tokWeight2) {
  typedef W Weight;
  typedef TokenWeightTpl<W> TokWt;
  typedef typename TokWt::value_type value_type;
  typedef std::pair<Token, Weight> TokenAndWeight;
  if (isZero(tokWeight1) || isZero(tokWeight2)) return TokWt::zero();

  if (isOne(tokWeight1)) return tokWeight2;
  if (isOne(tokWeight2)) return tokWeight1;

  // Build cross-product of the contained tokens in tokWeight1 and tokWeight2
  TokWt product;  //=1
  forall (value_type const& tw1, tokWeight1) {
    Token const& tok1 = tw1.first;
    for (typename TokWt::TokenMap::const_iterator i = tokWeight2.begin(), e = tokWeight2.end(); i != e; ++i) {
      value_type const& tw2 = *i;
      Token const& tok2 = tw2.first;
      if (tok1.empty() || tok2.empty()
          || tok1.blockRight() || tok2.isExtendableLeft() && (tok1.mustExtendRight() || tok2.mustExtendLeft())) {
        // concat tok1 tok2
        TokenAndWeight tw3(tw1);
        Hypergraph::timesBy(tw2.second, tw3.second);
        tw3.first.syms_.append(tok2.syms_);
        Token::Properties &props3 = tw3.first.props_, props2 = tok2.props_;
        if (props3 == Token::kUnspecified)
          props3 = props2;
        else if (props2 != Token::kUnspecified)
          props3 = (props3 & ~Token::kRightExtendability) | (props2 & Token::kRightExtendability);
        StateId end2 = tok2.endState_;
        if (end2 != kNoState)
          tw3.first.endState_ = end2;
        product.insert(tw3);
      } else if (tok2.isExtendable()) { // starting new token w/ tok2.
        StateId const end1 = tok1.getEndState();
        if (end1 == kNoState)
          product.insert(tw2);
        else {
          TokenAndWeight tw3(tw2);
          tw3.first.setStart(end1);
          product.insert(tw3);
        }
      }
    }
  }

  if (product.empty()) product = TokWt::zero();

  SDL_TRACE(Hypergraph.TokenWeightTpl, "times(" << tokWeight1 << ", " << tokWeight2 << ") = " << product);
  return product;
}


}}

#endif
