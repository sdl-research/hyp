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

    parse text format hypergraph arcs.
*/

#ifndef HYP__HYPERGRAPH_ARCPARSERFCT_HPP
#define HYP__HYPERGRAPH_ARCPARSERFCT_HPP
#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cassert>

#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Forall.hpp>

#include <sdl/IVocabulary.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/LabelPair.hpp>
#include <sdl/Hypergraph/SymbolPrint.hpp>
#include <sdl/Hypergraph/Exception.hpp>
#include <sdl/Hypergraph/ArcParser.hpp>
#include <sdl/Hypergraph/ParsedArcsToHg.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Contains.hpp>
#include <sdl/Util/Input.hpp>
#include <sdl/Util/Flag.hpp>
#include <sdl/Util/NormalizeUtf8.hpp>

namespace sdl {
namespace Hypergraph {

namespace ArcParserFctUtil {

/**
   Determines if string starts and end with angle brackets and
   has some chars in between, e.g., <eps>.
*/
inline bool isInAngleBrackets(std::string const& str) {
  std::string::size_type len = str.length();
  return len > 2 && str[0] == '<' && str[len - 1] == '>';
}

//
/**
   \return symbol from string

   unless output, if <xmt-blockN>, then increment numBlockStartSymsSeen

   Special syms like <eps> are not enclosed in quotes but in < >. If the parsed
   text contains other syms like "the" or "<html>", they will be in quotes.
*/
inline Sym add(IVocabulary& voc, std::string const& word, bool lex,
                       std::size_t* numBlockStartSymsSeen, Sym defaultSym = NoSymbol,
                      bool increaseNumBlocks = true) {
  if (word.empty()) return defaultSym;

  // we don't allow quoted "<special>"
  if (lex)
    return voc.add(word, kTerminal);
  else {
    if (isInAngleBrackets(word)) {
      Sym id = Vocabulary::specialSymbols().sym(word);
      if (id) {
        if (id == BLOCK_START::ID) {
          assert(numBlockStartSymsSeen);
          id += (SymInt)*numBlockStartSymsSeen;
          if (increaseNumBlocks) ++*numBlockStartSymsSeen;
        }
        return id;
      } else {
        SDL_WARN(Hypergraph.ArcParser, "unknown special symbol " << word << " - treating as nonterminal");
      }
    }
    return voc.add(word, kNonterminal);
  }
}
}

LabelPair const NoSymbols(NoSymbol, NoSymbol);

template <class Arc>
void addState(ParserUtil::State& s, SymsToState* symsToState, StateId& highestStateId, IVocabulary& voc,
              IMutableHypergraph<Arc>* result, std::string const& src, std::size_t linenum,
              std::size_t* numBlockStartSymsSeen) {
  // need to call input before output symbol so block-start is incremented (true)
  Sym const input = ArcParserFctUtil::add(voc, s.inputSymbol, s.isInputSymbolLexical,
                                                  numBlockStartSymsSeen, NoSymbol, true);
  Sym const output = ArcParserFctUtil::add(voc, s.outputSymbol, s.isOutputSymbolLexical,
                                                   numBlockStartSymsSeen, NoSymbol, false);
  LabelPair newLabels(input, output);

  if (s.id == kNoState) {
    if (newLabels == NoSymbols)
      SDL_THROW_LOG(Hypergraph.ArcParserFct, FileFormatException,
                    "syntax error: addState for no labels and no state");
    StateId* pState;
    if (Util::update(*symsToState, newLabels, pState)) {
      result->addStateId(s.id = *pState = ++highestStateId, newLabels);
    } else {
      s.id = *pState;
      assert(s.id < result->size());
    }
  } else if (s.id == ParserUtil::State::kStart || s.id == ParserUtil::State::kFinal) {
    if (newLabels != NoSymbols)
      SDL_THROW_LOG(Hypergraph.ArcParserFct, FileFormatException,
                    "syntax error: " << src << ":" << linenum
                                     << ":syntax error (START and FINAL cannot have symbols)"
                                     << ": " << s);
  } else {  // state was specified already
    (*symsToState)[newLabels]
        = s.id;  // this may be updated several times if user keeps using diff stateids w/ same label

    LabelPair existingLabels = result->labelPairOptionalOutput(s.id);
    if (newLabels == NoSymbols) {
      result->addStateId(s.id);
    } else if (existingLabels == NoSymbols || existingLabels == newLabels) {
      result->addStateId(s.id, newLabels);
    } else if (compatible(existingLabels, newLabels)) {
      if (!newLabels.second) {
        newLabels.second = newLabels.first;
        (*symsToState)[newLabels] = s.id;
      }
      result->addStateId(s.id, newLabels);
    } else {
      SDL_THROW_LOG(Hypergraph.ArcParserFct, FileFormatException,
                    src << ":" << linenum << ": syntax error (incompatible symbols for state " << s.id << ")"
                        << ": " << s << "; previous labels=" << Util::print(existingLabels, &voc)
                        << " vs. new labels=" << Util::print(newLabels, &voc));
    }
  }
}

namespace impl {
/**
   used as parseText(ParsedArcs ...) which takes ownership of new arcs. TODO: smart ptr
*/
struct ParsedArcsConsumer {
  ParsedArcs& arcs;
  ParsedArcsConsumer(ParsedArcs& arcs) : arcs(arcs) {}
  ArcParser arcParser;
  mutable Util::Counter linenum;

