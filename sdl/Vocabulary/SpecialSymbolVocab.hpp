/** \file

    handle kSpecialTerminal Sym type - notice that this isn't an IVocabulary (so
    it's faster)
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

  Sym add(std::string const& symbol);
  Sym addAssertId(std::string const& symbol, SymIdInt id);

  std::string const& str(Sym id) const;

  inline Sym sym(std::string const& symbol) const {
    return vocab->sym(symbol);
  }

  bool containsSym(Sym id) const;

  inline bool contains(std::string const& symbol) const {
    // Currently all special symbols are terminals.
    return vocab->contains(symbol);
  }

  inline void accept(IVocabularyVisitor& visitor) {
    vocab->accept(visitor);
  }

  inline unsigned getNumSymbols() const {
    return vocab->getNumSymbols();
  }

  inline unsigned getNumLexicals() {
    return vocab->getNumSymbols();
  }

  inline std::size_t getSize() const {
    return vocab->getNumSymbols();
  }

  static SpecialSymbolVocab gInstance_;
 private:
  typedef BasicVocabularyImpl *BasicVocabPtr;
  // pimpl for safe static init (single threaded at this point) - note
  // specialSymbolsForceInit WILL be called in static init or else bad things.
  BasicVocabPtr vocab;
};

/// may be used after static initialization
// extern SpecialSymbolVocab specialSymbols;
inline SpecialSymbolVocab& specialSymbols() {
  return SpecialSymbolVocab::gInstance_;
}

/// to be used during static initialization
SpecialSymbolVocab& specialSymbolsForceInit();


}}

#endif
