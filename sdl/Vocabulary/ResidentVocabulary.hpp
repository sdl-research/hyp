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

    'resident': in-memory (not starting from any predefined id<->string mapping)
*/

#ifndef SDL_RESIDENTVOCABULARY_H_
#define SDL_RESIDENTVOCABULARY_H_
#pragma once

#include <sdl/IVocabulary.hpp>
#include <sdl/Sym.hpp>
#include <sdl/Vocabulary/BasicVocabularyImpl.hpp>

#include <sdl/Util/LeakCheck.hpp>

namespace sdl {
namespace Vocabulary {

template <class ReadOnlyVocab>
struct ExtendedVocabulary;

struct ResidentVocabulary : IVocabulary {
  void addLeakChecks(Util::ILeakChecks& leaks) override;

  /**
     if you have ExtendedVocabulary and this is the second part, you should pass
     in the size of the first parts so we can put them all in the same
     (not-'persistent') space
  */
  explicit ResidentVocabulary(unsigned startingTerminal = 0, unsigned startingNonterminal = 0,
                              unsigned startingVariable = 0) {
    initStarts(startingTerminal, startingNonterminal, startingVariable);
  }

  void initStarts(unsigned startingTerminal = 0, unsigned startingNonterminal = 0,
                  unsigned startingVariable = 0);

  template <class ReadOnlyVocab>
  friend struct ExtendedVocabulary;
  friend class MultiVocabulary;

  void loadTerminals(std::string const& terminalPath) override;  //, std::string const& sourcePath);
  void loadNonterminals(std::string const& nonTerminalPath) override;  //, std::string const& sourcePath);

  void clearSinceFreeze() override {
    SDL_DEBUG(Leak.ResidentVocabulary.clearSinceFreeze, *this);
    vocabTerminal.clearSinceFreeze();
    vocabNonterminal.clearSinceFreeze();
    SDL_DEBUG(Leak.ResidentVocabulary.clearSinceFreeze, "after: " << *this);
  }

  WordCount countSinceFreeze() const override {
    return vocabNonterminal.countSinceFreeze() + vocabTerminal.countSinceFreeze();
  }

  void freeze() override {
    vocabTerminal.freeze();
    vocabNonterminal.freeze();
  }

  virtual void addSymbolCounts(SymbolType type, SymbolCounts& symbols) const override {
    // TODO: test
    symbols.thread += _GetNumSymbols(type);
    symbols.symbolType = type;
  }

  void enterThreadLocal() override {
    // TODO: test
    _enterThreadLocal();
  }

  virtual Sym addTerminal(std::string const& word) override;
  virtual Sym getTerminal(std::string const& word) const override;

 protected:
  Sym _addTerminal(std::string const& word) { return vocabTerminal.add(word); }

  Sym _getTerminal(std::string const& word) const { return vocabTerminal.sym(word); }

  /**
     before this, addSymbol go to per-process state and must be synchronized
     from the outside. after, they go to per-thread. the transition is one-way
  */
  void _enterThreadLocal() {}

  virtual Sym addImpl(std::string const& str, SymbolType symType) override { return _Add(str, symType); }

  virtual Sym doAddField(Slice const& word, SymbolType symType) override {
    return _Add(std::string(word.first, word.second), symType);
    // TODO: add Slice hash lookup to readonly/resident vocabs
  }

  virtual std::string const& strImpl(Sym sym) const override { return _Str(sym); }

  virtual Sym symImpl(std::string const& str, SymbolType symType) const override {
    return _Sym(str, symType);
  }

  virtual bool containsSymImpl(Sym sym) const override { return _containsSym(sym); }

  virtual bool containsImpl(std::string const& str, SymbolType symType) const override {
    return _contains(str, symType);
  }

  virtual unsigned doGetNumSymbols(SymbolType symType) const override { return _GetNumSymbols(symType); }

  virtual std::size_t doGetSize() const override { return _GetSize(); }

  virtual void accept(IVocabularyVisitor& visitor) override {
    // TODO: test
    return _Accept(visitor);
  }

  virtual void acceptType(IVocabularyVisitor& visitor, SymbolType symType) override {
    return _AcceptType(visitor, symType);
  }

  virtual Sym doAddSymbolMustBeNew(std::string const& str, SymbolType symType) override {
    // TODO: test
    return _AddSymbolMustBeNew(str, symType);
  }

  SymInt pastFrozenTerminalIndex() const override { return vocabTerminal.pastFrozenIndex(); }

  // faster calls for ExtendedVocabulary - no need for virtual dispatch
  Sym _Add(std::string const&, SymbolType symType);
  std::string const& _Str(Sym) const;
  Sym _Sym(std::string const&, SymbolType symType) const;
  bool _containsSym(Sym symbolId) const;
  bool _boundsSym(Sym symbolId) const;
  bool _contains(std::string const& symbol, SymbolType symType) const;
  void _Accept(IVocabularyVisitor& visitor);
  void _AcceptType(IVocabularyVisitor&, SymbolType);
  unsigned _GetNumSymbols(SymbolType) const;
  SymInt _pastFrozenTerminalIndex() const { return vocabTerminal.pastFrozenIndex(); }

  std::size_t _GetSize() const;
  Sym _AddSymbolMustBeNew(std::string const&, SymbolType);

  BasicVocabularyImpl& getVocab(SymbolType type) {
    return const_cast<BasicVocabularyImpl&>(const_cast<ResidentVocabulary const*>(this)->getVocab(type));
  }
  BasicVocabularyImpl const& getVocab(SymbolType type) const {
    switch (type) {
      case kTerminal:
        return vocabTerminal;
      case kNonterminal:
        return vocabNonterminal;
      case kVariable:
        return vocabVariable;
      default:
        SDL_THROW_LOG(ResidentVocabulary, InvalidSymType, "Invalid symbol type " << type << " requested.");
    }
  }

 protected:
  BasicVocabularyImpl vocabTerminal;
  BasicVocabularyImpl vocabNonterminal;
  BasicVocabularyImpl vocabVariable;
};


}}

#endif
