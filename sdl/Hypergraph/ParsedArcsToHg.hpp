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

  // increments on each <xmt-blockN> symbol
  std::size_t numBlockStartSymsSeen = 0;

  IVocabularyPtr pVoc = result->getVocabulary();

  //TODO: kCanonicalLex - need to not store out arcs while addArc parsing, restore them after if you like
  // Figure out next available state ID (for states for which no state
  // ID is specified)
  // this will have the effect of putting state-id-free lexical axioms after the rest of the states
  StateId highestStateId = 0; // should be -1 but that is kNoState, so added workaround using hasStateIds
  bool hasStateIds = false;
  forall (ParsedArc* arc, arcs) {
    arc->head.increaseMaxId(highestStateId);
    if (arc->head.hasId())
      hasStateIds = true;
    forall (ParserUtil::State s, arc->tails) {
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
  forall (ParsedArc* wrappedArc, arcs) {

    // Add states
    forall (ParserUtil::State& t, wrappedArc->tails) {
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
      forall (ParserUtil::State t, wrappedArc->tails) {
        arc->addTail(t.id);
      }
      if (!wrappedArc->weightStr.empty()) {
        Weight w;
        try {
          parseWeightString(wrappedArc->weightStr, &w);
        }
        catch(std::exception& e) {
          SDL_THROW_LOG(Hypergraph.ArcParserFct, FileFormatException,
                        inFilename << ":" << linenum
                        << ":syntax error (bad weight '" << wrappedArc->weightStr << "'): " << e.what());
        }
        arc->setWeight(w);
      }
      result->addArc(arc);
    }
    ++linenum;
    delete wrappedArc;
  }
}

}}

#endif
