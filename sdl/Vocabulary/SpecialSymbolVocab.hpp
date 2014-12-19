





#ifndef SPECIALSYMBOLVOCABULARYIMPL_H_
#define SPECIALSYMBOLVOCABULARYIMPL_H_























  void init();

  virtual ~SpecialSymbolVocab();


















  }

  inline void accept(IVocabularyVisitor& visitor) {


  }

  inline unsigned getNumSymbols(SymbolType type) const {


















// extern SpecialSymbolVocab specialSymbols;
inline SpecialSymbolVocab& specialSymbols() {
  static SpecialSymbolVocab voc; //TODO: not threadsafe in MSVC10
  return voc;
}








