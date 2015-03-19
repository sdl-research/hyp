/** \file

    structures built by parser (then used to build states and arcs for a hypergraph).
*/

#ifndef HYP__HYPERGRAPH_PARSERUTIL_HPP
#define HYP__HYPERGRAPH_PARSERUTIL_HPP
#pragma once

#include <string>
#include <vector>
#include <sdl/Hypergraph/Types.hpp>
#include <boost/optional.hpp>

namespace sdl {
namespace Hypergraph {
namespace ParserUtil {

// State and Arc hold information about the actual states and arcs
// parsed. We are using these wrappers instead of the real Arcs
// because otherwise we'd have to template the parser on the arc.

//TODO: Field instead of string (0 copy) - but then need to translate each arc
//as you go, which means custom spirit actions instead of just relying on
//attributes+fusion
struct State {

  State()
    : id(kNoState)
    , isInputSymbolLexical(false)
    , isOutputSymbolLexical(false)
  {}

  State(StateId i)
    : id(i)
    , isInputSymbolLexical(false)
    , isOutputSymbolLexical(false)
  {}

  StateId id;

  /// if empty / unset then we assume an unlabeled state (use <eps> instead of empty string when you mean empty string)
  std::string inputSymbol;

  /// if empty / unset, output sym is same as input (use <eps> instead of empty string when you mean empty string)
  std::string outputSymbol;

  bool isInputSymbolLexical;
  bool isOutputSymbolLexical;

  static const StateId kStart = kNoState - 1;
  static const StateId kFinal = kNoState - 2;

  bool hasId() const {
    return id != kNoState && id != kStart && id != kFinal;
  }

  void increaseMaxId(StateId &maxId) const {
    if (!hasId())
      return;
    if (maxId < id)
      maxId = id;
  }

  template <class Out>
  void print(Out &out) const {
    char const* inQuote = isInputSymbolLexical ? "\"" : "";
    char const* outQuote = isOutputSymbolLexical ? "\"" : "";
    out << id<<'('<<inQuote << inputSymbol << inQuote
     <<' '<<outQuote << outputSymbol << outQuote<<')';
  }

  template <class C, class T>
  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T> &out, State const& self) {
    self.print(out); return out;
  }

  template <class C, class T>
  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T> &out, State const* selfp) {
    out << "State@0x" << (void*)selfp << ": "; if (selfp) selfp->print(out); return out;
  }
};

struct Arc {
  State head;
  Arc()
      : head(kNoState)
  {}
  std::vector<ParserUtil::State> tails;
  std::string weightStr;
};


}}}

#endif