  /// line is chomped (no trailing '\n') per StringConsumer
  void operator()(std::string const& line) const {
    ++linenum;
    if (!(line.empty() || line[0] == '#')) {
      ParserUtil::Arc* pArc = arcParser.parse(line);
      if (!pArc)
        SDL_THROW_LOG(Hypergraph.ParsedLines, FileFormatException, ":" << linenum << ":syntax error" << line);
      arcs.push_back(pArc);
    }
  }
};
}

template <class Arc>
void parseText(std::istream& in, std::string const& inFilename, IMutableHypergraph<Arc>* result,
              bool requireNfc = true) {
  ParsedArcs arcs;
  impl::ParsedArcsConsumer accept(arcs);
  Util::visitChompedLines(in, accept, requireNfc);
  parsedArcsToHg(arcs, result, inFilename);
}

template <class Arc>
MutableHypergraph<Arc>* readHypergraphNew(std::istream& in, IVocabularyPtr pVoc,
                                          Properties props = kFsmOutProperties,
                                          std::string const& inName = "input") {
  MutableHypergraph<Arc>* r = new MutableHypergraph<Arc>(props);
  r->setVocabulary(pVoc);
  parseText(in, inName, r);
  return r;
}

template <class Arc>
shared_ptr<IHypergraph<Arc> > readHypergraph(std::istream& in, IVocabularyPtr pVoc,
                                             Properties props = kFsmOutProperties,
                                             std::string const& inName = "input") {
  return readHypergraphNew<Arc>(in, pVoc, props, inName);
}

template <class Arc>
IHypergraph<Arc>& readHypergraph(std::istream& in, IMutableHypergraph<Arc>& hg,
                                 std::string const& inName = "input") {
  hg.clear();
  hg.forceStoreArcs();
  parseText(in, inName, &hg);
  return hg;
}

template <class Arc>
shared_ptr<IHypergraph<Arc> > readHypergraph(Util::InputStream const& in, IVocabularyPtr pVoc,
                                             Properties props = kFsmOutProperties) {
  return readHypergraph<Arc>(*in, pVoc, props, in.name);
}

template <class Arc>
IHypergraph<Arc>& readHypergraph(Util::InputStream const& in, IMutableHypergraph<Arc>& hg) {
  return readHypergraph(*in, hg, in.name);
}


}}

#endif
