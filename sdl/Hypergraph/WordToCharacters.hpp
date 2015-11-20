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

    split hypergraph labels so they're at most 1 unicode codepoint long, optionally inserting token-begin
   symbols
*/

#ifndef HYP__HG_WORDTOCHARACTER
#define HYP__HG_WORDTOCHARACTER
#pragma once

#include <vector>
#include <sdl/Syms.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Util/Once.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/IntSet.hpp>
#include <sdl/SharedPtr.hpp>
#include <boost/cstdint.hpp>
#include <sdl/Hypergraph/DeferAddArcs.hpp>

namespace sdl {
namespace Hypergraph {

/**
   in-place add new states/arcs so every terminal token is at most one unicode
   codepoint (leaving special syms and nonterms alone). no 'space' or epsilon
   separators added.

   expands input label and so discards output labels of modified arcs

   TODO: works for graph (one lexical label per tail) only

   side effect: hg->storesFirstTailOutArcs(), !hg->storesInArcs()
*/
template <class Arc>
struct WordToCharacters {
  IMutableHypergraph<Arc>* hg_;
  DeferAddArcs<Arc> addArcLater_;
  IVocabulary* vocab_;
  Sym tokenPrefixSym_ = NoSymbol;
  // NoSymbol if you don't want one inserted. else you can use original weight
  // (FeatureId) as a marker. new arcs get no added cost/features.
  Util::FixUnicode fixUnicode_;
  StateId nAddedArcsMinimum_ = 0;
  std::vector<StateIdContainer> insertStatesForTail_;

  /// compute (>1 char lexical state)->(char states) mapping
  void splitInputLabelChars() {
    bool insertingPrefix = (bool)tokenPrefixSym_;
    HypergraphBase::Labels labels = hg_->copyOfLabels();
    // avoid canonical-lex problem by copying (can't store ref if non-canonical, either)
    StateId s = 0, N = labels.size();
    SDL_TRACE(WordToCharacters, "input labels: " << printer(labels, vocab_));
    insertStatesForTail_.resize(N);
    for (; s < N; ++s) {
      Sym wordsym = labels[s];
      if (!wordsym.isLexical()) continue;
      std::string const& word = vocab_->str(wordsym);
      StateId nchars = word.size();
      if (!nchars || nchars == 1 && !insertingPrefix) continue;
      typedef std::vector<std::string> Chars;
      Chars chars;
      chars.reserve(nchars);  // upper bound
      Util::toUtf8Chs(word, chars, fixUnicode_);  // assume utf8 is well-formed
      SDL_TRACE(WordToCharacters, "unicode chars={ " << printer(chars) << " }");
      StateId const nunicode = chars.size();
      if (!nunicode) continue;
      StateId const ninsert = nunicode - !insertingPrefix;
      nAddedArcsMinimum_ += ninsert;
      StateIdContainer& insertStates = insertStatesForTail_[s];
      insertStates.resize(ninsert);
      Chars::const_iterator i = chars.begin(), end = chars.end();
      StateIdContainer::iterator to = insertStates.begin();
      if (insertingPrefix) {
        hg_->setInputLabel(s, tokenPrefixSym_);
      } else {
        hg_->setInputLabel(s, vocab_->addTerminal(*i));
        ++i;
      }
      SDL_DEBUG(WordToCharacters, "for input state "
                                      << s << " with input label: " << printer(wordsym, vocab_)
                                      << ": replaced => label: " << printer(hg_->inputLabel(s), vocab_));
      for (; i != end; ++i) *to++ = hg_->addState(vocab_->addTerminal(*i));
      assert(to == insertStates.begin() + ninsert);
      SDL_DEBUG(WordToCharacters, "for input state " << s << " with input label: " << printer(wordsym, vocab_)
                                                     << ": output: " << printer(insertStates, hg_));
    }
  }

  void operator()(ArcBase* a) const {
    Arc* arc = (Arc*)a;
    StateIdContainer& tails = arc->tails_;
    for (StateIdContainer::iterator i = tails.begin(), iend = tails.end(); i != iend; ++i) {
      StateIdContainer const& replacelex = insertStatesForTail_[*i];
      StateIdContainer::const_iterator j = replacelex.begin(), jend = replacelex.end();
      if (j != jend) {
        StateId lastHead = arc->head_;
        StateId tail = hg_->addState();
        arc->head_ = tail;
        for (;;) {
          assert(j < jend);
          StateId symstate = *j;
          if (++j == jend) {
            addArcLater_(new Arc(HeadAndTail(), lastHead, tail, symstate));
            return;  // only one lexical sym expanded per arc (recall input is graph + one lexical per tail)
          } else {
            StateId const head = hg_->addState();
            addArcLater_(new Arc(HeadAndTail(), head, tail, symstate));
            tail = head;
          }
        }
      }
    }
  }

  WordToCharacters(IMutableHypergraph<Arc>* h, Sym tokenPrefixSym = NoSymbol)
      : hg_(h), addArcLater_(*hg_), vocab_(hg_->vocab()), fixUnicode_(false), tokenPrefixSym_(tokenPrefixSym) {
    if (!hg_->isGraph())
      SDL_THROW_LOG(Hypergraph.WordToCharacters, ConfigException,
                    "wordToCharacters() doesn't support cfg (only fsm):\n " << hg_);
    if (hg_->hasOutputLabels())
      SDL_INFO(Hypergraph.WordToCharacters,
               "wordToCharacters will modify input labels only but input hg_ has some output labels"
               " - ignoring output labels");
    splitInputLabelChars();
    if (nAddedArcsMinimum_) {
      hg_->reserveAdditionalStates(nAddedArcsMinimum_);
      // if all labeled states were used, we add at least this many tails (could
      // be more if multiple arcs per state)
      hg_->clearCanonicalLexCache();  // we will be modifying some labels
      hg_->forceFirstTailOutArcsOnly();
      for (StateId s = 0, N = hg_->sizeForHeads(); s < N; ++s) hg_->visitOutArcs(s, *this);
      addArcLater_.finish();
    }
  }
};

/// side effect: hg->storesFirstTailOutArcs(), !hg->storesInArcs()
template <class Arc>
void wordToCharacters(IMutableHypergraph<Arc>* hg, Sym tokenPrefixSym = NoSymbol) {
  WordToCharacters<Arc>(hg, tokenPrefixSym);
}

/// side effect: hg->storesFirstTailOutArcs(), !hg->storesInArcs()
template <class Arc>
void wordToCharacters(IMutableHypergraph<Arc>& hg, Sym tokenPrefixSym = NoSymbol) {
  WordToCharacters<Arc>(&hg, tokenPrefixSym);
}

struct WordToCharactersOptions {
  bool tokSep = false;
  std::string terminalSep;
  template <class Config>
  void configure(Config& config) {
    config.is("WordToCharacters");
    config("tok-sep", &tokSep).init(false)("add <tok> special at start of every word (of 1 char or more)");
    config("string-sep", &tokSep)
        .init(" ")("if nonempty, add (instead of tok-sep) this terminal at start of word");
  }
  template <class Arc>
  void inplace(IMutableHypergraph<Arc>& hg) const {
    Sym sep = terminalSep.empty() ? (tokSep ? TOK_START::ID : NoSymbol) : hg.vocab()->addTerminal(terminalSep);
    wordToCharacters(&hg, sep);
  }
};


}}

#endif
