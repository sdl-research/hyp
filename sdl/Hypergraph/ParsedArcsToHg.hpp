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

    make a hg from parsed arcs.
*/

#ifndef PARSEDARCSTOHG_JG_2014_01_14_HPP
#define PARSEDARCSTOHG_JG_2014_01_14_HPP
#pragma once

#include <sdl/Hypergraph/ParsedArcs.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Util/Unordered.hpp>

namespace sdl { namespace Hypergraph {

typedef unordered_map<LabelPair, StateId> SymsToState;

/// add arcs to empty hg *result. inFilename is for error messages
template<class Arc>
void parsedArcsToHg(ParsedArcs const& arcs
                    , IMutableHypergraph<Arc>* result
                    , std::string const& inFilename)
{
  assert(result->size() == 0);
  typedef typename Arc::Weight Weight;
  typedef typename Weight::FloatT Cost;

  // increments on each <xmt-blockN> symbol
  std::size_t numBlockStartSymsSeen = 0;

  IVocabularyPtr pVoc = result->getVocabulary();

  //TODO: kCanonicalLex - need to not store out arcs while addArc parsing, restore them after if you like
  // Figure out next available state ID (for states for which no state
  // ID is specified)
  // this will have the effect of putting state-id-free lexical axioms after the rest of the states
  StateId highestStateId = 0; // should be -1 but that is kNoState, so added workaround using hasStateIds
  bool hasStateIds = false;
  for (ParsedArc* arc : arcs) {
    arc->head.increaseMaxId(highestStateId);
    if (arc->head.hasId())
      hasStateIds = true;
    for (ParserUtil::State s : arc->tails) {
        s.increaseMaxId(highestStateId);
        if (s.hasId())
          hasStateIds = true;
    }
  }

  if (!hasStateIds) {
    assert(highestStateId == 0);
    highestStateId = (StateId)-1; // so that next available ID is 0
  }

  std::size_t linenum = 1;
  SymsToState symsToState;
  for (ParsedArc* wrappedArc : arcs) {

    // Add states
    for (ParserUtil::State& t : wrappedArc->tails) {
      addState(t, &symsToState, highestStateId, *pVoc, result,
               inFilename, linenum, &numBlockStartSymsSeen);
    }
    addState(wrappedArc->head, &symsToState, highestStateId, *pVoc, result,
             inFilename, linenum, &numBlockStartSymsSeen);

    if (wrappedArc->head.id == ParserUtil::State::kStart) {
      if (wrappedArc->tails.size() > 1)
        SDL_THROW_LOG(Hypergraph.ArcParserFct, FileFormatException,
                      inFilename << ":" << linenum << ":syntax error (START must have one tail only)");
      result->setStart(wrappedArc->tails[0].id);
    }
    else if (wrappedArc->head.id == ParserUtil::State::kFinal) {
      if (wrappedArc->tails.size() > 1)
        SDL_THROW_LOG(Hypergraph.ArcParserFct, FileFormatException,
                      inFilename << ":" << linenum << ":syntax error (FINAL must have one tail only)");
      result->setFinal(wrappedArc->tails[0].id);
    }
    else {
      Arc* arc = new Arc();
      arc->setHead(wrappedArc->head.id);
      for (ParserUtil::State t : wrappedArc->tails) {
        arc->addTail(t.id);
      }
      std::string const& wtstr = wrappedArc->weightStr;
      if (wtstr.empty()) {
        setOne(arc->weight_);
      } else {
        try {
          arc->weight_.set(wtstr);
        }
        catch(std::exception& e) {
          SDL_THROW_LOG(Hypergraph.ArcParserFct, FileFormatException,
                        inFilename << ":" << linenum
                        << ":syntax error (bad weight '" << wrappedArc->weightStr << "'): " << e.what());
        }
      }
      result->addArc(arc);
    }
    ++linenum;
    delete wrappedArc;
  }
}

}}

#endif
