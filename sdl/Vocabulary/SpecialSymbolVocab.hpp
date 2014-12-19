/** \file

    handle kSpecialTerminal and kSpecialNonterminal Sym types - notice that
    this isn't an IVocabulary (so it's faster)
*/

#ifndef SPECIALSYMBOLVOCABULARYIMPL_H_
#define SPECIALSYMBOLVOCABULARYIMPL_H_
#pragma once

#include <sdl/Vocabulary/BasicVocabularyImpl.hpp>
#include <sdl/Util/Singleton.hpp>

namespace sdl {
namespace Vocabulary {

/**
   Vocabulary for special symbols (implemented as a singleton).
   This class closely follows the IVocabulary interface although it does not inherit from it.
*/
class SpecialSymbolVocab {
 public:
  explicit SpecialSymbolVocab() {
    init();
  }

  /**
     idempotent, but should be called before threads start. this happens in
     static initialization (in initializing the SpecialSymbolsList.ipp constant
     symbols)
  */
  void init();

  virtual ~SpecialSymbolVocab();

  inline Sym add(std::string const& symbol, SymbolType symType) {
    assert(Sym::isSpecialType(symType));
    return (symType == kSpecialTerminal ? vocab : ntVocab)->add(symbol, symType);
  }

  std::string const& str(Sym const& id) const;

  inline Sym sym(std::string const& symbol, SymbolType symType) const {
    assert(Sym::isSpecialType(symType));
    return (symType == kSpecialTerminal ? vocab : ntVocab)->sym(symbol);
  }

  bool containsSym(Sym const& id) const;

  inline bool contains(std::string const& symbol, SymbolType symType) const {
    assert(Sym::isSpecialType(symType));
    return (symType == kSpecialTerminal ? vocab : ntVocab)->contains(symbol);
  }

  inline void accept(IVocabularyVisitor& visitor) {
    vocab->accept(visitor);
    ntVocab->accept(visitor);
  }

  inline unsigned getNumSymbols(SymbolType type) const {
    return (type == kSpecialTerminal ? vocab : ntVocab)->getNumSymbols();
  }

  inline unsigned getNumLexicals() {
    return vocab->getNumSymbols();
  }

  inline std::size_t getSize() const {
    return vocab->getNumSymbols() + ntVocab->getNumSymbols();
  }

 private:
  typedef BasicVocabularyImpl *BasicVocabPtr;
  // not a scoped_ptr because we want static init to give us 0 for idempotent init()
  BasicVocabPtr vocab, ntVocab;
};

/// may be used after static initialization
// extern SpecialSymbolVocab specialSymbols;
inline SpecialSymbolVocab& specialSymbols() {
  static SpecialSymbolVocab voc; //TODO: not threadsafe in MSVC10
  return voc;
}

/// to be used during static initialization
SpecialSymbolVocab& specialSymbolsForceInit();


}}

#endif
