#ifndef EMPTYSTRINGEPSILON_LW2012420_HPP
#define EMPTYSTRINGEPSILON_LW2012420_HPP
#pragma once

#include <sdl/IVocabulary.hpp>
#include <sdl/Util/IsInAngleBrackets.hpp>

namespace sdl { namespace Hypergraph {


inline Sym lexicalSymbol(std::string const& sym, IVocabulary &voc) {
  Sym id = NoSymbol;
  if(Util::isInAngleBrackets(sym)) {
    id = Vocabulary::specialSymbols().sym(sym, kSpecialTerminal);
    if(!id)
      id = Vocabulary::specialSymbols().sym(sym, kSpecialNonterminal);
  }
  if(!id)
    id = voc.add(sym, kTerminal);
  return id;
}

inline LabelPair lexicalPair(std::string const& i, std::string const& o, IVocabulary &voc) {
  return LabelPair(lexicalSymbol(i, voc), lexicalSymbol(o, voc));
}

inline Sym lexicalOrEpsilon(std::string const& sym, IVocabulary &voc)
{
  if (sym.empty())
    return EPSILON::ID;
  return lexicalSymbol(sym, voc);
}


}}

#endif
