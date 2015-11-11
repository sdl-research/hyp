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

    vocabulary resource: (string, type id) <-> Sym which is (index, type id)

    in xmt, novel input symbols are added only after an initialization phase
*/

#ifndef SDL_IVOCABULARY_HPP
#define SDL_IVOCABULARY_HPP
#pragma once

#include <sdl/IVocabulary-fwd.hpp>
#include <string>
#include <ostream>
#include <stdexcept>
#include <sdl/SharedPtr.hpp>
#include <sdl/Sym.hpp>  // std::string, Sym
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Resource.hpp>
#include <sdl/Types.hpp>
#include <sdl/Util/ThreadSpecific.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/IsDebugBuild.hpp>
#include <sdl/gsl.hpp>


namespace sdl {

/// a symbol string should not *usually* not contain space tab or newline (those
/// are syntactic chars that give us segments and tokens). whitespace is
/// considered to split tokens. most code assumes nonempty non-whitespace
/// symbols (the EPSILON::ID special symbol is more frequently appropriate
/// (though it means 0 tokens rather than 1 empty-string token)
inline bool validToken(std::string const& str) {
  return str.find_first_of(" \t\n") == std::string::npos;
}

std::string const kUnnamedVocabulary(("unnamed-vocabulary"));

/**
   processes (id, string) pairs of a Vocabulary.

   \see IVocabulary::accept
*/
struct IVocabularyVisitor {
  virtual ~IVocabularyVisitor() {}
  virtual void operator()(Sym id, std::string const& symbol) = 0;
};

struct IVocabulary : Resource {
  typedef std::size_t WordCount;
  WordCount const nSpecials_;

 protected:
  bool threadlocal_;

 public:
  static char const* getTypeName() { return "vocabulary"; }
  /**
     Purge (some) words from the vocabulary, so that they may get different
     ids next time they're added. Phrase grammar knows about some words;
     others get assigned ids as needed for an input sentence. But later, we
     want to purge those unknown words. This implies that people might want to
     be notified when a vocab is reset. Alternatively, we provide
     addSymbol that are immune to this purging. */
  bool evictThread(Occupancy const&) override;

  IVocabulary();

  virtual ~IVocabulary();

  /**
     add symbol if it doesn't exist. return id of added or existing symbol of type SymbolType
  */
  Sym add(std::string const& word, SymbolType symType) { return addImpl(word, symType); }
  Sym add(cstring_span<> word, SymbolType symType) { return addImpl(word, symType); }
  Sym add(char const* word, SymbolType symType) { return addImpl(std::string(word), symType); }
  Sym add(Slice field, SymbolType symType) {
    return addImpl(cstring_span<>(field.first, field.second), symType);
  }
  Sym addTerminal(Slice field) { return addTerminal(cstring_span<>(field.first, field.second)); }

  Sym add(Unicode c, SymbolType symType) { return addImpl(Util::utf8s(c), symType); }

  Sym sym(Sym symOther, IVocabulary const& otherVocab) const {
    assert(symOther);
    return symOther.isSpecial() ? symOther : this == &otherVocab
                                                 ? symOther
                                                 : symImpl(otherVocab.str(symOther), symOther.type());
  }

  Sym symFromOther(Sym symOther, IVocabulary const& otherVocab) const {
    assert(symOther);
    return symOther.isSpecial() ? symOther : symImpl(otherVocab.str(symOther), symOther.type());
  }

  /// return same string in our vocabulary. pre: symOther is not NoSymbol
  Sym add(Sym symOther, IVocabulary const& otherVocab) {
    assert(symOther);
    return symOther.isSpecial() ? symOther : this == &otherVocab
                                                 ? symOther
                                                 : addImpl(otherVocab.str(symOther), symOther.type());
  }

  Sym addFromOther(Sym symOther, IVocabulary const& otherVocab) {
    assert(symOther);
    return symOther.isSpecial() ? symOther : addImpl(otherVocab.str(symOther), symOther.type());
  }

  /// does symOther = add(symOther, otherVocab). pre: symOther is not
  /// NoSymbol
  void translateSymbol(Sym& symOther, IVocabulary const& otherVocab) {
    assert(symOther);
    if (!symOther.isSpecial()) symOther = addImpl(otherVocab.str(symOther), symOther.type());
  }

  /**
     change from per-process addSymbol to per-thread if \param threadlocal. ConfigException thrown if
     setThreadLocal(false) after setThreadLocal(true) and adding any symbols, since we don't use a separate
     symbol type but rather a symbol
  */
  virtual void setThreadLocal(bool threadlocal = true) {
    if (!threadlocal && threadlocal_)
      SDL_THROW_LOG(Vocabulary, ConfigException, "can't add process-wide symbols after thread-local");
    threadlocal_ = threadlocal;
    if (threadlocal) enterThreadLocal();
  }

