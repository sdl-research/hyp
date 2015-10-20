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

    open constraints for each word index

*/

#ifndef CONSTRAINTSFORWORD_JG_2014_06_17_HPP
#define CONSTRAINTSFORWORD_JG_2014_06_17_HPP
#pragma once

#include <sdl/Util/StringToTokens.hpp>
#include <sdl/Constraints/Constraints.hpp>
#include <sdl/HypergraphExt/Blocks.hpp>
#include <sdl/Util/AcceptStringImpls.hpp>
#include <sdl/Util/StringToTokens.hpp>
#include <sdl/Util/Add.hpp>


namespace sdl {

typedef Position WordPosition;
typedef std::vector<WordPosition> WordPositions;

struct RememberWordPositions {
  typedef Position WordIndex;
  typedef std::vector<WordIndex> WordIndexForPosition;
  WordIndexForPosition& wordIndex_;
  WordPositions& wordLength_; // should have correct utf8 lengths via StringToTokens utf8ToWordSpanImpl
  mutable Position nextWordIndex_;
  Position numWords() const { return nextWordIndex_; }

  RememberWordPositions(WordIndexForPosition& wordIndex, WordPositions& wordLength)
      : wordIndex_(wordIndex), wordLength_(wordLength), nextWordIndex_() {}
  bool spans() const { return true; }
  template <class Word>
  void operator()(Word const&, TokenSpan span) const {
    wordLength_.push_back(len(span)); // span should be in terms of unicode chars
    SDL_DEBUG(ConstraintForWord, "word # " << nextWordIndex_ << " len="<<wordLength_.back());
    Position const word = nextWordIndex_++;
    Util::increaseSize(wordIndex_, span.second, kNullPosition);
    for (Position i = span.first; i < span.second; ++i) wordIndex_[i] = word;
  }
  void finishPhrase(SdlFloat) const {}
};

/// alternative impl: O(#words) instead of O(#chars) by sorting word spans,
/// lower bound to begin.
struct WordsInSpan : WordPositions {
  Position numWords_;
  friend inline std::ostream& operator<<(std::ostream& out, WordsInSpan const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const;

  void init(std::string const& str, WordPositions& wordLength,
            Util::StringToTokens const& tokens = Util::StringToTokens())
      // should have correct utf8 lengths via StringToTokens utf8ToWordSpanImpl
  {
    reserve(str.size());
    RememberWordPositions r(*this, wordLength);
    tokens.acceptImpl(str, r);
    numWords_ = r.numWords();
  }

  template <class VisitWordArg, class VisitArg>
  void acceptWithArg(TokenSpan span, VisitWordArg& visit, VisitArg const& arg) const {
    assert(span.second <= size());
    if (span.first != span.second) {
      Position word = (*this)[span.first];
      if (word != kNullPosition) {
        visit(word, arg);
      }
      for (;;) {
        ++span.first;
        if (span.first == span.second) return;
        Position const newWord = (*this)[span.first];
        if (newWord != word) {
          if (newWord != kNullPosition) {
            visit(newWord, arg);
          }
          word = newWord;
        }
      }
    }
  }
};

/// index into constraints vector, rather than constraints[index].id
typedef Hypergraph::BlockIds ConstraintIndices;
typedef std::vector<ConstraintIndices> ConstraintsForWord;

typedef TokenSpan WordSpan;
typedef std::vector<WordSpan> WordSpanForConstraint;

struct ComputeConstraintsForWord {
  enum { kFirstWordOnly = true };
  ConstraintsForWord& constraintsForWord_;
  WordSpanForConstraint& wordSpanForConstraint_;
  WordsInSpan const& wordsInSpan_;

  void operator()(Position word, BlockId constraintIndex) {
    assert(word < constraintsForWord_.size());
    constraintsForWord_[word].push_back(constraintIndex);
    assert(constraintIndex < wordSpanForConstraint_.size());
    growSpan(wordSpanForConstraint_[constraintIndex], word);
    SDL_DEBUG(ComputeConstraintsForWord,
              "indexed word # " << word << " => open constraint @i=" << constraintIndex << " word-span => "
                                << PrintSpan(wordSpanForConstraint_[constraintIndex]));
  }
  ComputeConstraintsForWord(Constraints const& constraints, WordsInSpan const& wordsInSpan,
                            ConstraintsForWord& constraintsForWord,
                            WordSpanForConstraint& wordSpanForConstraint)
      : constraintsForWord_(constraintsForWord)
      , wordsInSpan_(wordsInSpan)
      , wordSpanForConstraint_(wordSpanForConstraint) {
    ConstraintIndex const n = constraints.size();
    assert(wordSpanForConstraint_.empty());
    wordSpanForConstraint_.resize(n, kMinTokenSpan);
    constraintsForWord_.resize(wordsInSpan.numWords_);
    SDL_DEBUG(ComputeConstraintsForWord, "computing for " << constraints.size()
                                                          << " constraints: " << constraints);
    for (ConstraintIndex i = 0; i < n; ++i) {
      SDL_DEBUG(ComputeConstraintsForWord, "char-span " << PrintSpan(constraints[i].span)
                                                        << " for constraint @i=" << i);
      wordsInSpan.acceptWithArg(constraints[i].span, *this, i);
    }
  }
};


}

#endif
