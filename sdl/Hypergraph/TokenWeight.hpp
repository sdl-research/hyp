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

  typedef uint64 Properties;

  static const Properties kExtendableLeft = 0x0000000000000002ULL;
  static const Properties kExtendableRight = 0x0000000000000004ULL;
  static const Properties kMustExtendLeft = 0x0000000000000008ULL;
  static const Properties kMustExtendRight = 0x0000000000000010ULL;
  static const Properties kUnspecified = 0x0000000000000020ULL;

  static inline Properties getDefaultProperties() {
    return kExtendableLeft | kExtendableRight | kMustExtendLeft | kMustExtendRight;
  }

  Token() : props_(getDefaultProperties()), symIdsVec_(), start_(kNoState), endState_(kNoState) {}

  // default copy/equal/assign

  Token(Sym symId, Properties props = getDefaultProperties())
      : props_(props), symIdsVec_(), start_(kNoState), endState_(kNoState) {
    symIdsVec_.push_back(symId);
  }

  Token(Sym symId, Properties props, StateId startState, StateId endState)
      : props_(props), symIdsVec_(), start_(startState), endState_(endState) {
    if (symId != EPSILON::ID) symIdsVec_.push_back(symId);
  }

  /**
     create empty token.
  */
  static inline Token createEmptyToken(StateId startState, StateId endState) {
    return Token(startState, endState, false);
  }
  /**
     create empty token.
  */
  enum { EmptyToken = 0 };  // doc for unused bool arg tag

  Token(StateId startState, StateId endState, bool)
      : props_(kUnspecified), symIdsVec_(), start_(startState), endState_(endState) {}

  /**
     Is this token extendable to the left?
  */
  bool isExtendableLeft() const { return props_ & kExtendableLeft; }

  /**
     Is this token extendable to the right?
  */
  bool isExtendableRight() const { return props_ & kExtendableRight; }

  bool isExtendable() const { return isExtendableLeft() || isExtendableRight(); }

  bool mustExtendLeft() const { return props_ & kMustExtendLeft; }

  bool isComplete() const { return !mustExtendLeft() && !mustExtendRight(); }

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

  void setStart(StateId s) { start_ = s; }

  StateId getEndState() const { return endState_; }

  void setEndState(StateId s) { endState_ = s; }

  const_iterator begin() const { return symIdsVec_.begin(); }

  const_iterator end() const { return symIdsVec_.end(); }

  SymsIndex size() const { return (SymsIndex)symIdsVec_.size(); }

  bool empty() const { return symIdsVec_.empty(); }

  Sym const& front() const { return symIdsVec_.front(); }

  Sym const& back() const { return symIdsVec_.back(); }

  Sym const& operator[](SymsIndex n) const { return symIdsVec_[n]; }

  void append(Token const& other);

  Properties properties() const { return props_; }

  void print(std::ostream& out, IVocabularyPtr pVoc) const;

  bool operator==(Token const& other) const {
    return props_ == other.props_ && start_ == other.start_ && endState_ == other.endState_
           && symIdsVec_ == other.symIdsVec_;
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

    return symIdsVec_ < other.symIdsVec_;
  }

 private:
  SymsVector symIdsVec_;
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

  typedef typename TokenMap::value_type value_type;  // a pair
  typedef typename TokenMap::const_iterator const_iterator;
  typedef typename TokenMap::iterator iterator;

  TokenWeightTpl() : pTokens_(make_shared<TokenMap>()) {}

  TokenWeightTpl(StateId startState, StateId endState) : pTokens_(make_shared<TokenMap>()) {
    insert(Token(startState, endState, Token::EmptyToken), Weight::one());
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
  // SDL_CACHE_STATIC_LOCAL(Self, one(), Self())
  // this is a little slower, so probably worth the caching overhead
  SDL_CACHE_STATIC_LOCAL(Self, zero(), Self(Token(kNoState, kNoState, Token::EmptyToken), W::zero()))
#else
  static Self const& one() { return kOne; }
  static Self const& zero() { return kZero; }
#endif

  std::pair<iterator, bool> insert(Token const& tok, Weight const& weight) {
    return pTokens_->insert(value_type(tok, weight));
  }

  std::pair<iterator, bool> insert(std::pair<Token, Weight> const& aPair) { return pTokens_->insert(aPair); }

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
TokenWeightTpl<W> TokenWeightTpl<W>::kZero(Token(kNoState, kNoState, Token::EmptyToken), W::zero());

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
  if (isZero(tokWeight1) || isZero(tokWeight2)) return TokWt::zero();

  if (isOne(tokWeight1)) return tokWeight2;
  if (isOne(tokWeight2)) return tokWeight1;

  // Build cross-product of the contained tokens in tokWeight1 and tokWeight2
  TokWt product;  //=1
  forall (value_type tokWeightPair1, tokWeight1) {
    Token const& tok1 = tokWeightPair1.first;
    Weight const& w1 = tokWeightPair1.second;
    forall (value_type tokWeightPair2, tokWeight2) {
      Token const& tok2 = tokWeightPair2.first;
      Weight const& w2 = tokWeightPair2.second;
      if (tok1.empty() || tok2.empty()
          || tok2.isExtendableLeft() && (tok1.mustExtendRight() || tok2.mustExtendLeft())) {
        Token tok3(tok1);
        tok3.append(tok2);
        Weight w3 = Hypergraph::times(w1, w2);
        if (tok1.properties() == Token::kUnspecified)
          tok3.setProperties(tok2.properties());
        else if (tok2.properties() == Token::kUnspecified)
          tok3.setProperties(tok1.properties());
        product.insert(tok3, w3);
      } else if (tok2.isExtendable()) {
        Token tok(tok2);
        if (tok1.getEndState() != kNoState) tok.setStart(tok1.getEndState());
        product.insert(tok, w2);
      }
    }
  }

  if (product.empty()) product = TokWt::zero();

  SDL_TRACE(Hypergraph.TokenWeightTpl, "times(" << tokWeight1 << ", " << tokWeight2 << ") = " << product);
  return product;
}


}}

#endif