  struct SymbolCounts {
    WordCount disk, process, thread, threadUnknown;
    SymbolType symbolType;
    SymbolCounts() : disk(), process(), thread(), threadUnknown(), symbolType(kAllSymbols) {}
    WordCount totalKnown() const { return disk + process + thread; }
    WordCount totalUnknown() const { return threadUnknown; }
    WordCount total() const { return totalKnown() + totalUnknown(); }
    friend inline std::ostream& operator<<(std::ostream& out, SymbolCounts const& self) {
      self.print(out);
      return out;
    }
    void print(std::ostream& out) const {
      out << '[';
      out << Sym::getTypeName(symbolType) << " count: ";
      out << total() << " total symbols, " << disk << " on disk, " << process << " process-wide in memory, "
          << thread << " thread-specific permanent, " << totalUnknown() << " thread-specific unknowns";
      out << ']';
    }
  };

  virtual void addSymbolCounts(SymbolType type, SymbolCounts& symbols) const {
    symbols.process += size(type);
    symbols.symbolType = type;
  }

  virtual void addSymbolCounts(SymbolCounts& symbols) const {
    addSymbolCounts(kTerminal, symbols);
    addSymbolCounts(kNonterminal, symbols);
    symbols.symbolType = kAllSymbols;
  }

  /**
     make permanent all symbols that were addSymbol at this time (grammar db
     symbols are always permanent)
  */
  virtual void freeze() = 0;

  /// all frozen terminal syms have index lower than this
  virtual SymInt pastFrozenTerminalIndex() const { return size(kTerminal); }

  /**
     \return number of symbols added since freeze.
  */
  virtual WordCount countSinceFreeze() const { return 0; }

  /**
     remove all addSymbol since last freeze (if no freeze, then all of them
     except whatever was permanent on vocab creation e.g. from grammar db).
  */
  virtual void clearSinceFreeze() = 0;

  // TODO: rename. getSymbolId should be called getSymbol and this should be
  // called getString. then, getSymbol/addSymbol could be getSym / addSym and
  // Sym could be Sym

  std::string const& str(Sym sym) const { return sym.isSpecial() ? strSpecial(sym) : strImpl(sym); }

  static std::string const& strSpecial(Sym special);
  static bool containsSymSpecial(Sym special);
  static bool containsSpecial(std::string const& word);
  static bool containsSpecial(cstring_span<> word);
  static Sym symSpecial(std::string const& word);
  static Sym symSpecial(cstring_span<> word);

  /// add(word, kTerminal) but should be faster
  virtual Sym addTerminal(std::string const& word) = 0;
  virtual Sym addTerminal(cstring_span<> word) = 0;

  /// sym(word, kTerminal), but should be faster
  virtual Sym terminal(std::string const& word) const = 0;
  virtual Sym terminal(cstring_span<> word) const = 0;
  Sym terminal(Slice field) const { return terminal(cstring_span<>(field.first, field.second)); }

  Sym sym(char const* word, SymbolType symType) const { return sym(std::string(word), symType); }

  Sym sym(std::string const& word, SymbolType symType) const {
    return symType == kSpecialTerminal ? symSpecial(word) : symImpl(word, symType);
  }

  Sym sym(cstring_span<> word, SymbolType symType) const {
    return symType == kSpecialTerminal ? symSpecial(word) : symImpl(word, symType);
  }

  Sym sym(Slice field, SymbolType symType) const {
    return symImpl(cstring_span<>(field.first, field.second), symType);
  }

  virtual Sym symImpl(cstring_span<> word, SymbolType symType) const { return sym(word, symType); }

  SymInt size(SymbolType symType) const {
    return symType == kSpecialTerminal ? nSpecials_ : sizeImpl(symType);
  }

  WordCount size() const { return nSpecials_ + sizeImpl(); }

  /// \return number of symbols *possibly* unk that would be affected by reset - likely same as
  /// countSinceFreeze
  virtual WordCount residentSize() const { return countSinceFreeze(); }

  /// \return number of fixed Sym<->string mappings
  virtual WordCount readOnlySize() const { return 0; }

  bool containsSym(Sym sym) const {
    return sym.isSpecial() ? containsSymSpecial(sym) : containsSymImpl(sym);
    ;
  }

  bool contains(std::string const& word, SymbolType symType) const {
    return symType == kSpecialTerminal ? containsSpecial(word) : containsImpl(word, symType);
  }

  bool contains(cstring_span<> word, SymbolType symType) const {
    return symType == kSpecialTerminal ? containsSpecial(word) : containsImpl(word, symType);
  }

  virtual void accept(IVocabularyVisitor& visitor) {
    acceptType(visitor, kTerminal);
    acceptType(visitor, kNonterminal);
    acceptType(visitor, kVariable);  // TODO: these should be constant x0...x<max>
  }

  /// type should be one of kTerminal kNonterminal kVariable (even though we
  /// also have kSpecialTerminal; those are statically defined in
  /// SpecialSymbolVocab
  virtual void acceptType(IVocabularyVisitor& visitor, SymbolType type) = 0;

  std::string const& getName() const { return this->name_.empty() ? kUnnamedVocabulary : this->name_; }

  /// used by RuleSerializer. TODO: cover in regr tests
  virtual void loadTerminals(std::string const& terminalPath) {
    SDL_THROW_LOG(IVocabulary, UnimplementedException, "Not implemented: loadTerminals");
  }
  virtual void loadNonterminals(std::string const& nonTerminalPath) {
    SDL_THROW_LOG(IVocabulary, UnimplementedException, "Not implemented: loadTerminals");
  }

  /// return address of string for sym if vocab contains sym. else return null.
  virtual std::string const* strOrNull(Sym sym) const {
    return containsSymImpl(sym) ? &strImpl(sym) : (std::string const*)0;
  }

  friend inline std::ostream& operator<<(std::ostream& out, IVocabulary const& self) {
    self.print(out);
    return out;
  }

  void print(std::ostream& out) const;

  char const* category() const override { return "vocabulary"; }
  std::string name() const override { return getName(); }

  void initProcessPhase(InitProcessPhase phase) override {
    if (phase == kPhase1) setThreadLocal(true);
  }

  void initThread() override { SDL_DEBUG(evict.init.Vocabulary, "vocabulary initThread " << getName()); }

 protected:
  virtual void enterThreadLocal() {}

  virtual Sym addImpl(std::string const&, SymbolType) {
    SDL_THROW_LOG(IVocabulary, UnimplementedException, "Not implemented: addImpl");
  }

  virtual Sym addImpl(cstring_span<>, SymbolType) {
    SDL_THROW_LOG(IVocabulary, UnimplementedException, "Not implemented: addImpl");
  }

  /**
     requires contains(Sym).
  */
  virtual std::string const& strImpl(Sym) const = 0;

  /**
     returns NoSymbol if !contains(symbol of type symType), else the Sym such that str(Sym) ==
     symbol
  */
  virtual Sym symImpl(std::string const& symbol, SymbolType symType) const = 0;

  virtual unsigned sizeImpl(SymbolType) const = 0;

  virtual WordCount sizeImpl() const = 0;

  virtual bool containsSymImpl(Sym symbolId) const = 0;

  // TODO@SK: better names: if containsSymbol then idForSymbol and if containsId then symbolForId
  virtual bool containsImpl(std::string const& symbol, SymbolType symType) const = 0;
  virtual bool containsImpl(cstring_span<> str, SymbolType symType) const = 0;
};

namespace {
std::string const kNoVocabularyName("No-Vocabulary");
}

inline std::string vocabName(IVocabulary const* voc) {
  return voc ? voc->getName() : kNoVocabularyName;
}

struct IPerThreadVocabulary {
  IVocabularyPtr processVocab;
  IPerThreadVocabulary() {}
  IPerThreadVocabulary(IVocabularyPtr const& processVocab) : processVocab(processVocab) {}
  virtual ~IPerThreadVocabulary() {}
  virtual void operator()(IVocabularyPtr&) = 0;
  IVocabularyPtr getPerThreadVocabulary() {
    IVocabularyPtr r;
    operator()(r);
    return r;
  }
};

typedef Util::ThreadSpecific<IVocabularyPtr> IVocabularyPtrPerThread;

struct PerProcessVocabulary : IPerThreadVocabulary {
  IVocabularyPtr processVocab;
  PerProcessVocabulary() {}
  PerProcessVocabulary(IVocabularyPtr const& processVocab) : processVocab(processVocab) {}
  virtual void operator()(IVocabularyPtr& p) { p = processVocab; }
};

inline std::ostream& operator<<(std::ostream& out, IVocabularyPtr const& p) {
  if (p)
    out << *p;
  else
    out << "[no vocab]";
  return out;
}

inline void printSym(std::ostream& out, Sym sym, IVocabulary const* voc) {
  if (voc && voc->containsSym(sym))
    out << voc->str(sym);
  else
    out << sym;
}

struct PrintSym {
  Sym sym;
  IVocabulary const* voc;
  PrintSym(Sym sym, IVocabulary const* voc) : sym(sym), voc(voc) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintSym const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { printSym(out, sym, voc); }
};


}

#endif
